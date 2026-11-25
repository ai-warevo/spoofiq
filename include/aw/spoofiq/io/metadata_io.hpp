#ifndef AW_SPOOFIQ_IO_METADATA_IO_HPP
#define AW_SPOOFIQ_IO_METADATA_IO_HPP

#include <aw/spoofiq/core/metadata.hpp>
#include <vector>
#include <cstdint>
#include <expected>
#include <string>

namespace aw::spoofiq::io {

/// Low-level EXIF and QuickTime atom manipulation.
class MetadataIO {
public:
    /// Build a minimal EXIF APP1 segment from ExifData struct.
    /// Returns the binary blob ready to be inserted into JPEG.
    static std::vector<uint8_t> build_exif_blob(const core::ExifData& exif_data);

    /// Inject EXIF APP1 marker into JPEG data.
    /// Takes raw JPEG bytes and EXIF blob, returns modified JPEG with EXIF embedded.
    static std::vector<uint8_t> inject_exif_into_jpeg(
        const std::vector<uint8_t>& jpeg_data,
        const std::vector<uint8_t>& exif_blob);

private:
    // Helper methods for TIFF/EXIF encoding
    static void write_u16_le(std::vector<uint8_t>& buf, uint16_t val);
    static void write_u32_le(std::vector<uint8_t>& buf, uint32_t val);
};

} // namespace aw::spoofiq::io

#endif // AW_SPOOFIQ_IO_METADATA_IO_HPP
