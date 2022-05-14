#include "color.h"

auto writeColor(Color rPixelColor) -> std::string {

    auto vRed = static_cast<int>(255.999 * rPixelColor.x());
    auto vGreen = static_cast<int>(255.999 * rPixelColor.y());
    auto vBlue = static_cast<int>(255.999 * rPixelColor.z());
    return fmt::format("{} {} {}", vRed, vGreen, vBlue);
};

auto writeColor(Color rPixelColor, uint32_t rSamplesPerPixel) -> std::string {
    // Average colour samples
    auto vScale = 1.0 / static_cast<double>(rSamplesPerPixel);
    auto vRed = std::sqrt(vScale * rPixelColor.x());
    auto vGreen = std::sqrt(vScale * rPixelColor.y());
    auto vBlue = std::sqrt(vScale * rPixelColor.z());

    auto vScaledRed = static_cast<int>(256 * clamp(vRed, 0.0, 0.999));
    auto vScaledGreen = static_cast<int>(256 * clamp(vGreen, 0.0, 0.999));
    auto vScaledBlue = static_cast<int>(256 * clamp(vBlue, 0.0, 0.999));
    return fmt::format("{} {} {}", vScaledRed, vScaledGreen, vScaledBlue);
};

