#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>

namespace spoofiq::mvp {

// EXIF data structure matching Python piexif approach
struct ExifData {
    // 0th IFD (Image)
    std::string make = "Apple";
    std::string model = "iPhone 16 Pro Max";
    std::string software = "18.1";
    uint16_t orientation = 1;
    
    // Exif IFD
    std::string datetime_original = "2024:11:15 14:32:18";
    std::string datetime_digitized = "2024:11:15 14:32:18";
    std::pair<uint32_t, uint32_t> f_number = {178, 100};  // f/1.78
    uint16_t iso_speed = 50;
    std::pair<uint32_t, uint32_t> focal_length = {677, 100};  // 6.77mm
    std::string lens_make = "Apple";
    std::string lens_model = "iPhone 16 Pro Max back triple camera 6.77mm f/1.78";
    
    // GPS IFD
    std::string gps_latitude_ref = "N";
    std::vector<std::pair<uint32_t, uint32_t>> gps_latitude = {{37, 1}, {20, 1}, {5, 1}};  // 37°20'5"N
    std::string gps_longitude_ref = "W";
    std::vector<std::pair<uint32_t, uint32_t>> gps_longitude = {{122, 1}, {0, 1}, {32, 1}};  // 122°0'32"W
    std::pair<uint32_t, uint32_t> gps_altitude = {30, 1};  // 30m
    uint8_t gps_altitude_ref = 0;
};

class ImageProcessor {
public:
    // Process single image: load → inject EXIF → save as JPG
    static std::expected<std::filesystem::path, std::string> 
    process(const std::filesystem::path& input_path, 
            const std::filesystem::path& output_dir,
            const ExifData& exif_data,
            int output_index);

private:
    // Build EXIF binary blob (simplified approach for MVP)
    static std::vector<uint8_t> build_exif_blob(const ExifData& exif_data);
    
    // Inject EXIF APP1 marker into JPEG data
    static std::vector<uint8_t> inject_exif_into_jpeg(
        const std::vector<uint8_t>& jpeg_data,
        const std::vector<uint8_t>& exif_blob);
};

} // namespace spoofiq::mvp
