#pragma once
#include "ray.h"
#include "utility.h"
#include <Eigen/Dense>
#include <fmt/core.h>
#include <iostream>

using Color = Eigen::Vector3d;

auto writeColor(Color rPixelColor) -> std::string;
auto writeColor(Color rPixelColor, size_t rSamplesPerPixel) -> std::string;
auto getRGB(Color rPixelColor) -> Color;
auto getRGB(Color rPixelColor, size_t rSamplesPerPixel) -> Color;
