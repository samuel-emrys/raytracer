#pragma once

#include "color.h"
#include "matrix.h"
#include <filesystem>
#include <fstream>
#include <png++/png.hpp>

class Image {
  public:
    virtual void render(std::filesystem::path rPath, Matrix<Color> rPicture, size_t rSamplesPerPixel) = 0;
    Image() = default;
    Image(const Image&) = default;
    Image(Image&&) = default;
    auto operator=(const Image&) -> Image& = default;
    auto operator=(Image&&) -> Image& = default;
    virtual ~Image() = default;
};

class Ppm : public Image {
  public:
    auto render(std::filesystem::path rPath,
                Matrix<Color> rPicture,
                size_t rSamplesPerPixel) -> void override;
};

class Png : public Image {
  public:
    auto render(std::filesystem::path rPath,
                Matrix<Color> rPicture,
                size_t rSamplesPerPixel) -> void override;
};

class Jpeg : public Image {
  public:
    auto render(std::filesystem::path rPath,
                Matrix<Color> rPicture,
                size_t rSamplesPerPixel) -> void override;
};
