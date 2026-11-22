#include <aw/spoofiq/ui/panels/batch_spoofer_panel.hpp>
#include <aw/spoofiq/io/image_io.hpp>
#include <aw/spoofiq/io/metadata_io.hpp>
#include <aw/spoofiq/logger.hpp>
#include <aw/tobus/tobus.hpp>
#include <imgui.h>
#include <format>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace aw::spoofiq::ui {

BatchSpooferPanel::BatchSpooferPanel() {
    // Load settings from config
    if (auto result = ConfigManager::load()) {
        settings_ = std::move(*result);
        input_folder_ = settings_.last_input_directory;
        refresh_preset_list();
        Logger::info("Loaded config: {} presets available", settings_.presets.size());
    } else {
        Logger::error("Failed to load config: {}", result.error());
        settings_ = {}; // use empty settings
    }
}

BatchSpooferPanel::~BatchSpooferPanel() {
    cancel_flag_ = true;
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

void BatchSpooferPanel::refresh_preset_list() {
    available_presets_.clear();
    for (const auto& preset : settings_.presets) {
        available_presets_.push_back(preset.name);
    }
    
    // Find selected preset index
    selected_preset_idx_ = 0;
    for (size_t i = 0; i < settings_.presets.size(); ++i) {
        if (settings_.presets[i].id == settings_.selected_preset_id) {
            selected_preset_idx_ = static_cast<int>(i);
            break;
        }
    }
}

int BatchSpooferPanel::find_next_index(const std::filesystem::path& output_dir) {
    if (!std::filesystem::exists(output_dir)) {
        return 1;
    }
    
    int max_index = 0;
    for (const auto& entry : std::filesystem::directory_iterator(output_dir)) {
        if (!entry.is_regular_file()) continue;
        
        std::string filename = entry.path().stem().string();
        if (filename.starts_with("IMG_") && filename.length() == 8) {
            if (int idx = std::stoi(filename.substr(4)); idx > max_index) {
                max_index = idx;
            }
        }
    }
    
    return max_index + 1;
}

void BatchSpooferPanel::render() {
    ImGui::Text("1. Source & Export:");
    ImGui::Separator();
    
    // Input folder
    ImGui::Text("Folder:");
    ImGui::SameLine();
    char folder_buf[512];
    std::strncpy(folder_buf, input_folder_.c_str(), sizeof(folder_buf) - 1);
    folder_buf[sizeof(folder_buf) - 1] = '\0';
    if (ImGui::InputText("##input_folder", folder_buf, sizeof(folder_buf))) {
        input_folder_ = folder_buf;
        settings_.last_input_directory = input_folder_;
    }
    ImGui::SameLine();
    if (ImGui::Button("Browse...")) {
        status_message_ = "File browser not yet implemented (nfd stub)";
    }
    
    // Export to subfolder
    ImGui::Checkbox("Export to subfolder:", &settings_.export_to_subfolder);
    if (settings_.export_to_subfolder) {
        ImGui::SameLine();
        char subfolder_buf[256];
        std::strncpy(subfolder_buf, settings_.subfolder_name.c_str(), sizeof(subfolder_buf) - 1);
        subfolder_buf[sizeof(subfolder_buf) - 1] = '\0';
        if (ImGui::InputText("##subfolder", subfolder_buf, sizeof(subfolder_buf))) {
            settings_.subfolder_name = subfolder_buf;
        }
    }
    
    // Skip existing + Start index
    ImGui::Checkbox("Skip existing (IMG_XXXX)", &settings_.skip_existing_files);
    ImGui::SameLine();
    ImGui::Text("Start Index:");
    ImGui::SameLine();
    const char* index_modes[] = { "Auto-detect", "Manual" };
    int mode = settings_.auto_detect_index ? 0 : 1;
    if (ImGui::Combo("##index_mode", &mode, index_modes, 2)) {
        settings_.auto_detect_index = (mode == 0);
    }
    if (!settings_.auto_detect_index) {
        ImGui::SameLine();
        ImGui::InputInt("##manual_index", &settings_.manual_start_index);
    }
    
    ImGui::Spacing();
    ImGui::Text("2. Quick Spoofing Settings:");
    ImGui::Separator();
    
    // Device preset combo
    ImGui::Text("Device Preset:");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##preset_combo", available_presets_.empty() ? "No presets" : available_presets_[selected_preset_idx_].c_str())) {
        for (int i = 0; i < static_cast<int>(available_presets_.size()); ++i) {
            bool selected = (i == selected_preset_idx_);
            if (ImGui::Selectable(available_presets_[i].c_str(), selected)) {
                selected_preset_idx_ = i;
                settings_.selected_preset_id = settings_.presets[i].id;
            }
            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    
    // Timestamp mode
    ImGui::Text("Timestamp:");
    ImGui::SameLine();
    bool current_time = settings_.use_current_time;
    if (ImGui::RadioButton("Current Time", current_time)) {
        settings_.use_current_time = true;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Custom Date/Time", !current_time)) {
        settings_.use_current_time = false;
    }
    
    if (!settings_.use_current_time) {
        ImGui::SameLine();
        char datetime_buf[128];
        std::strncpy(datetime_buf, settings_.custom_datetime.c_str(), sizeof(datetime_buf) - 1);
        datetime_buf[sizeof(datetime_buf) - 1] = '\0';
        if (ImGui::InputText("##custom_datetime", datetime_buf, sizeof(datetime_buf))) {
            settings_.custom_datetime = datetime_buf;
        }
    }
    
    // Timestamp shift
    ImGui::Checkbox("Shift timestamp per file:", &settings_.shift_timestamp_per_file);
    if (settings_.shift_timestamp_per_file) {
        ImGui::SameLine();
        ImGui::InputInt("sec##shift_sec", &settings_.shift_seconds);
    }
    
    // GPS controls
    ImGui::Checkbox("Embed GPS", &settings_.embed_gps);
    if (settings_.embed_gps) {
        ImGui::SameLine();
        ImGui::Checkbox("Randomize radius:", &settings_.randomize_gps);
        if (settings_.randomize_gps) {
            ImGui::SameLine();
            ImGui::InputFloat("m##gps_radius", &settings_.gps_radius_meters, 1.0f, 10.0f, "%.1f");
        }
    }
    
    ImGui::Spacing();
    ImGui::Text("3. Execution:");
    ImGui::Separator();
    
    // Start/Cancel button
    bool is_processing = processing_.load();
    if (!is_processing) {
        // Count files
        int file_count = 0;
        if (std::filesystem::exists(input_folder_)) {
            for (const auto& entry : std::filesystem::directory_iterator(input_folder_)) {
                if (entry.is_regular_file()) {
                    auto ext = entry.path().extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                    if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp" || ext == ".tif" || ext == ".tiff") {
                        ++file_count;
                    }
                }
            }
        }
        
        std::string btn_label = std::format("START BATCH PROCESSING ({} files)", file_count);
        if (ImGui::Button(btn_label.c_str(), ImVec2(-1.0f, 0.0f))) {
            process_batch();
        }
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        if (ImGui::Button("CANCEL", ImVec2(-1.0f, 0.0f))) {
            cancel_flag_ = true;
            Logger::warn("Batch cancelled by user");
        }
        ImGui::PopStyleColor();
    }
    
    // Progress bar
    if (is_processing || progress_percent_ > 0.0f) {
        float progress = progress_percent_.load();
        int processed = processed_count_.load();
        int total = total_files_.load();
        std::string overlay = std::format("{} / {}", processed, total);
        ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), overlay.c_str());
    }
    
    // Status
    if (!status_message_.empty()) {
        ImGui::TextWrapped("%s", status_message_.c_str());
    }
}

void BatchSpooferPanel::process_batch() {
    if (processing_.load()) {
        return;
    }
    
    // Gather input files
    std::vector<std::filesystem::path> input_files;
    if (!std::filesystem::exists(input_folder_)) {
        status_message_ = "Input folder does not exist: " + input_folder_;
        return;
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(input_folder_)) {
        if (!entry.is_regular_file()) continue;
        
        auto ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp" || ext == ".tif" || ext == ".tiff") {
            input_files.push_back(entry.path());
        }
    }
    
    if (input_files.empty()) {
        status_message_ = "No supported image files found in folder";
        return;
    }
    
    // Determine output directory
    std::filesystem::path output_dir = input_folder_;
    if (settings_.export_to_subfolder) {
        output_dir = output_dir / settings_.subfolder_name;
        std::filesystem::create_directories(output_dir);
    }
    
    // Save settings
    if (auto result = ConfigManager::save(settings_); !result) {
        Logger::error("Failed to save config: {}", result.error());
    }
    
    // Reset state
    total_files_ = static_cast<int>(input_files.size());
    processed_count_ = 0;
    progress_percent_ = 0.0f;
    cancel_flag_ = false;
    processing_ = true;
    status_message_ = std::format("Processing {} files...", input_files.size());
    
    // Join previous thread if any
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
    
    // Start worker thread
    worker_thread_ = std::thread([this, input_files = std::move(input_files), output_dir]() {
        process_batch_worker(input_files, output_dir);
    });
}

void BatchSpooferPanel::process_batch_worker(std::vector<std::filesystem::path> input_files, std::filesystem::path output_dir) {
    // Find selected preset
    if (selected_preset_idx_ < 0 || selected_preset_idx_ >= static_cast<int>(settings_.presets.size())) {
        Logger::error("Invalid preset index: {}", selected_preset_idx_);
        processing_ = false;
        return;
    }
    
    const auto& preset = settings_.presets[selected_preset_idx_];
    
    // Determine start index
    int next_index = settings_.auto_detect_index ? find_next_index(output_dir) : settings_.manual_start_index;
    std::atomic<int> atomic_index{next_index};
    
    Logger::info("Batch started: {} files, preset: {}, start index: {}", input_files.size(), preset.name, next_index);
    
    int success_count = 0;
    int error_count = 0;
    
    for (size_t i = 0; i < input_files.size(); ++i) {
        if (cancel_flag_.load()) {
            Logger::warn("Batch cancelled after {} files", i);
            status_message_ = std::format("Cancelled | Success: {} | Errors: {}", success_count, error_count);
            break;
        }
        
        const auto& input_path = input_files[i];
        
        // Skip existing files if requested
        int file_index = atomic_index.fetch_add(1);
        std::string output_filename = std::format("IMG_{:04d}.jpg", file_index);
        auto output_path = output_dir / output_filename;
        
        if (settings_.skip_existing_files && std::filesystem::exists(output_path)) {
            Logger::info("Skipped existing: {}", output_filename);
            processed_count_++;
            progress_percent_ = static_cast<float>(processed_count_.load()) / static_cast<float>(total_files_.load());
            continue;
        }
        
        // Build metadata from preset + settings
        core::ExifData exif;
        exif.make = preset.make;
        exif.model = preset.model;
        exif.software = preset.software;
        exif.lens_model = preset.lens_model;
        exif.focal_length = preset.focal_length;
        exif.fnumber = preset.f_number;
        exif.iso_speed = preset.iso;
        
        // GPS handling
        if (settings_.embed_gps) {
            exif.latitude = preset.base_latitude;
            exif.longitude = preset.base_longitude;
            
            // TODO: apply GPS randomization if enabled
            if (settings_.randomize_gps && settings_.gps_radius_meters > 0.0f) {
                // Haversine formula + random bearing stub
                Logger::debug("GPS randomization stub (not yet implemented)");
            }
        }
        
        // Timestamp handling
        // TODO: apply timestamp modes + shift
        if (!settings_.use_current_time && !settings_.custom_datetime.empty()) {
            Logger::debug("Custom datetime stub: {}", settings_.custom_datetime);
        }
        if (settings_.shift_timestamp_per_file && settings_.shift_seconds != 0) {
            Logger::debug("Timestamp shift stub: +{} sec", settings_.shift_seconds);
        }
        
        // Process image
        auto result = proc::ImageProcessor::process(input_path, output_path, exif);
        if (result) {
            ++success_count;
            Logger::info("Processed: {} -> {}", input_path.filename().string(), output_filename);
        } else {
            ++error_count;
            Logger::error("Failed: {} | {}", input_path.filename().string(), result.error());
        }
        
        processed_count_++;
        progress_percent_ = static_cast<float>(processed_count_.load()) / static_cast<float>(total_files_.load());
    }
    
    processing_ = false;
    status_message_ = std::format("Batch complete | Success: {} | Errors: {}", success_count, error_count);
    Logger::info("Batch finished: {} success, {} errors", success_count, error_count);
}

} // namespace aw::spoofiq::ui
