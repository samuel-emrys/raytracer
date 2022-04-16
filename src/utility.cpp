#include "utility.h"

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
