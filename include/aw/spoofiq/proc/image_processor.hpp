#pragma once

#include <filesystem>
#include <expected>
#include <aw/spoofiq/core/metadata.hpp>

namespace aw::spoofiq::proc {

class ImageProcessor {
public:
    static std::expected<void, std::string> process(
        const std::filesystem::path& input,
        const std::filesystem::path& output,
        const core::ExifData& exif_data
    );

private:
    static std::expected<void, std::string> convert_rgba_to_rgb(
        const std::filesystem::path& input,
        const std::filesystem::path& temp_rgb
    );
};

} // namespace aw::spoofiq::proc
