#pragma once

#include <aw/spoofiq/core/types.hpp>
#include <aw/spoofiq/core/metadata.hpp>
#include <aw/spoofiq/proc/image_processor.hpp>
#include <filesystem>
#include <string>
#include <vector>
#include <atomic>
#include <thread>

namespace aw::spoofiq::ui {

class BatchSpooferPanel : public IPanel {
public:
    explicit BatchSpooferPanel();
    ~BatchSpooferPanel();
    
    PanelId id() const override { return PanelId::BatchSpoofer; }
    std::string name() const override { return "Batch Spoofer"; }
    void render() override;

private:
    // UI state
    std::string input_paths_buffer_;
    std::string output_dir_;
    std::string status_message_;
    std::atomic<bool> processing_{false};
    
    // Progress tracking (atomic for thread-safe access)
    std::atomic<int> total_files_{0};
    std::atomic<int> processed_count_{0};
    std::atomic<float> progress_percent_{0.0f};
    
    // Image processing
    core::ExifData exif_data_;
    std::atomic<int> next_index_{1};
    std::atomic<bool> cancel_flag_{false};
    
    // Worker thread
    std::thread worker_thread_;
    
    void process_batch();
    void process_batch_worker(std::vector<std::filesystem::path> input_files, std::filesystem::path output_dir);
};

} // namespace aw::spoofiq::ui
