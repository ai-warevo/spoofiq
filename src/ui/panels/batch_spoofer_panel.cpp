#include <aw/spoofiq/ui/panels/batch_spoofer_panel.hpp>
#include <aw/spoofiq/event_bus_manager.hpp>
#include <aw/spoofiq/core/events.hpp>
#include <imgui.h>
#include <format>
#include <sstream>

namespace aw::spoofiq::ui {

BatchSpooferPanel::BatchSpooferPanel()
    : input_paths_buffer_(4096, '\0')
    , output_dir_(256, '\0') {
    
    // Default output directory
    output_dir_ = "./spoofiq_output";
    
    // Default iPhone 16 Pro Max metadata
    exif_data_ = core::ExifData{}; // uses defaults from struct
}

BatchSpooferPanel::~BatchSpooferPanel() {
    // Signal cancellation and wait for worker to finish
    cancel_flag_.store(true, std::memory_order_release);
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

void BatchSpooferPanel::render() {
    ImGui::Text("Batch Spoofer v" APP_VERSION);
    ImGui::Separator();
    
    ImGui::Spacing();
    
    bool is_processing = processing_.load(std::memory_order_relaxed);
    
    // Input files (manual list entry)
    ImGui::Text("Input Files (one per line):");
    ImGui::BeginDisabled(is_processing);
    ImGui::InputTextMultiline("##input", input_paths_buffer_.data(), input_paths_buffer_.capacity(), ImVec2(-1, 100));
    ImGui::EndDisabled();
    
    ImGui::Spacing();
    
    // Output directory
    ImGui::Text("Output Directory:");
    ImGui::SetNextItemWidth(-1);
    ImGui::BeginDisabled(is_processing);
    ImGui::InputText("##output", output_dir_.data(), output_dir_.capacity());
    ImGui::EndDisabled();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Metadata display (read-only for MVP)
    ImGui::Text("Metadata (hardcoded for MVP):");
    ImGui::BeginChild("MetadataPreview", ImVec2(0, 150), true);
    ImGui::TextColored(ImVec4(0.22f, 0.74f, 0.97f, 1.0f), "Device:");
    ImGui::Text("  Make: %s", exif_data_.make.c_str());
    ImGui::Text("  Model: %s", exif_data_.model.c_str());
    ImGui::Text("  Software: iOS %s", exif_data_.software.c_str());
    
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.22f, 0.74f, 0.97f, 1.0f), "Camera:");
    ImGui::Text("  F-Number: f/%.2f", exif_data_.fnumber);
    ImGui::Text("  ISO: %d", exif_data_.iso_speed);
    ImGui::Text("  Focal Length: %.2fmm", exif_data_.focal_length);
    ImGui::Text("  Lens: %s", exif_data_.lens_model.c_str());
    
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.22f, 0.74f, 0.97f, 1.0f), "Location:");
    ImGui::Text("  GPS: 37°20'5\"N 122°0'32\"W");
    ImGui::Text("  Altitude: 30m");
    ImGui::EndChild();
    
    ImGui::Spacing();
    
    // Process button (or Cancel if running)
    if (is_processing) {
        if (ImGui::Button("Cancel Batch", ImVec2(200, 40))) {
            cancel_flag_.store(true, std::memory_order_release);
        }
    } else {
        if (ImGui::Button("Process Batch", ImVec2(200, 40))) {
            process_batch();
        }
    }
    
    ImGui::SameLine();
    
    // Status message
    if (!status_message_.empty()) {
        if (status_message_.find("Success") != std::string::npos) {
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "%s", status_message_.c_str());
        } else if (status_message_.find("Error") != std::string::npos) {
            ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "%s", status_message_.c_str());
        } else if (status_message_.find("Cancelled") != std::string::npos) {
            ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.4f, 1.0f), "%s", status_message_.c_str());
        } else {
            ImGui::Text("%s", status_message_.c_str());
        }
    }
    
    ImGui::Spacing();
    
    // Progress bar (if processing)
    if (is_processing) {
        float progress = progress_percent_.load(std::memory_order_relaxed);
        int processed = processed_count_.load(std::memory_order_relaxed);
        int total = total_files_.load(std::memory_order_relaxed);
        
        ImGui::ProgressBar(progress / 100.0f, ImVec2(-1, 0));
        ImGui::Text("File %d of %d (%.0f%%)", processed, total, progress);
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Next Index: IMG_%04d.jpg", next_index_.load(std::memory_order_relaxed));
}

void BatchSpooferPanel::process_batch() {
    // Join previous worker if any
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
    
    processing_.store(true, std::memory_order_release);
    cancel_flag_.store(false, std::memory_order_release);
    status_message_ = "Processing batch...";
    total_files_.store(0, std::memory_order_release);
    processed_count_.store(0, std::memory_order_release);
    progress_percent_.store(0.0f, std::memory_order_release);
    
    // Parse input paths (one per line)
    std::vector<std::filesystem::path> input_files;
    std::string line;
    std::istringstream stream(input_paths_buffer_);
    while (std::getline(stream, line)) {
        if (!line.empty()) {
            input_files.emplace_back(line);
        }
    }
    
    if (input_files.empty()) {
        status_message_ = "Error: No input files provided";
        processing_.store(false, std::memory_order_release);
        return;
    }
    
    std::filesystem::path output(output_dir_.c_str());
    
    // Create output directory
    try {
        std::filesystem::create_directories(output);
    } catch (const std::exception& e) {
        status_message_ = std::format("Error: Failed to create output dir: {}", e.what());
        processing_.store(false, std::memory_order_release);
        return;
    }
    
    total_files_.store(static_cast<int>(input_files.size()), std::memory_order_release);
    
    // Spawn worker thread
    worker_thread_ = std::thread([this, input_files = std::move(input_files), output]() {
        process_batch_worker(std::move(input_files), output);
    });
}

void BatchSpooferPanel::process_batch_worker(
    std::vector<std::filesystem::path> input_files,
    std::filesystem::path output_dir
) {
    auto bus = EventBusManager::get();
    int total = static_cast<int>(input_files.size());
    int success_count = 0;
    int error_count = 0;
    
    for (int i = 0; i < total; ++i) {
        // Check cancellation
        if (cancel_flag_.load(std::memory_order_relaxed)) {
            bus->publish(core::BatchCancelledEvent{});
            status_message_ = "Batch cancelled";
            processing_.store(false, std::memory_order_release);
            return;
        }
        
        const auto& input = input_files[i];
        
        // Update progress
        processed_count_.store(i, std::memory_order_release);
        progress_percent_.store((100.0f * i) / total, std::memory_order_release);
        
        // Publish progress event
        bus->publish(core::ProgressEvent{
            .total_files = total,
            .processed_count = i,
            .current_filename = input.filename().string(),
            .progress_percent = (100.0f * i) / total
        });
        
        // Check file exists
        if (!std::filesystem::exists(input)) {
            std::string error_msg = std::format("Not found: {}", input.string());
            error_count++;
            
            bus->publish(core::FileFinishedEvent{
                .input_filename = input.filename().string(),
                .output_filename = "",
                .success = false,
                .error_message = error_msg
            });
            continue;
        }
        
        // Generate output filename
        int idx = next_index_.fetch_add(1, std::memory_order_relaxed);
        std::string filename = std::format("IMG_{:04d}.jpg", idx);
        std::filesystem::path output_file = output_dir / filename;
        
        // Process file
        auto result = proc::ImageProcessor::process(input, output_file, exif_data_);
        
        if (!result) {
            error_count++;
            bus->publish(core::FileFinishedEvent{
                .input_filename = input.filename().string(),
                .output_filename = filename,
                .success = false,
                .error_message = result.error()
            });
        } else {
            success_count++;
            bus->publish(core::FileFinishedEvent{
                .input_filename = input.filename().string(),
                .output_filename = filename,
                .success = true,
                .error_message = ""
            });
        }
    }
    
    // Final progress
    processed_count_.store(total, std::memory_order_release);
    progress_percent_.store(100.0f, std::memory_order_release);
    
    // Final status
    if (error_count == 0) {
        status_message_ = std::format("✓ All {} files processed successfully", success_count);
    } else {
        status_message_ = std::format("Processed: {} success, {} errors", success_count, error_count);
    }
    
    // Publish batch completed event
    bus->publish(core::BatchCompletedEvent{
        .total_images = success_count,
        .total_videos = 0,
        .total_errors = error_count
    });
    
    processing_.store(false, std::memory_order_release);
}

} // namespace aw::spoofiq::ui
