#include <aw/spoofiq/proc/video_processor.hpp>

namespace aw::spoofiq::proc {

std::expected<std::filesystem::path, std::string> 
VideoProcessor::process(
    const std::filesystem::path& input_path,
    const std::filesystem::path& output_dir,
    const core::VideoMetadata& metadata,
    int output_index) {
    
    (void)input_path;
    (void)output_dir;
    (void)metadata;
    (void)output_index;
    
    return std::unexpected("Video processing not yet implemented (Alpha 3)");
}

} // namespace aw::spoofiq::proc
