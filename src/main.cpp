#include "camera.h"
#include "color.h"
#include "constants.h"
#include "hittable.h"
#include "hittablelist.h"
#include "ray.h"
#include "sphere.h"
#include "utility.h"
#include <iostream>
#include <memory>

auto rayColor(const Ray& rRay, const Hittable& rWorld) -> Color {
    HitRecord vHitRecord;
    if (rWorld.hit(rRay, 0, gINFINITY, vHitRecord)) {
        return 0.5 * (vHitRecord.normal + Color(1, 1, 1));
    }
    Vector3d vUnitDirection = rRay.direction() / rRay.direction().norm();
    auto vVectorScalingFactor = 0.5 * (vUnitDirection.y() + 1.0);
    return (1.0 - vVectorScalingFactor) * Color(1.0, 1.0, 1.0)
         + vVectorScalingFactor * Color(0.5, 0.7, 1.0);
};

auto main() -> int {
    // Image

    const auto vAspectRatio = 16.0 / 9.0;
    const int vImageWidth = 800;
    const int vImageHeight = static_cast<int>(vImageWidth / vAspectRatio);
    const int vSamplesPerPixel = 100;

    // World
    HittableList vWorld;
    vWorld.add(std::make_shared<Sphere>(Point3d(0, 0, -1), 0.5));
    vWorld.add(std::make_shared<Sphere>(Point3d(0, -100.5, -1), 100));

    // Camera
    Camera vCamera;

    // Render

    std::cout << "P3\n" << vImageWidth << ' ' << vImageHeight << "\n255\n";

    for (int vRow = vImageHeight - 1; vRow >= 0; --vRow) {
        std::cerr << "\rScanlines remaining: " << vRow << ' ' << std::flush;
        for (int vCol = 0; vCol < vImageWidth; ++vCol) {
            //auto u = double(vCol) / (vImageWidth - 1);
            //auto v = double(vRow) / (vImageHeight - 1);
            //Ray vRay = vCamera.getRay(u, v);
            //Color vPixelColor = rayColor(vRay, vWorld);
            //writeColor(std::cout, vPixelColor);
            //writeColor(std::cout, vPixelColor, 1);
            Color vPixelColor(0, 0, 0);
            for (int vSample = 0; vSample < vSamplesPerPixel; ++vSample) {
                auto u = (vCol + randomNumber<double>()) / (vImageWidth - 1); // NOLINT
                auto v = (vRow + randomNumber<double>()) / (vImageHeight - 1); // NOLINT
                Ray vRay = vCamera.getRay(u, v);
                vPixelColor += rayColor(vRay, vWorld);
            }
            writeColor(std::cout, vPixelColor, vSamplesPerPixel);
        }
    }
    std::cerr << "\nDone\n";

    return EXIT_SUCCESS;
}
