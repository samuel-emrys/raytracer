#include "camera.h"
#include "color.h"
#include "constants.h"
#include "conversion.h"
#include "hittable.h"
#include "hittablelist.h"
#include "material.h"
#include "ray.h"
#include "sphere.h"
#include "timer.h"
#include "utility.h"
#include <chrono>
#include <iostream>
#include <memory>

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

auto main() -> int {
    Timer<std::chrono::milliseconds> vTimer;
    // Image
    const auto vAspectRatio = 16.0 / 9.0;
    const int vImageWidth = 800;
    const int vImageHeight = static_cast<int>(vImageWidth / vAspectRatio);
    const int vSamplesPerPixel = 100;
    const int vMaxDepth = 20;

    // World
    HittableList vWorld;

    auto vGroundMaterial = std::make_shared<Lambertian>(Color(0.8, 0.8, 0.0));
    auto vCenterMaterial = std::make_shared<Lambertian>(Color(0.1, 0.2, 0.5));
    auto vLeftMaterial = std::make_shared<Dialectric>(1.5);
    auto vRightMaterial = std::make_shared<Metal>(Color(0.8, 0.6, 0.2), 0.0);

    vWorld.add(std::make_shared<Sphere>(Point3d(0.0, -100.5, -1.0), 100.0, vGroundMaterial));
    vWorld.add(std::make_shared<Sphere>(Point3d(0.0, 0.0, -1.0), 0.5, vCenterMaterial));
    vWorld.add(std::make_shared<Sphere>(Point3d(-1.0, 0.0, -1.0), 0.5, vLeftMaterial));
    vWorld.add(std::make_shared<Sphere>(Point3d(-1.0, 0.0, -1.0), -0.45, vLeftMaterial));
    vWorld.add(std::make_shared<Sphere>(Point3d(1.0, 0.0, -1.0), 0.5, vRightMaterial));

    // Camera
    Point3d vLookFrom = {3, 3, 2};
    Point3d vLookAt = {0, 0, -1};
    Vector3d vViewUp = {0, 1, 0};
    auto vDistanceToFocus = (vLookFrom - vLookAt).norm();
    auto vAperture = 1.0/2.0;
    Camera vCamera(vLookFrom, vLookAt, vViewUp, radians(20.0), vAspectRatio, vAperture, vDistanceToFocus);

    // Render

    std::cout << "P3\n" << vImageWidth << ' ' << vImageHeight << "\n255\n";

    for (int vRow = vImageHeight - 1; vRow >= 0; --vRow) {
        std::cerr << "\rScanlines remaining: " << vRow << ' ' << std::flush;
        for (int vCol = 0; vCol < vImageWidth; ++vCol) {
            Color vPixelColor(0, 0, 0);
            for (int vSample = 0; vSample < vSamplesPerPixel; ++vSample) {
                auto u = (vCol + randomNumber<double>()) / (vImageWidth - 1);  // NOLINT
                auto v = (vRow + randomNumber<double>()) / (vImageHeight - 1); // NOLINT
                Ray vRay = vCamera.getRay(u, v);
                vPixelColor += rayColor(vRay, vWorld, vMaxDepth);
            }
            writeColor(std::cout, vPixelColor, vSamplesPerPixel);
        }
    }
    std::cerr << "\nDone\n";

    return EXIT_SUCCESS;
}
