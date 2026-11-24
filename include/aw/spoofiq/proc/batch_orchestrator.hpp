#pragma once

#include <aw/spoofiq/core/metadata.hpp>
#include <aw/spoofiq/config/config_manager.hpp>
#include <filesystem>
#include <vector>
#include <expected>
#include <atomic>

namespace aw::spoofiq::proc {

/// Batch processing orchestrator: separates batch logic from UI concerns.
/// Handles sequential/concurrent processing of media files.
class BatchOrchestrator {
public:
    /// Result of batch processing.
    struct BatchResult {
        int success_count = 0;
        int error_count = 0;
        std::vector<std::string> error_messages;
    };

    /// Find the next available IMG_XXXX index in output directory.
    static int find_next_index(const std::filesystem::path& output_dir);

    /// Process a batch of files synchronously.
    /// Returns BatchResult with counts and error messages.
    static std::expected<BatchResult, std::string> process_batch(
        const std::vector<std::filesystem::path>& input_files,
        const std::filesystem::path& output_dir,
        const DevicePreset& device_preset,
        int start_index,
        std::atomic<bool>& cancel_flag,
        std::atomic<int>& processed_count,
        int total_files);

private:
    /// Process a single file (image or video).
    /// Returns true on success, false on error.
    static bool process_file(
        const std::filesystem::path& input_path,
        const std::filesystem::path& output_dir,
        const DevicePreset& device_preset,
        int output_index,
        std::string& out_error);
};

} // namespace aw::spoofiq::proc
