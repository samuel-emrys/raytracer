#include "camera.h"
#include "color.h"
#include "constants.h"
#include "conversion.h"
#include "hittable.h"
#include "hittablelist.h"
#include "material.h"
#include "matrix.h"
#include "ray.h"
#include "sphere.h"
#include "threadpool.h"
#include "timer.h"
#include "utility.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

auto rayColor(const Ray& rRay, const Hittable& rWorld, uint8_t rDepth) -> Color {
    HitRecord vHitRecord;

    if (rDepth <= 0) {
        // Return black if we've exceeded the ray bounce limit
        return {0, 0, 0};
    }

    if (rWorld.hit(rRay, 0.001, gINFINITY, vHitRecord)) {
        Ray vScatteredRay;
        Color vAttenuation;
        if (vHitRecord.material->scatter(rRay, vHitRecord, vAttenuation, vScatteredRay)) {
            return vAttenuation.cwiseProduct(rayColor(vScatteredRay, rWorld, rDepth - 1));
        }
        return {0, 0, 0};
    }
    Vector3d vUnitDirection = rRay.direction().normalized();
    auto vVectorScalingFactor = 0.5 * (vUnitDirection.y() + 1.0);
    return (1.0 - vVectorScalingFactor) * Color(1.0, 1.0, 1.0)
         + vVectorScalingFactor * Color(0.5, 0.7, 1.0);
};

auto randomScene() -> HittableList {
    HittableList vWorld;

    auto vGroundMaterial = std::make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
    vWorld.add(std::make_shared<Sphere>(Point3d(0, -1000, 0), 1000, vGroundMaterial));

    for (int vX = -11; vX < 11; vX++) {
        for (int vY = -11; vY < 11; vY++) {
            auto vChooseMaterial = randomNumber<double>();
            Point3d vCenter(vX + 0.9 * randomNumber<double>(),
                            0.2,
                            vY + 0.9 * randomNumber<double>());

            if ((vCenter - Point3d(4, 0.2, 0)).norm() > 0.9) {
                std::shared_ptr<Material> vSphereMaterial;

                if (vChooseMaterial < 0.8) {
                    // Diffuse
                    auto vAlbedo = Color(randomVector().cwiseProduct(randomVector()));
                    vSphereMaterial = std::make_shared<Lambertian>(vAlbedo);
                    vWorld.add(std::make_shared<Sphere>(vCenter, 0.2, vSphereMaterial));
                } else if (vChooseMaterial < 0.95) {
                    // Metal
                    auto vAlbedo = Color(randomVector(0.5, 1));
                    auto vFuzz = randomNumber<double>(0, 0.5);
                    vSphereMaterial = std::make_shared<Metal>(vAlbedo, vFuzz);
                    vWorld.add(std::make_shared<Sphere>(vCenter, 0.2, vSphereMaterial));
                } else {
                    // Glass
                    vSphereMaterial = std::make_shared<Dialectric>(1.5);
                    vWorld.add(std::make_shared<Sphere>(vCenter, 0.2, vSphereMaterial));
                }
            }
        }
    }

    auto vMaterial1 = std::make_shared<Dialectric>(1.5);
    vWorld.add(std::make_shared<Sphere>(Point3d(0, 1, 0), 1.0, vMaterial1));

    auto vMaterial2 = std::make_shared<Lambertian>(Color(0.4, 0.2, 0.1));
    vWorld.add(std::make_shared<Sphere>(Point3d(-4, 1, 0), 1.0, vMaterial2));

    auto vMaterial3 = std::make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
    vWorld.add(std::make_shared<Sphere>(Point3d(4, 1, 0), 1.0, vMaterial3));

    return vWorld;
};

auto main() -> int {
    Timer<std::chrono::milliseconds> vTimer;
    // Image
    const auto vAspectRatio = 16.0 / 9.0;
    const int vImageWidth = 1200;
    const int vImageHeight = static_cast<int>(vImageWidth / vAspectRatio);
    const int vSamplesPerPixel = 500;
    const int vMaxDepth = 50;

    // World
    HittableList vWorld = randomScene();

    // Camera
    Point3d vLookFrom = {13, 2, 3};
    Point3d vLookAt = {0, 0, 0};
    Vector3d vViewUp = {0, 1, 0};
    auto vDistanceToFocus = 10.0;
    auto vAperture = 0.1;
    Camera vCamera(vLookFrom,
                   vLookAt,
                   vViewUp,
                   radians(20.0),
                   vAspectRatio,
                   vAperture,
                   vDistanceToFocus);

    // Compute
    ThreadPool vThreadPool;
    // 2D vector of pixels to represent the image
    auto vPixelFutures = Matrix<std::future<Color>>(vImageHeight, vImageWidth);
    auto vPicture = Matrix<Color>(vImageHeight, vImageWidth);

    for (int vRow = vImageHeight - 1; vRow >= 0; --vRow) {
        for (int vCol = 0; vCol < vImageWidth; ++vCol) {
            vPixelFutures(vRow, vCol) = vThreadPool.submit([vRow,
                                                            vCol,
                                                            vSamplesPerPixel,
                                                            vImageWidth,
                                                            vImageHeight,
                                                            vCamera,
                                                            vWorld]() -> Color {
                Color vPixelColor(0, 0, 0);
                for (int vSample = 0; vSample < vSamplesPerPixel; ++vSample) {
                    auto u = (static_cast<double>(vCol) + randomNumber<double>()) // NOLINT
                           / (vImageWidth - 1);
                    auto v = (static_cast<double>(vRow) + randomNumber<double>()) // NOLINT
                           / (vImageHeight - 1);
                    Ray vRay = vCamera.getRay(u, v);
                    vPixelColor += rayColor(vRay, vWorld, vMaxDepth);
                }
                return vPixelColor;
            });
        }
    }

    // Wait for compute to finish
    for (int vRow = vImageHeight - 1; vRow >= 0; --vRow) {
        std::cerr << "\rScanlines remaining: " << vRow << ' ' << std::flush;
        for (int vCol = 0; vCol < vImageWidth; ++vCol) {
            vPicture(vRow, vCol) = vPixelFutures(vRow, vCol).get();
        }
        std::cerr << std::endl;
    }

    // Render

    std::cerr << "Rendering Image...\n";
    std::cout << "P3\n" << vImageWidth << ' ' << vImageHeight << "\n255\n";
    for (int vRow = vImageHeight - 1; vRow >= 0; --vRow) {
        for (int vCol = 0; vCol < vImageWidth; ++vCol) {
            writeColor(std::cout, vPicture(vRow, vCol), vSamplesPerPixel);
        }
    }
    std::cerr << "\nDone." << std::endl;

    return EXIT_SUCCESS;
}
