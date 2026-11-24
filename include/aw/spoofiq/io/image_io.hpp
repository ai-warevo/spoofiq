#ifndef AW_SPOOFIQ_IO_IMAGE_IO_HPP
#define AW_SPOOFIQ_IO_IMAGE_IO_HPP

#include <vector>
#include <cstdint>
#include <string>
#include <filesystem>
#include <expected>

namespace aw::spoofiq::io {

/// In-memory image representation.
struct Image {
    int width = 0;
    int height = 0;
    int channels = 0;  // 1=gray, 3=RGB, 4=RGBA
    std::vector<uint8_t> data;
    
    [[nodiscard]] bool is_valid() const {
        return width > 0 && height > 0 && channels > 0 && 
               !data.empty() && data.size() == static_cast<size_t>(width * height * channels);
    }
};

/// Image I/O operations using stb_image/stb_image_write.
class ImageIO {
public:
    /// Load image from file (supports PNG, JPG, BMP, TIF, etc. via stb_image).
    /// Automatically converts to RGB (3 channels).
    static std::expected<Image, std::string> load(const std::filesystem::path& path);

    /// Save image as JPEG with specified quality (0-100).
    static std::expected<void, std::string> save_jpeg(
        const Image& image,
        const std::filesystem::path& output_path,
        int quality = 95);

    /// Save image as PNG.
    static std::expected<void, std::string> save_png(
        const Image& image,
        const std::filesystem::path& output_path);

    /// Convert image to RGB (3 channels) if needed.
    static std::expected<Image, std::string> convert_to_rgb(const Image& input);

private:
    // stb_image callback for JPEG writing
    static void write_callback(void* context, void* data, int size);
};

} // namespace aw::spoofiq::io

#endif // AW_SPOOFIQ_IO_IMAGE_IO_HPP
