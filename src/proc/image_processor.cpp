#include <aw/spoofiq/proc/image_processor.hpp>
#include <aw/spoofiq/io/image_io.hpp>
#include <aw/spoofiq/io/metadata_io.hpp>
#include <format>
#include <fstream>

namespace aw::spoofiq::proc {

std::expected<std::filesystem::path, std::string> 
ImageProcessor::process(
    const std::filesystem::path& input_path,
    const std::filesystem::path& output_dir,
    const core::ExifData& exif_data,
    int output_index) {
    
    // Load image
    auto img_result = io::ImageIO::load(input_path);
    if (!img_result) {
        return std::unexpected(img_result.error());
    }
    auto& image = img_result.value();
    
    // Convert to RGB if needed
    if (image.channels != 3) {
        auto rgb_result = io::ImageIO::convert_to_rgb(image);
        if (!rgb_result) {
            return std::unexpected(rgb_result.error());
        }
        image = rgb_result.value();
    }
    
    // Create output path: IMG_0001.jpg
    std::filesystem::path temp_jpg = output_dir / std::format("temp_{:04d}.jpg", output_index);
    std::filesystem::path final_jpg = output_dir / std::format("IMG_{:04d}.jpg", output_index);
    
    // Save as JPEG first
    auto save_result = io::ImageIO::save_jpeg(image, temp_jpg, 95);
    if (!save_result) {
        return std::unexpected(save_result.error());
    }
    
    // Read JPEG back
    std::ifstream file(temp_jpg, std::ios::binary);
    if (!file) {
        return std::unexpected("Failed to read temporary JPEG");
    }
    std::vector<uint8_t> jpeg_data((std::istreambuf_iterator<char>(file)), 
                                    std::istreambuf_iterator<char>());
    file.close();
    
    // Build EXIF and inject
    auto exif_blob = io::MetadataIO::build_exif_blob(exif_data);
    auto final_jpeg = io::MetadataIO::inject_exif_into_jpeg(jpeg_data, exif_blob);
    
    // Write final output
    std::ofstream out(final_jpg, std::ios::binary);
    if (!out) {
        return std::unexpected("Failed to write final JPEG");
    }
    out.write(reinterpret_cast<const char*>(final_jpeg.data()), final_jpeg.size());
    out.close();
    
    // Clean up temp file
    std::filesystem::remove(temp_jpg);
    
    return final_jpg;
}

} // namespace aw::spoofiq::proc
