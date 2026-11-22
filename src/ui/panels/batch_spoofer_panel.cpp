#include <aw/spoofiq/ui/panels/batch_spoofer_panel.hpp>
#include <imgui.h>
#include <format>

namespace aw::spoofiq::ui {

BatchSpooferPanel::BatchSpooferPanel()
    : input_path_(256, '\0')
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
    
    // Input file path
    ImGui::Text("Input Image:");
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##input", input_path_.data(), input_path_.capacity());
    ImGui::SameLine();
    ImGui::Text("(Supports: JPG, PNG, BMP, TIF, etc.)");
    
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
    if (ImGui::Button("Process Image", ImVec2(200, 40))) {
        process_single_file();
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

void BatchSpooferPanel::process_single_file() {
    processing_ = true;
    status_message_ = "Processing...";
    
    std::filesystem::path input(input_path_.c_str());
    std::filesystem::path output(output_dir_.c_str());
    
    // Validation
    if (!std::filesystem::exists(input)) {
        status_message_ = std::format("Error: Input file not found: {}", input.string());
        processing_ = false;
        return;
    }
    
    // Create output directory
    try {
        std::filesystem::create_directories(output);
    } catch (const std::exception& e) {
        status_message_ = std::format("Error: Failed to create output dir: {}", e.what());
        processing_ = false;
        return;
    }
    
    // Process
    auto result = proc::ImageProcessor::process(input, output, exif_data_, next_index_);
    
    if (!result) {
        status_message_ = std::format("Error: {}", result.error());
        processing_ = false;
        return;
    }
    
    status_message_ = std::format("Success! Created: {}", result.value().filename().string());
    next_index_++;
    processing_ = false;
}

} // namespace aw::spoofiq::ui
