#include <aw/spoofiq/proc/image_processor.hpp>
#include <aw/spoofiq/io/image_io.hpp>
#include <aw/spoofiq/io/metadata_io.hpp>
#include <fstream>

namespace aw::spoofiq::proc {

std::expected<void, std::string> ImageProcessor::process(
    const std::filesystem::path& input,
    const std::filesystem::path& output,
    const core::ExifData& exif_data
) {
    // Load image
    auto load_result = io::ImageIO::load(input);
    if (!load_result) {
        return std::unexpected(load_result.error());
    }
    
    auto& img = load_result.value();
    
    // Convert RGBA to RGB if needed
    if (img.channels == 4) {
        std::vector<uint8_t> rgb_data(img.width * img.height * 3);
        for (size_t i = 0; i < img.width * img.height; ++i) {
            rgb_data[i * 3 + 0] = img.data[i * 4 + 0];
            rgb_data[i * 3 + 1] = img.data[i * 4 + 1];
            rgb_data[i * 3 + 2] = img.data[i * 4 + 2];
        }
        img.data = std::move(rgb_data);
        img.channels = 3;
    }
    
    // Save to temp location
    std::filesystem::path temp_path = output.parent_path() / ".temp.jpg";
    auto save_result = io::ImageIO::save_jpeg(img, temp_path);
    if (!save_result) {
        return std::unexpected(save_result.error());
    }
    
    // Load temp JPEG and inject EXIF
    std::ifstream temp_file(temp_path, std::ios::binary);
    if (!temp_file.is_open()) {
        return std::unexpected("Failed to open temp JPEG");
    }
    
    std::vector<uint8_t> jpeg_data((std::istreambuf_iterator<char>(temp_file)), std::istreambuf_iterator<char>());
    temp_file.close();
    
    // Build EXIF blob
    auto exif_blob = io::MetadataIO::build_exif_blob(exif_data);
    
    // Inject EXIF
    auto final_jpeg = io::MetadataIO::inject_exif_into_jpeg(jpeg_data, exif_blob);
    
    // Write final output
    std::ofstream out_file(output, std::ios::binary);
    if (!out_file.is_open()) {
        std::filesystem::remove(temp_path);
        return std::unexpected("Failed to open output file: " + output.string());
    }
    
    out_file.write(reinterpret_cast<const char*>(final_jpeg.data()), final_jpeg.size());
    out_file.close();
    
    // Clean up temp
    std::filesystem::remove(temp_path);
    
    return {};
}

} // namespace aw::spoofiq::proc
