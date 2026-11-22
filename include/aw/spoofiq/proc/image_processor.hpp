#pragma once

#include <aw/spoofiq/core/metadata.hpp>
#include <filesystem>
#include <string>
#include <expected>

namespace aw::spoofiq::proc {

/// High-level image processing pipeline.
/// Handles loading, EXIF injection, and saving images.
class ImageProcessor {
public:
    /// Process a single image: load → inject EXIF → save as JPEG.
    /// Returns the output file path if successful.
    static std::expected<std::filesystem::path, std::string> 
    process(
        const std::filesystem::path& input_path,
        const std::filesystem::path& output_dir,
        const core::ExifData& exif_data,
        int output_index);
};

} // namespace aw::spoofiq::proc
