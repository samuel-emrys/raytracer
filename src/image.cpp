#include "image.h"

auto Ppm::render(std::filesystem::path rPath,
                 Matrix<Color> rPicture,
                 size_t rSamplesPerPixel) -> void {
    std::ofstream vFile(rPath);
    vFile << fmt::format("P3\n{} {}\n255\n", rPicture.cols(), rPicture.rows());
    // Iterate in reverse order for ppm format
    for (size_t vRow = rPicture.rows() - 1; vRow < rPicture.rows(); --vRow) {
        for (size_t vCol = 0; vCol < rPicture.cols(); ++vCol) {
            vFile << fmt::format("{}\n",
                                 writeColor(rPicture(vRow, vCol), rSamplesPerPixel));
        }
    }
}

auto Png::render(std::filesystem::path rPath,
                 Matrix<Color> rPicture,
                 size_t rSamplesPerPixel) -> void {
    png::image<png::rgb_pixel> vImage(rPicture.cols(), rPicture.rows());

    for (png::uint_32 vRow = 0; vRow < vImage.get_height(); ++vRow) {
        for (png::uint_32 vCol = 0; vCol < vImage.get_width(); ++vCol) {
            // Image is stored upside down in rPicture, store first rows at the end
            vImage[vImage.get_height() - 1 - vRow][vCol] = png::rgb_pixel(
                static_cast<uint8_t>(getRGB(rPicture(vRow, vCol), rSamplesPerPixel).x()),
                static_cast<uint8_t>(getRGB(rPicture(vRow, vCol), rSamplesPerPixel).y()),
                static_cast<uint8_t>(getRGB(rPicture(vRow, vCol), rSamplesPerPixel).z()));
        }
    }

    vImage.write(rPath.c_str());
}

auto Jpeg::render(std::filesystem::path rPath,
                  Matrix<Color> rPicture,
                  size_t rSamplesPerPixel) -> void {
}
