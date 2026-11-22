#pragma once

#include <string>
#include <cstdint>

namespace aw::spoofiq::core {

/// Progress update during batch processing
struct ProgressEvent {
    int total_files = 0;
    int processed_count = 0;
    std::string current_filename;
    float progress_percent = 0.0f;
};

/// Single file processing completed
struct FileFinishedEvent {
    std::string input_filename;
    std::string output_filename;
    bool success = false;
    std::string error_message;
};

/// Entire batch completed
struct BatchCompletedEvent {
    int total_images = 0;
    int total_videos = 0;
    int total_errors = 0;
};

/// Batch processing cancelled
struct BatchCancelledEvent {};

} // namespace aw::spoofiq::core
