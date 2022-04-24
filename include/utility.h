#pragma once
#include "ray.h"
#include <random>

template <typename Type>
auto randomNumber() -> Type { // NOLINT
    static std::uniform_real_distribution<Type> sDistribution(static_cast<Type>(0), static_cast<Type>(1));
    static std::mt19937 sGenerator;
    return static_cast<Type>(sDistribution(sGenerator));
};

template <typename Type>
auto randomNumber(Type rMin, Type rMax) -> Type { // NOLINT
    return rMin + (rMax - rMin) * randomNumber<Type>();
};

auto randomVector() -> Vector3d;
auto randomVector(double rMin, double rMax) -> Vector3d;
auto randomInUnitSphere() -> Vector3d;
auto randomUnitVector() -> Vector3d;
auto randomInUnitDisk() -> Vector3d;

auto vectorNearZero(const Vector3d& rVector) -> bool;

auto clamp(double rX, double rMin, double rMax) -> double;
