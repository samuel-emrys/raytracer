#pragma once
#include "ray.h"
#include "utility.h"
#include <Eigen/Dense>
#include <fmt/core.h>
#include <iostream>

using Color = Eigen::Vector3d;

auto writeColor(Color rPixelColor) -> std::string;
auto writeColor(Color rPixelColor, uint32_t rSamplesPerPixel) -> std::string;
