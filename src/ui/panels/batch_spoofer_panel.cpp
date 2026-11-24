#include <aw/spoofiq/ui/panels/batch_spoofer_panel.hpp>
#include <aw/spoofiq/ui/error_dialog.hpp>
#include <aw/spoofiq/proc/batch_orchestrator.hpp>
#include <aw/spoofiq/proc/async_batch_processor.hpp>
#include <aw/spoofiq/proc/task_executor.hpp>
#include <aw/spoofiq/event_bus/event_bus_manager.hpp>
#include <aw/spoofiq/logging/logger.hpp>
#include <imgui.h>
#include <format>
#include <algorithm>
#include <cstring>

namespace aw::spoofiq::ui {

/* Cross-platform safe string copy */
static inline void safe_strncpy(char* dest, size_t dest_size, const char* src) {
#ifdef _WIN32
    strncpy_s(dest, dest_size, src, _TRUNCATE);
#else
    std::strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
#endif
}

BatchSpooferPanel::BatchSpooferPanel() {
    if (auto result = ConfigManager::load()) {
        settings_ = std::move(*result);
        input_folder_ = settings_.last_input_directory;
        refresh_preset_list();
        Logger::info("Loaded config: {} presets available", settings_.presets.size());
    } else {
        Logger::error("Failed to load config: {}", result.error());
        settings_ = {}; // use empty settings
    }

    // Connect to event bus for batch progress updates
    event_bus_ = EventBusManager::get();
    if (event_bus_) {
        // Subscribe to progress events with async adapter
        // (Subscriptions kept alive by event bus for panel lifetime)
        static_cast<void>(
            event_bus_->subscribe<core::ProgressEvent>(
                [this](const core::ProgressEvent& evt) -> aw::tpl::threading::tasks::Task<void> {
                    handle_progress_event(evt);
                    co_return;
                }));
        // Subscribe to file finished events
        static_cast<void>(
            event_bus_->subscribe<core::FileFinishedEvent>(
                [this](const core::FileFinishedEvent& evt) -> aw::tpl::threading::tasks::Task<void> {
                    handle_file_finished_event(evt);
                    co_return;
                }));
        // Subscribe to batch completed events
        static_cast<void>(
            event_bus_->subscribe<core::BatchCompletedEvent>(
                [this](const core::BatchCompletedEvent& evt) -> aw::tpl::threading::tasks::Task<void> {
                    handle_batch_completed_event(evt);
                    co_return;
                }));
    }
}

BatchSpooferPanel::~BatchSpooferPanel() {
    cancel_flag_ = true;
}

void BatchSpooferPanel::refresh_preset_list() {
    available_presets_.clear();
    for (const auto& preset : settings_.presets) {
        available_presets_.push_back(preset.name);
    }

    /* Find selected preset index */
    selected_preset_idx_ = 0;
    for (size_t i = 0; i < settings_.presets.size(); ++i) {
        if (settings_.presets[i].id == settings_.selected_preset_id) {
            selected_preset_idx_ = static_cast<int>(i);
            break;
        }
    }
}

void BatchSpooferPanel::handle_progress_event(const core::ProgressEvent& event) {
    total_files_ = event.total_files;
    processed_count_ = event.processed_count;
    progress_percent_ = event.progress_percent;
}

void BatchSpooferPanel::handle_file_finished_event(const core::FileFinishedEvent& event) {
    if (!event.success) {
        Logger::error("File failed: {}", event.error_message);
    }
}

void BatchSpooferPanel::handle_batch_completed_event(const core::BatchCompletedEvent& event) {
    processing_ = false;
    
    if (event.error_code != 0) {
        // Batch failed
        status_message_ = std::format("Batch FAILED | Error: {}", event.error_message);
        Logger::error("Batch failed: {}", event.error_message);
    } else if (event.total_errors > 0) {
        // Some files failed
        status_message_ = std::format("Batch complete | Success: {} | Errors: {} | {}",
                                     event.total_images, event.total_errors,
                                     event.error_message);
        Logger::info("Batch finished: {} success, {} errors", event.total_images, event.total_errors);
    } else {
        // All success
        status_message_ = std::format("Batch complete | Success: {} files", event.total_images);
        Logger::info("Batch finished successfully: {} files", event.total_images);
    }
}

void BatchSpooferPanel::render() {
    ImGui::Text("1. Source & Export:");
    ImGui::Separator();

    /* Input folder */
    ImGui::Text("Folder:");
    ImGui::SameLine();
    char folder_buf[512];
    safe_strncpy(folder_buf, sizeof(folder_buf), input_folder_.c_str());
    if (ImGui::InputText("##input_folder", folder_buf, sizeof(folder_buf))) {
        input_folder_ = folder_buf;
        settings_.last_input_directory = input_folder_;
    }
    ImGui::SameLine();
    if (ImGui::Button("Browse...")) {
        status_message_ = "File browser not yet implemented (nfd stub)";
    }

    ImGui::Checkbox("Export to subfolder:", &settings_.export_to_subfolder);
    if (settings_.export_to_subfolder) {
        char subfolder_buf[256];
        safe_strncpy(subfolder_buf, sizeof(subfolder_buf), settings_.subfolder_name.c_str());
        if (ImGui::InputText("##subfolder_name", subfolder_buf, sizeof(subfolder_buf))) {
            settings_.subfolder_name = subfolder_buf;
        }
    }

    ImGui::Checkbox("Skip existing IMG_XXXX files:", &settings_.skip_existing_files);
    ImGui::Checkbox("Auto-detect next index:", &settings_.auto_detect_index);
    if (!settings_.auto_detect_index) {
        ImGui::SliderInt("Manual start index:", &settings_.manual_start_index, 1, 9999);
    }

    ImGui::NewLine();
    ImGui::Text("2. Quick Spoofing Settings:");
    ImGui::Separator();

    /* Device preset selector */
    if (ImGui::Combo("Device Preset:", &selected_preset_idx_, 
                     [](void* data, int idx, const char** out_text) {
                         auto* presets = static_cast<std::vector<std::string>*>(data);
                         if (idx < 0 || idx >= static_cast<int>(presets->size())) return false;
                         *out_text = (*presets)[idx].c_str();
                         return true;
                     },
                     &available_presets_, static_cast<int>(available_presets_.size()))) {
        if (selected_preset_idx_ >= 0 && selected_preset_idx_ < static_cast<int>(settings_.presets.size())) {
            settings_.selected_preset_id = settings_.presets[selected_preset_idx_].id;
        }
    }

    /* Timestamp mode */
    ImGui::Checkbox("Use current time:", &settings_.use_current_time);
    if (!settings_.use_current_time) {
        char datetime_buf[32];
        safe_strncpy(datetime_buf, sizeof(datetime_buf), settings_.custom_datetime.c_str());
        if (ImGui::InputText("Custom datetime (ISO8601):", datetime_buf, sizeof(datetime_buf))) {
            settings_.custom_datetime = datetime_buf;
        }
    }

    ImGui::Checkbox("Shift timestamp per file:", &settings_.shift_timestamp_per_file);
    if (settings_.shift_timestamp_per_file) {
        ImGui::SliderInt("Shift seconds:", &settings_.shift_seconds, -86400, 86400);
    }

    /* GPS settings */
    ImGui::Checkbox("Embed GPS coordinates:", &settings_.embed_gps);
    if (settings_.embed_gps) {
        ImGui::Checkbox("Randomize within radius:", &settings_.randomize_gps);
        if (settings_.randomize_gps) {
            ImGui::SliderFloat("Radius (meters):", &settings_.gps_radius_meters, 0.0f, 10000.0f);
        }
    }

    ImGui::NewLine();
    ImGui::Text("3. Execution & Progress:");
    ImGui::Separator();

    /* Process batch button */
    bool is_processing = processing_.load();
    
    if (!is_processing) {
        int file_count = 0;
        if (std::filesystem::exists(input_folder_)) {
            for (const auto& entry : std::filesystem::directory_iterator(input_folder_)) {
                if (!entry.is_regular_file()) continue;
                auto ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp" || ext == ".tif" || ext == ".tiff") {
                    file_count++;
                }
            }
        }
        
        std::string btn_label = std::format("START BATCH PROCESSING ({} files)", file_count);
        if (ImGui::Button(btn_label.c_str(), ImVec2(-1.0f, 0.0f))) {
            start_batch_processing();
        }
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        if (ImGui::Button("CANCEL", ImVec2(-1.0f, 0.0f))) {
            cancel_flag_ = true;
            Logger::warn("Batch cancelled by user");
        }
        ImGui::PopStyleColor();
    }

    if (is_processing || progress_percent_ > 0.0f) {
        float progress = progress_percent_.load();
        int processed = processed_count_.load();
        int total = total_files_.load();
        std::string overlay = std::format("{} / {}", processed, total);
        ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), overlay.c_str());
    }

    if (!status_message_.empty()) {
        ImGui::TextWrapped("%s", status_message_.c_str());
    }
}

void BatchSpooferPanel::start_batch_processing() {
    if (processing_.load()) {
        return;
    }

    /* Validation: check input folder */
    if (input_folder_.empty()) {
        ErrorDialog::show("Input Error", "Please select an input folder");
        return;
    }

    if (!std::filesystem::exists(input_folder_)) {
        ErrorDialog::show("Input Error", std::format("Input folder does not exist:\n{}", input_folder_));
        return;
    }

    /* Validation: check preset selected */
    if (selected_preset_idx_ < 0 || selected_preset_idx_ >= static_cast<int>(available_presets_.size())) {
        ErrorDialog::show("Preset Error", "Please select a device preset");
        return;
    }

    /* Gather input files */
    std::vector<std::filesystem::path> input_files;
    for (const auto& entry : std::filesystem::directory_iterator(input_folder_)) {
        if (!entry.is_regular_file()) continue;

        auto ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp" || ext == ".tif" || ext == ".tiff") {
            input_files.push_back(entry.path());
        }
    }

    if (input_files.empty()) {
        ErrorDialog::show("Input Error", "No supported image files found in folder.\nSupported: .jpg, .jpeg, .png, .bmp, .tif, .tiff");
        return;
    }

    /* Determine output directory */
    std::filesystem::path output_dir = input_folder_;
    if (settings_.export_to_subfolder) {
        output_dir = output_dir / settings_.subfolder_name;
        std::filesystem::create_directories(output_dir);
    }

    /* Save settings */
    if (auto result = ConfigManager::save(settings_); !result) {
        Logger::error("Failed to save config: {}", result.error());
    }

    /* Reset state */
    total_files_ = static_cast<int>(input_files.size());
    processed_count_ = 0;
    progress_percent_ = 0.0f;
    cancel_flag_ = false;
    processing_ = true;
    status_message_ = std::format("Processing {} files...", input_files.size());

    /* Determine start index */
    int start_index = settings_.auto_detect_index ? 
        proc::BatchOrchestrator::find_next_index(output_dir) : 
        settings_.manual_start_index;

    const auto& preset = settings_.presets[selected_preset_idx_];

    // Spawn async batch processing task via TaskExecutor
    auto task = proc::AsyncBatchProcessor::process_batch_async(
        input_files,
        output_dir,
        preset,
        start_index,
        cancel_flag_,
        event_bus_
    );

    proc::TaskExecutor::spawn(std::move(task));
    Logger::info("Batch task spawned on executor (async processing started)");
}

} // namespace aw::spoofiq::ui
