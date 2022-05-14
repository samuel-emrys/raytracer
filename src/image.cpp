#include "image.h"

auto Ppm::render(std::filesystem::path rPath, Matrix<Color> rPicture, size_t rSamplesPerPixel) -> void {
    std::ofstream vFile(rPath);
    vFile << fmt::format("P3\n{} {}\n255\n", rPicture.cols(), rPicture.rows());
    // Iterate in reverse order for ppm format
    for (size_t vRow = rPicture.rows() - 1; vRow < rPicture.rows(); --vRow) {
        for (size_t vCol = 0; vCol < rPicture.cols(); ++vCol) {
            vFile << fmt::format("{}\n", writeColor(rPicture(vRow, vCol), rSamplesPerPixel));
        }
    }
}

auto Png::render(std::filesystem::path rPath, Matrix<Color> rPicture, size_t rSamplesPerPixel) -> void {

}

auto Jpeg::render(std::filesystem::path rPath, Matrix<Color> rPicture, size_t rSamplesPerPixel) -> void {

}
