#pragma once
#include "ray.h"
#include <Eigen/Dense>
#include <iostream>

using Color = Eigen::Vector3d;

auto writeColor(std::ostream &rOut, Color rPixelColor) -> void;
auto writeColor(std::ostream &rOut, Color rPixelColor, uint32_t rSamplesPerPixel) -> void;
