#include "image.h"

auto Ppm::render(Matrix<Color> rPicture, size_t rSamplesPerPixel) -> void {

    std::cout << "P3\n" << rPicture.cols() << ' ' << rPicture.rows() << "\n255\n";
    // Iterate in reverse order for ppm format
    for (size_t vRow = rPicture.rows() - 1; vRow < rPicture.rows(); --vRow) {
        for (size_t vCol = 0; vCol < rPicture.cols(); ++vCol) {
            std::cout << writeColor(rPicture(vRow, vCol), rSamplesPerPixel);
        }
    }
}

auto Png::render(Matrix<Color> rPicture, size_t rSamplesPerPixel) -> void {

}

auto Jpeg::render(Matrix<Color> rPicture, size_t rSamplesPerPixel) -> void {

}
