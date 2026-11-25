#include <aw/spoofiq/io/metadata_io.hpp>
#include <format>
#include <cstring>

namespace aw::spoofiq::io {

namespace {

/* TIFF/EXIF tags */
constexpr uint16_t TAG_MAKE = 0x010F;
constexpr uint16_t TAG_MODEL = 0x0110;
constexpr uint16_t TAG_ORIENTATION = 0x0112;
constexpr uint16_t TAG_SOFTWARE = 0x0131;
constexpr uint16_t TAG_EXIF_OFFSET = 0x8769;
constexpr uint16_t TAG_GPS_OFFSET = 0x8825;

/* TIFF field types */
constexpr uint16_t TYPE_ASCII = 2;
constexpr uint16_t TYPE_SHORT = 3;

} // anonymous namespace

void MetadataIO::write_u16_le(std::vector<uint8_t>& buf, uint16_t val) {
    buf.push_back(val & 0xFF);
    buf.push_back((val >> 8) & 0xFF);
}

void MetadataIO::write_u32_le(std::vector<uint8_t>& buf, uint32_t val) {
    buf.push_back(val & 0xFF);
    buf.push_back((val >> 8) & 0xFF);
    buf.push_back((val >> 16) & 0xFF);
    buf.push_back((val >> 24) & 0xFF);
}

std::vector<uint8_t> MetadataIO::build_exif_blob(const core::ExifData& exif) {
    std::vector<uint8_t> blob;
    
    /* EXIF header: "Exif\0\0" */
    const char exif_header[] = "Exif\0\0";
    blob.insert(blob.end(), exif_header, exif_header + 6);
    
    /* TIFF header: little-endian, magic 42, IFD0 offset */
    blob.push_back(0x49); blob.push_back(0x49);  // "II"
    write_u16_le(blob, 42);
    write_u32_le(blob, 8);
    
    /* IFD0: 6 entries */
    write_u16_le(blob, 6);
    
    std::vector<uint8_t> ifd0_data;
    std::vector<uint8_t> data_area;
    uint32_t data_offset = 8 + 2 + (6 * 12) + 4;
    
    /* Make (ASCII) */
    write_u16_le(ifd0_data, TAG_MAKE);
    write_u16_le(ifd0_data, TYPE_ASCII);
    write_u32_le(ifd0_data, static_cast<uint32_t>(exif.make.size()) + 1);
    write_u32_le(ifd0_data, data_offset + static_cast<uint32_t>(data_area.size()));
    data_area.insert(data_area.end(), exif.make.begin(), exif.make.end());
    data_area.push_back(0);
    
    write_u16_le(ifd0_data, TAG_MODEL);
    write_u16_le(ifd0_data, TYPE_ASCII);
    write_u32_le(ifd0_data, static_cast<uint32_t>(exif.model.size()) + 1);
    write_u32_le(ifd0_data, data_offset + static_cast<uint32_t>(data_area.size()));
    data_area.insert(data_area.end(), exif.model.begin(), exif.model.end());
    data_area.push_back(0);
    
    write_u16_le(ifd0_data, TAG_ORIENTATION);
    write_u16_le(ifd0_data, TYPE_SHORT);
    write_u32_le(ifd0_data, 1);
    write_u16_le(ifd0_data, 1);
    write_u16_le(ifd0_data, 0);
    
    write_u16_le(ifd0_data, TAG_SOFTWARE);
    write_u16_le(ifd0_data, TYPE_ASCII);
    write_u32_le(ifd0_data, static_cast<uint32_t>(exif.software.size()) + 1);
    write_u32_le(ifd0_data, data_offset + static_cast<uint32_t>(data_area.size()));
    data_area.insert(data_area.end(), exif.software.begin(), exif.software.end());
    data_area.push_back(0);
    
    uint32_t exif_ifd_offset = data_offset + static_cast<uint32_t>(data_area.size());
    write_u16_le(ifd0_data, TAG_EXIF_OFFSET);
    write_u16_le(ifd0_data, 4);
    write_u32_le(ifd0_data, 1);
    write_u32_le(ifd0_data, exif_ifd_offset);
    
    uint32_t gps_ifd_offset = exif_ifd_offset + 200;
    write_u16_le(ifd0_data, TAG_GPS_OFFSET);
    write_u16_le(ifd0_data, 4);
    write_u32_le(ifd0_data, 1);
    write_u32_le(ifd0_data, gps_ifd_offset);
    
    blob.insert(blob.end(), ifd0_data.begin(), ifd0_data.end());
    write_u32_le(blob, 0);
    blob.insert(blob.end(), data_area.begin(), data_area.end());
    
    return blob;
}

std::vector<uint8_t> MetadataIO::inject_exif_into_jpeg(
    const std::vector<uint8_t>& jpeg_data,
    const std::vector<uint8_t>& exif_blob) {
    
    std::vector<uint8_t> result;
    
    if (jpeg_data.size() < 2 || jpeg_data[0] != 0xFF || jpeg_data[1] != 0xD8) {
        return jpeg_data;
    }
    
    /* SOI marker */
    result.push_back(0xFF);
    result.push_back(0xD8);
    
    /* APP1 marker with EXIF */
    result.push_back(0xFF);
    result.push_back(0xE1);
    
    uint16_t app1_length = 2 + static_cast<uint16_t>(exif_blob.size());
    result.push_back((app1_length >> 8) & 0xFF);
    result.push_back(app1_length & 0xFF);
    
    result.insert(result.end(), exif_blob.begin(), exif_blob.end());
    result.insert(result.end(), jpeg_data.begin() + 2, jpeg_data.end());
    
    return result;
}

} // namespace aw::spoofiq::io
