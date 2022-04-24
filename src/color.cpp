#include "color.h"
#include "ray.h"
#include "utility.h"

auto writeColor(std::ostream &rOut, Color rPixelColor) -> void {

    rOut << static_cast<int>(255.999 * rPixelColor.x()) << ' '
         << static_cast<int>(255.999 * rPixelColor.y()) << ' '
         << static_cast<int>(255.999 * rPixelColor.z()) << '\n';
};

auto writeColor(std::ostream &rOut, Color rPixelColor, uint32_t rSamplesPerPixel) -> void {
    auto r = rPixelColor.x(); // NOLINT
    auto g = rPixelColor.y(); // NOLINT
    auto b = rPixelColor.z(); // NOLINT

    // Average colour samples
    auto vScale = 1.0 / static_cast<double>(rSamplesPerPixel);
    r = std::sqrt(vScale * r);
    g = std::sqrt(vScale * g);
    b = std::sqrt(vScale * b);

    rOut << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
         << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
         << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
};

