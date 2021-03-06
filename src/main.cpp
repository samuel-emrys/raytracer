#include "camera.h"
#include "color.h"
#include "constants.h"
#include "conversion.h"
#include "hittable.h"
#include "hittablelist.h"
#include "image.h"
#include "material.h"
#include "matrix.h"
#include "ray.h"
#include "sphere.h"
#include "threadpool.h"
#include "timer.h"
#include "utility.h"
#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fmt/core.h>
#include <future>
#include <iostream>
#include <memory>
#include <thread>

auto rayColor(const Ray& rRay, const Hittable& rWorld, uint8_t rDepth) -> Color {

    Color vAttenuation{1.0, 1.0, 1.0};
    Ray vRay = rRay;
    auto vDone = false;

    for (uint8_t vBounce = 0; vBounce < rDepth; ++vBounce) {
        HitRecord vHitRecord;
        Color vRayColor;
        if (rWorld.hit(vRay, 0.001, gINFINITY, vHitRecord)) {
            Ray vScatteredRay;
            if (vHitRecord.material->scatter(vRay, vHitRecord, vRayColor, vScatteredRay)) {
                // The scattered ray is the next ray to process, so stage it
                vRay = vScatteredRay;
            }
            else {
                // Ray is black when it doesn't scatter - no light reflected
                vRayColor = {0, 0, 0};
            }
        }
        else {
            // We're done processing this ray when it no longer intersects with any elements
            // in the world, or the depth limit is reached. Whichever comes first.
            // This condition deals with the former.
            Vector3d vUnitDirection = vRay.direction().normalized();
            auto vVectorScalingFactor = 0.5 * (vUnitDirection.y() + 1.0);
            vRayColor = (1.0 - vVectorScalingFactor) * Color(1.0, 1.0, 1.0)
                 + vVectorScalingFactor * Color(0.5, 0.7, 1.0);
            vDone = true;
        }
        vAttenuation = vAttenuation.cwiseProduct(vRayColor);
        if (vDone) {
            return vAttenuation;
        }
    }
    // Return black if we've exceeded the ray bounce limit
    return {0, 0, 0};
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
                    auto vAlbedo = randomVector(0.5, 1);
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

auto main(int argc, char** argv) -> int {
    Timer<std::chrono::milliseconds> vTimer;

    // Image defaults
    std::string vOutputFilename = "image.ppm";
    std::string vFormat = "ppm";
    auto vAspectRatio = 16.0 / 9.0;
    size_t vImageWidth = 1200;
    uint64_t vSamplesPerPixel = 500;
    uint8_t vMaxDepth = 50;
    double vVerticalFieldOfView = 20.0;

    // Command line interface

    CLI::App vApplication{"A simple program to generate a static scene of spheres using a raytracer"};
    vApplication.add_option("-o,--output", vOutputFilename, fmt::format("The name of the file in which to store the output [{}].", vOutputFilename));
    vApplication.add_option("-f,--format", vFormat, fmt::format("The format of the image to output [{}].", vFormat));
    vApplication.add_option("-a,--aspect-ratio", vAspectRatio, fmt::format("The aspect ratio of the image [{}].", vAspectRatio));
    vApplication.add_option("-w,--width", vImageWidth, fmt::format("The width of the image [{}].", vImageWidth));
    vApplication.add_option("-s,--samples-per-pixel", vSamplesPerPixel, fmt::format("The number of samples to take for each pixel [{}].", vSamplesPerPixel));
    vApplication.add_option("-d,--max-depth", vMaxDepth, fmt::format("The maxmimum number of ray bounces to compute [{}].", vMaxDepth));
    vApplication.add_option("-v,--vertical-field-of-view", vVerticalFieldOfView, fmt::format("The vertical field of view with which to view the scene, in degrees [{}].", vVerticalFieldOfView));

    try {
        vApplication.parse(argc, argv);
    } catch (const CLI::ParseError &vError) {
        return vApplication.exit(vError);
    }

    // Image computations

    const auto vImageHeight = static_cast<size_t>(static_cast<double>(vImageWidth) / vAspectRatio);


    std::unique_ptr<Image> vImage;

    // Switch image type based on CLI parameter
    if (vFormat == "ppm") {
        vImage = std::make_unique<Ppm>();
    }
    else if (vFormat == "png") {
        vImage = std::make_unique<Png>();
    }
    else if (vFormat == "jpeg" || vFormat == "jpg") {
        vImage = std::make_unique<Jpeg>();
    }
    else {
        std::cerr << "Image format not recognised. Defaulting to ppm." << std::endl;
        vImage = std::make_unique<Ppm>();
    }

    // World
    std::cerr << "Generating scene...\n";
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
                   radians(vVerticalFieldOfView),
                   vAspectRatio,
                   vAperture,
                   vDistanceToFocus);

    // Compute
    ThreadPool vThreadPool;
    // 2D vector of pixels to represent the image
    auto vPixelFutures = Matrix<std::future<Color>>(vImageHeight, vImageWidth);
    auto vPicture = Matrix<Color>(vImageHeight, vImageWidth);

    std::cerr << "Initialising computation...\n";
    for (size_t vRow = 0; vRow < vImageHeight; ++vRow) {
        for (size_t vCol = 0; vCol < vImageWidth; ++vCol) {
            vPixelFutures(vRow, vCol) = vThreadPool.submit([=]() -> Color {
                Color vPixelColor(0, 0, 0);
                for (size_t vSample = 0; vSample < vSamplesPerPixel; ++vSample) {
                    auto vU = (static_cast<double>(vCol) + randomNumber<double>())
                           / static_cast<double>(vImageWidth - 1);
                    auto vV = (static_cast<double>(vRow) + randomNumber<double>())
                           / static_cast<double>(vImageHeight - 1);
                    auto vRay = vCamera.getRay(vU, vV);
                    vPixelColor += rayColor(vRay, vWorld, vMaxDepth);
                }
                return vPixelColor;
            });
        }
    }

    // Wait for compute to finish
    for (size_t vRow = 0; vRow < vImageHeight; ++vRow) {
        std::string vOutput = fmt::format("\rScanlines completed: {}/{}", vRow, vImageHeight-1);
        std::cerr << vOutput << std::flush;
        for (size_t vCol = 0; vCol < vImageWidth; ++vCol) {
            // Run another task if this element isn't ready yet
            while (vPixelFutures(vRow, vCol).wait_for(std::chrono::seconds(0))
                   == std::future_status::timeout) {
                vThreadPool.runPendingTask();
            }
            vPicture(vRow, vCol) = vPixelFutures(vRow, vCol).get();
        }
    }
    std::cerr << std::endl;

    // Render
    std::filesystem::path vPath(vOutputFilename);
    vImage->render(vPath, vPicture, vSamplesPerPixel);

    std::cerr << "\nDone." << std::endl;

    return EXIT_SUCCESS;
}
