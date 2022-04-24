#include "utility.h"
#include "ray.h"

auto clamp(double rX, double rMin, double rMax) -> double {
    if (rX < rMin) {
        return rMin;
    }
    if (rX > rMax) {
        return rMax;
    }
    return rX;
};

auto randomVector() -> Vector3d {
    return {randomNumber<double>(), randomNumber<double>(), randomNumber<double>()};
};
auto randomVector(double rMin, double rMax) -> Vector3d {
    return {randomNumber<double>(rMin, rMax), randomNumber<double>(rMin, rMax), randomNumber<double>(rMin, rMax)};
};
auto randomInUnitSphere() -> Vector3d {
    while (true) {
        auto vRandomPoint = randomVector(-1, 1);
        if (std::pow(vRandomPoint.norm(), 2.0) >= 1) {
            continue;
        }
        return vRandomPoint;
    }
};
auto randomInUnitDisk() -> Vector3d {
    while (true) {
        auto vPoint = Vector3d(randomNumber<double>(-1,1), randomNumber<double>(-1,1), 0);
        if (std::pow(vPoint.norm(), 2.0) < 1) {
            return vPoint;
        };
    }
};
auto randomUnitVector() -> Vector3d {
    auto vRandomVector = randomInUnitSphere();
    return vRandomVector / vRandomVector.norm();
};
auto vectorNearZero(const Vector3d& rVector) -> bool {
    // Return true if the vector is close to zero in all dimensions
    const auto vTolerance = 1e-8;
    return (rVector.x() < vTolerance && rVector.y() < vTolerance && rVector.z() < vTolerance);
};
