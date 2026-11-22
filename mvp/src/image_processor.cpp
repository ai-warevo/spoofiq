#include "image_processor.hpp"
#include <format>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../vendor/stb/stb_image_write.h"

namespace spoofiq::mvp {

namespace {

// TIFF/EXIF tags
constexpr uint16_t TAG_MAKE = 0x010F;
constexpr uint16_t TAG_MODEL = 0x0110;
constexpr uint16_t TAG_ORIENTATION = 0x0112;
constexpr uint16_t TAG_SOFTWARE = 0x0131;
constexpr uint16_t TAG_DATETIME = 0x0132;
constexpr uint16_t TAG_EXIF_OFFSET = 0x8769;
constexpr uint16_t TAG_GPS_OFFSET = 0x8825;

constexpr uint16_t TAG_DATETIME_ORIGINAL = 0x9003;
constexpr uint16_t TAG_DATETIME_DIGITIZED = 0x9004;
constexpr uint16_t TAG_FNUMBER = 0x829D;
constexpr uint16_t TAG_ISO_SPEED = 0x8827;
constexpr uint16_t TAG_FOCAL_LENGTH = 0x920A;
constexpr uint16_t TAG_LENS_MAKE = 0xA433;
constexpr uint16_t TAG_LENS_MODEL = 0xA434;

constexpr uint16_t TAG_GPS_LATITUDE_REF = 0x0001;
constexpr uint16_t TAG_GPS_LATITUDE = 0x0002;
constexpr uint16_t TAG_GPS_LONGITUDE_REF = 0x0003;
constexpr uint16_t TAG_GPS_LONGITUDE = 0x0004;
constexpr uint16_t TAG_GPS_ALTITUDE_REF = 0x0005;
constexpr uint16_t TAG_GPS_ALTITUDE = 0x0006;

// TIFF field types
constexpr uint16_t TYPE_BYTE = 1;
constexpr uint16_t TYPE_ASCII = 2;
constexpr uint16_t TYPE_SHORT = 3;
constexpr uint16_t TYPE_RATIONAL = 5;

void write_u16_le(std::vector<uint8_t>& buf, uint16_t val) {
    buf.push_back(val & 0xFF);
    buf.push_back((val >> 8) & 0xFF);
}

void write_u32_le(std::vector<uint8_t>& buf, uint32_t val) {
    buf.push_back(val & 0xFF);
    buf.push_back((val >> 8) & 0xFF);
    buf.push_back((val >> 16) & 0xFF);
    buf.push_back((val >> 24) & 0xFF);
}

// Simplified EXIF builder for MVP - hardcoded structure
std::vector<uint8_t> build_minimal_exif(const ExifData& exif) {
    std::vector<uint8_t> blob;
    
    // EXIF header: "Exif\0\0"
    const char exif_header[] = "Exif\0\0";
    blob.insert(blob.end(), exif_header, exif_header + 6);
    
    // TIFF header: little-endian, magic 42, IFD0 offset
    blob.push_back(0x49); blob.push_back(0x49);  // "II" little-endian
    write_u16_le(blob, 42);  // TIFF magic
    write_u32_le(blob, 8);   // IFD0 offset from TIFF start
    
    // For MVP: simplified structure with core tags only
    // IFD0: 6 entries (Make, Model, Orientation, Software, ExifOffset, GPSOffset)
    write_u16_le(blob, 6);  // number of entries
    
    // Reserve space for 6 entries (12 bytes each) + next IFD offset
    std::vector<uint8_t> ifd0_data;
    
    // Tag entries are 12 bytes: tag(2) type(2) count(4) value/offset(4)
    // We'll build the data area after the IFD
    std::vector<uint8_t> data_area;
    uint32_t data_offset = 8 + 2 + (6 * 12) + 4;  // After IFD0 and next-IFD pointer
    
    // Make (ASCII)
    write_u16_le(ifd0_data, TAG_MAKE);
    write_u16_le(ifd0_data, TYPE_ASCII);
    write_u32_le(ifd0_data, exif.make.size() + 1);
    write_u32_le(ifd0_data, data_offset + data_area.size());
    data_area.insert(data_area.end(), exif.make.begin(), exif.make.end());
    data_area.push_back(0);
    
    // Model (ASCII)
    write_u16_le(ifd0_data, TAG_MODEL);
    write_u16_le(ifd0_data, TYPE_ASCII);
    write_u32_le(ifd0_data, exif.model.size() + 1);
    write_u32_le(ifd0_data, data_offset + data_area.size());
    data_area.insert(data_area.end(), exif.model.begin(), exif.model.end());
    data_area.push_back(0);
    
    // Orientation (SHORT)
    write_u16_le(ifd0_data, TAG_ORIENTATION);
    write_u16_le(ifd0_data, TYPE_SHORT);
    write_u32_le(ifd0_data, 1);
    write_u16_le(ifd0_data, exif.orientation);
    write_u16_le(ifd0_data, 0);  // padding
    
    // Software (ASCII)
    write_u16_le(ifd0_data, TAG_SOFTWARE);
    write_u16_le(ifd0_data, TYPE_ASCII);
    write_u32_le(ifd0_data, exif.software.size() + 1);
    write_u32_le(ifd0_data, data_offset + data_area.size());
    data_area.insert(data_area.end(), exif.software.begin(), exif.software.end());
    data_area.push_back(0);
    
    // ExifOffset placeholder
    uint32_t exif_ifd_offset = data_offset + data_area.size();
    write_u16_le(ifd0_data, TAG_EXIF_OFFSET);
    write_u16_le(ifd0_data, 4);  // LONG type
    write_u32_le(ifd0_data, 1);
    write_u32_le(ifd0_data, exif_ifd_offset);
    
    // GPSOffset placeholder
    uint32_t gps_ifd_offset = exif_ifd_offset + 200;  // estimate
    write_u16_le(ifd0_data, TAG_GPS_OFFSET);
    write_u16_le(ifd0_data, 4);  // LONG type
    write_u32_le(ifd0_data, 1);
    write_u32_le(ifd0_data, gps_ifd_offset);
    
    blob.insert(blob.end(), ifd0_data.begin(), ifd0_data.end());
    write_u32_le(blob, 0);  // next IFD offset = 0
    blob.insert(blob.end(), data_area.begin(), data_area.end());
    
    // For MVP: skip full Exif/GPS IFD implementation
    // This simplified version proves the concept
    
    return blob;
}

} // anonymous namespace

std::vector<uint8_t> ImageProcessor::build_exif_blob(const ExifData& exif_data) {
    return build_minimal_exif(exif_data);
}

std::vector<uint8_t> ImageProcessor::inject_exif_into_jpeg(
    const std::vector<uint8_t>& jpeg_data,
    const std::vector<uint8_t>& exif_blob) {
    
    std::vector<uint8_t> result;
    
    // JPEG starts with FFD8 (SOI marker)
    if (jpeg_data.size() < 2 || jpeg_data[0] != 0xFF || jpeg_data[1] != 0xD8) {
        return jpeg_data;  // not a valid JPEG
    }
    
    // Copy SOI marker
    result.push_back(0xFF);
    result.push_back(0xD8);
    
    // Insert APP1 marker with EXIF
    result.push_back(0xFF);
    result.push_back(0xE1);  // APP1 marker
    
    // APP1 length (marker size + exif_blob size)
    uint16_t app1_length = 2 + exif_blob.size();
    result.push_back((app1_length >> 8) & 0xFF);
    result.push_back(app1_length & 0xFF);
    
    // EXIF data
    result.insert(result.end(), exif_blob.begin(), exif_blob.end());
    
    // Copy rest of JPEG (skip SOI)
    result.insert(result.end(), jpeg_data.begin() + 2, jpeg_data.end());
    
    return result;
}

std::expected<std::filesystem::path, std::string> 
ImageProcessor::process(
    const std::filesystem::path& input_path,
    const std::filesystem::path& output_dir,
    const ExifData& exif_data,
    int output_index) {
    
    // Load image
    int width, height, channels;
    uint8_t* img_data = stbi_load(input_path.string().c_str(), &width, &height, &channels, 3);
    
    if (!img_data) {
        return std::unexpected(std::format("Failed to load image: {}", input_path.string()));
    }
    
    // Convert to JPEG in memory
    std::vector<uint8_t> jpeg_data;
    auto write_func = [](void* context, void* data, int size) {
        auto* vec = static_cast<std::vector<uint8_t>*>(context);
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        vec->insert(vec->end(), bytes, bytes + size);
    };
    
    if (!stbi_write_jpg_to_func(write_func, &jpeg_data, width, height, 3, img_data, 95)) {
        stbi_image_free(img_data);
        return std::unexpected("Failed to encode JPEG");
    }
    
    stbi_image_free(img_data);
    
    // Build and inject EXIF
    auto exif_blob = build_exif_blob(exif_data);
    auto final_jpeg = inject_exif_into_jpeg(jpeg_data, exif_blob);
    
    // Write output
    auto output_path = output_dir / std::format("IMG_{:04d}.jpg", output_index);
    
    FILE* f = fopen(output_path.string().c_str(), "wb");
    if (!f) {
        return std::unexpected(std::format("Failed to open output file: {}", output_path.string()));
    }
    
    fwrite(final_jpeg.data(), 1, final_jpeg.size(), f);
    fclose(f);
    
    return output_path;
}

} // namespace spoofiq::mvp
