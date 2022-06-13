#include "image.h"
#include <cstddef>
#include <cstdint>
#include <jpeglib.h>
#include <png++/types.hpp>

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
    png::image<png::rgb_pixel> vImage(static_cast<png::uint_32>(rPicture.cols()),
                                      static_cast<png::uint_32>(rPicture.rows()));

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
    // Struct to store JPEG compression parametrs and pointers to the working space
    struct jpeg_compress_struct vCInfo {};
    // JPEG error handler
    struct jpeg_error_mgr vJpegErrorManager {};

    vCInfo.err = jpeg_std_error(&vJpegErrorManager);
    // Initialize the JPEG compression object
    jpeg_create_compress(&vCInfo);

    gsl::owner<FILE*> pvOutputFile = nullptr;
    if ((pvOutputFile = fopen(rPath.c_str(), "wbe")) == nullptr) {
        std::cerr << fmt::format("Unable to open {}. Render failed.", rPath.string());
        return;
    }

    jpeg_stdio_dest(&vCInfo, pvOutputFile);
    vCInfo.image_width = static_cast<uint32_t>(rPicture.cols());
    vCInfo.image_height = static_cast<uint32_t>(rPicture.rows());
    vCInfo.input_components = 3; // Number of color components per pixel
    vCInfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&vCInfo);
    int vQuality = 100;
    jpeg_set_quality(&vCInfo, vQuality, TRUE);
    jpeg_start_compress(&vCInfo, TRUE);

    std::vector<unsigned char> vImageBuffer;
    auto vRowStride = rPicture.cols() * 3;
    // Transform image to format that libjpeg expects
    for (size_t vRow = rPicture.rows() - 1; vRow < rPicture.rows(); --vRow) {
        for (size_t vCol = 0; vCol < rPicture.cols(); ++vCol) {
            // Image is stored upside down in rPicture, store first rows at the end
            auto vElementRGB = getRGB(rPicture(vRow, vCol), rSamplesPerPixel);
            vImageBuffer.push_back(static_cast<unsigned char>(vElementRGB.x()));
            vImageBuffer.push_back(static_cast<unsigned char>(vElementRGB.y()));
            vImageBuffer.push_back(static_cast<unsigned char>(vElementRGB.z()));
        }
    }

    JSAMPROW pvRowPointer = nullptr;
    while (vCInfo.next_scanline < vCInfo.image_height) {
        pvRowPointer = &vImageBuffer[vCInfo.next_scanline * vRowStride];
        jpeg_write_scanlines(&vCInfo, &pvRowPointer, 1);
    }

    jpeg_finish_compress(&vCInfo);
    fclose(pvOutputFile);
    jpeg_destroy_compress(&vCInfo);
}
