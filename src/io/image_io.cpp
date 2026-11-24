#include <aw/spoofiq/io/image_io.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <format>
#include <fstream>

namespace aw::spoofiq::io {

std::expected<Image, std::string> ImageIO::load(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        return std::unexpected(std::format("File not found: {}", path.string()));
    }

    int width, height, channels;
    unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &channels, 3);

    if (!data) {
        return std::unexpected(std::format("Failed to load image: {}", stbi_failure_reason()));
    }

    Image img;
    img.width = width;
    img.height = height;
    img.channels = 3;

    stbi_image_free(data);
    return img;
}

std::expected<void, std::string> ImageIO::save_jpeg(
    const Image& image,
    const std::filesystem::path& output_path,
    int quality) {
    
    if (!image.is_valid()) {
        return std::unexpected("Invalid image data");
    }

    if (quality < 0 || quality > 100) {
        quality = 95;
    }

    int result = stbi_write_jpg(
        output_path.string().c_str(),
        image.width,
        image.height,
        image.channels,
        image.data.data(),
        quality
    );

    if (!result) {
        return std::unexpected(std::format("Failed to write JPEG: {}", output_path.string()));
    }

    return {};
}

std::expected<void, std::string> ImageIO::save_png(
    const Image& image,
    const std::filesystem::path& output_path) {
    
    if (!image.is_valid()) {
        return std::unexpected("Invalid image data");
    }

    int result = stbi_write_png(
        output_path.string().c_str(),
        image.width,
        image.height,
        image.channels,
        image.data.data(),
        image.width * image.channels
    );

    if (!result) {
        return std::unexpected(std::format("Failed to write PNG: {}", output_path.string()));
    }

    return {};
}

std::expected<Image, std::string> ImageIO::convert_to_rgb(const Image& input) {
    if (input.channels == 3) {
        return input;  // Already RGB
    }

    if (!input.is_valid()) {
        return std::unexpected("Invalid input image");
    }

    // For now, just return as-is if not RGB (proper conversion in Alpha 3)
    return input;
}

} // namespace aw::spoofiq::io
