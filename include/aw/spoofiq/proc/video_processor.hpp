#ifndef AW_SPOOFIQ_PROC_VIDEO_PROCESSOR_HPP
#define AW_SPOOFIQ_PROC_VIDEO_PROCESSOR_HPP

#include <aw/spoofiq/core/metadata.hpp>
#include <filesystem>
#include <string>
#include <expected>

namespace aw::spoofiq::proc {

/**
 * Video processing pipeline (stub for Alpha 3).
 * Will handle loading, metadata injection, and saving video files.
 */
class VideoProcessor {
public:
    /**
     * Process a single video: inject metadata → save as MP4/MOV.
     * (Stub - implementation in Alpha 3 when TagLib is integrated)
     */
    static std::expected<std::filesystem::path, std::string> 
    process(
        const std::filesystem::path& input_path,
        const std::filesystem::path& output_dir,
        const core::VideoMetadata& metadata,
        int output_index);
};

} // namespace aw::spoofiq::proc

#endif // AW_SPOOFIQ_PROC_VIDEO_PROCESSOR_HPP
