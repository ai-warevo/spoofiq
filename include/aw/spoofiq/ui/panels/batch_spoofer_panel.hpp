#pragma once

#include <aw/spoofiq/core/types.hpp>
#include <aw/spoofiq/core/metadata.hpp>
#include <aw/spoofiq/config/config_manager.hpp>
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
    // Settings (loaded from config)
    AppSettings settings_;
    
    // UI state
    std::string input_folder_;
    std::vector<std::string> available_presets_; // cached preset names for combo
    int selected_preset_idx_ = 0;
    std::string status_message_;
    std::atomic<bool> processing_{false};
    
    // Progress tracking (atomic for thread-safe access)
    std::atomic<int> total_files_{0};
    std::atomic<int> processed_count_{0};
    std::atomic<float> progress_percent_{0.0f};
    
    // Worker thread
    std::thread worker_thread_;
    std::atomic<bool> cancel_flag_{false};
    
    void process_batch();
    void process_batch_worker(std::vector<std::filesystem::path> input_files, std::filesystem::path output_dir);
    void refresh_preset_list();
    int find_next_index(const std::filesystem::path& output_dir);
};

} // namespace aw::spoofiq::ui
