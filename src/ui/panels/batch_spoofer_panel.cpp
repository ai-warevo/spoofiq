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

void BatchSpooferPanel::render() {
    ImGui::Text("Batch Spoofer v" APP_VERSION);
    ImGui::Separator();
    
    ImGui::Spacing();
    
    // Input files (manual list entry)
    ImGui::Text("Input Files (one per line):");
    ImGui::InputTextMultiline("##input", input_paths_buffer_.data(), input_paths_buffer_.capacity(), ImVec2(-1, 100));
    
    ImGui::Spacing();
    
    // Output directory
    ImGui::Text("Output Directory:");
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##output", output_dir_.data(), output_dir_.capacity());
    
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
    
    // Process button
    ImGui::BeginDisabled(processing_);
    if (ImGui::Button("Process Batch", ImVec2(200, 40))) {
        process_batch();
    }
    ImGui::EndDisabled();
    
    ImGui::SameLine();
    
    // Status message
    if (!status_message_.empty()) {
        if (status_message_.find("Success") != std::string::npos) {
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "%s", status_message_.c_str());
        } else if (status_message_.find("Error") != std::string::npos) {
            ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "%s", status_message_.c_str());
        } else {
            ImGui::Text("%s", status_message_.c_str());
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Next Index: IMG_%04d.jpg", next_index_);
}

void BatchSpooferPanel::process_batch() {
    processing_ = true;
    status_message_ = "Processing batch...";
    
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
        processing_ = false;
        return;
    }
    
    std::filesystem::path output(output_dir_.c_str());
    
    // Create output directory
    try {
        std::filesystem::create_directories(output);
    } catch (const std::exception& e) {
        status_message_ = std::format("Error: Failed to create output dir: {}", e.what());
        processing_ = false;
        return;
    }
    
    // Get event bus
    auto bus = EventBusManager::get();
    int total_files = static_cast<int>(input_files.size());
    
    // Process each file
    int success_count = 0;
    int error_count = 0;
    std::string last_error;
    
    for (int i = 0; i < total_files; ++i) {
        const auto& input = input_files[i];
        
        // Publish progress event
        bus->publish(core::ProgressEvent{
            .total_files = total_files,
            .processed_count = i,
            .current_filename = input.filename().string(),
            .progress_percent = (100.0f * i) / total_files
        });
        
        if (!std::filesystem::exists(input)) {
            last_error = std::format("Not found: {}", input.string());
            error_count++;
            
            // Publish failure event
            bus->publish(core::FileFinishedEvent{
                .input_filename = input.filename().string(),
                .output_filename = "",
                .success = false,
                .error_message = last_error
            });
            continue;
        }
        
        // Generate output filename: IMG_XXXX.jpg
        std::string filename = std::format("IMG_{:04d}.jpg", next_index_);
        std::filesystem::path output_file = output / filename;
        
        // Process
        auto result = proc::ImageProcessor::process(input, output_file, exif_data_);
        
        if (!result) {
            last_error = result.error();
            error_count++;
            
            // Publish failure event
            bus->publish(core::FileFinishedEvent{
                .input_filename = input.filename().string(),
                .output_filename = filename,
                .success = false,
                .error_message = last_error
            });
        } else {
            success_count++;
            
            // Publish success event
            bus->publish(core::FileFinishedEvent{
                .input_filename = input.filename().string(),
                .output_filename = filename,
                .success = true,
                .error_message = ""
            });
            
            next_index_++;
        }
    }
    
    // Final status
    if (error_count == 0) {
        status_message_ = std::format("✓ All {} files processed successfully", success_count);
    } else {
        status_message_ = std::format("Processed: {} success, {} errors. Last error: {}", 
                                     success_count, error_count, last_error);
    }
    
    // Publish batch completed event
    bus->publish(core::BatchCompletedEvent{
        .total_images = success_count,
        .total_videos = 0,
        .total_errors = error_count
    });
    
    processing_ = false;
}

} // namespace aw::spoofiq::ui
