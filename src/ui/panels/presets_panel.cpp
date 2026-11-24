#include <aw/spoofiq/ui/panels/presets_panel.hpp>
#include <aw/spoofiq/logging/logger.hpp>
#include <imgui.h>
#include <format>
#include <cstring>

namespace aw::spoofiq::ui {

// Cross-platform safe string copy
static inline void safe_strncpy(char* dest, size_t dest_size, const char* src) {
#ifdef _WIN32
    strncpy_s(dest, dest_size, src, _TRUNCATE);
#else
    std::strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
#endif
}

PresetsPanel::PresetsPanel() {
    // Load settings from config
    if (auto result = ConfigManager::load()) {
        settings_ = std::move(*result);
        Logger::info("Presets loaded: {} presets available", settings_.presets.size());
    } else {
        Logger::error("Failed to load config: {}", result.error());
        settings_ = {};
    }
    
    // Initialize edited preset with first one
    if (!settings_.presets.empty()) {
        selected_preset_idx_ = 0;
        edited_preset_ = settings_.presets[0];
    }
}

PresetsPanel::~PresetsPanel() = default;

void PresetsPanel::render() {
    ImGui::Text("Device Preset Management");
    ImGui::Separator();
    ImGui::Spacing();
    
    // Two-column layout
    ImGui::Columns(2, "presetsLayout", true);
    
    // Left pane: preset list
    render_preset_list();
    
    ImGui::NextColumn();
    
    // Right pane: preset editor
    render_preset_editor();
    
    ImGui::Columns(1);
}

void PresetsPanel::render_preset_list() {
    ImGui::Text("Saved Presets");
    ImGui::Separator();
    
    ImGui::BeginChild("PresetList", ImVec2(0, -100), true);
    for (int i = 0; i < static_cast<int>(settings_.presets.size()); ++i) {
        bool selected = (i == selected_preset_idx_);
        if (ImGui::Selectable(settings_.presets[i].name.c_str(), selected)) {
            if (selected_preset_idx_ != i) {
                // Check for unsaved changes
                if (unsaved_changes_) {
                    Logger::warn("Discarding unsaved changes to preset");
                }
                selected_preset_idx_ = i;
                edited_preset_ = settings_.presets[i];
                unsaved_changes_ = false;
            }
        }
        if (selected) {
            ImGui::SetItemDefaultFocus();
        }
    }
    ImGui::EndChild();
    
    // Action buttons
    if (ImGui::Button("+ New", ImVec2(70, 0))) {
        create_new_preset();
    }
    ImGui::SameLine();
    if (ImGui::Button("Duplicate", ImVec2(80, 0))) {
        duplicate_current_preset();
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(selected_preset_idx_ < 0 || settings_.presets.empty());
    if (ImGui::Button("Delete", ImVec2(60, 0))) {
        delete_current_preset();
    }
    ImGui::EndDisabled();
}

void PresetsPanel::render_preset_editor() {
    if (selected_preset_idx_ < 0 || selected_preset_idx_ >= static_cast<int>(settings_.presets.size())) {
        ImGui::Text("No preset selected");
        return;
    }
    
    ImGui::Text("Preset Parameters");
    ImGui::Separator();
    
    ImGui::BeginChild("PresetEditor", ImVec2(0, -100), true);
    
    // Preset Name
    char name_buf[256];
    safe_strncpy(name_buf, sizeof(name_buf), edited_preset_.name.c_str());
    if (ImGui::InputText("Preset Name:", name_buf, sizeof(name_buf))) {
        edited_preset_.name = name_buf;
        unsaved_changes_ = true;
    }
    
    ImGui::Spacing();
    ImGui::Text("Camera Info:");
    
    // Make
    char make_buf[256];
    safe_strncpy(make_buf, sizeof(make_buf), edited_preset_.make.c_str());
    if (ImGui::InputText("Make:", make_buf, sizeof(make_buf))) {
        edited_preset_.make = make_buf;
        unsaved_changes_ = true;
    }
    
    // Model
    char model_buf[256];
    safe_strncpy(model_buf, sizeof(model_buf), edited_preset_.model.c_str());
    if (ImGui::InputText("Model:", model_buf, sizeof(model_buf))) {
        edited_preset_.model = model_buf;
        unsaved_changes_ = true;
    }
    
    // Software
    char software_buf[256];
    safe_strncpy(software_buf, sizeof(software_buf), edited_preset_.software.c_str());
    if (ImGui::InputText("Software:", software_buf, sizeof(software_buf))) {
        edited_preset_.software = software_buf;
        unsaved_changes_ = true;
    }
    
    ImGui::Spacing();
    ImGui::Text("Lens Info:");
    
    // Lens Model
    char lens_buf[512];
    safe_strncpy(lens_buf, sizeof(lens_buf), edited_preset_.lens_model.c_str());
    if (ImGui::InputTextMultiline("Lens Model:", lens_buf, sizeof(lens_buf), ImVec2(0, 60))) {
        edited_preset_.lens_model = lens_buf;
        unsaved_changes_ = true;
    }
    
    ImGui::Spacing();
    ImGui::Text("Camera Specs:");
    
    // Focal length
    if (ImGui::InputFloat("Focal Length (mm):", &edited_preset_.focal_length)) {
        unsaved_changes_ = true;
    }
    ImGui::SameLine();
    if (ImGui::InputFloat("(35mm equiv):", &edited_preset_.focal_length_35mm)) {
        unsaved_changes_ = true;
    }
    
    // Aperture
    if (ImGui::InputFloat("Aperture (f-number):", &edited_preset_.f_number)) {
        unsaved_changes_ = true;
    }
    ImGui::SameLine();
    if (ImGui::InputInt("ISO:", &edited_preset_.iso)) {
        unsaved_changes_ = true;
    }
    
    ImGui::Spacing();
    ImGui::Text("Base GPS:");
    
    // GPS coordinates
    if (ImGui::InputDouble("Latitude:", &edited_preset_.base_latitude, 0.0001, 0.001)) {
        unsaved_changes_ = true;
    }
    ImGui::SameLine();
    if (ImGui::InputDouble("Longitude:", &edited_preset_.base_longitude, 0.0001, 0.001)) {
        unsaved_changes_ = true;
    }
    
    ImGui::EndChild();
    
    // Action buttons
    ImGui::BeginDisabled(!unsaved_changes_);
    if (ImGui::Button("Apply Changes", ImVec2(100, 0))) {
        apply_edited_preset();
    }
    ImGui::EndDisabled();
    
    ImGui::SameLine();
    if (ImGui::Button("Reset", ImVec2(100, 0))) {
        reset_edited_preset();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Save to config.json", ImVec2(140, 0))) {
        save_to_config();
    }
}

void PresetsPanel::create_new_preset() {
    // Show a simple input dialog to get the name
    // For now, just create a copy of the current preset with a new name
    if (settings_.presets.empty()) {
        Logger::warn("Cannot create preset: no presets available");
        return;
    }
    
    DevicePreset new_preset = settings_.presets[selected_preset_idx_];
    new_preset.id = std::format("custom_{}", settings_.presets.size());
    new_preset.name = "New Preset";
    
    settings_.presets.push_back(new_preset);
    selected_preset_idx_ = static_cast<int>(settings_.presets.size()) - 1;
    edited_preset_ = new_preset;
    unsaved_changes_ = false;
    
    Logger::info("Created new preset: {}", new_preset.name);
}

void PresetsPanel::duplicate_current_preset() {
    if (selected_preset_idx_ < 0 || selected_preset_idx_ >= static_cast<int>(settings_.presets.size())) {
        return;
    }
    
    DevicePreset dup = settings_.presets[selected_preset_idx_];
    dup.id = std::format("copy_of_{}", dup.id);
    dup.name = std::format("{} (copy)", dup.name);
    
    settings_.presets.push_back(dup);
    selected_preset_idx_ = static_cast<int>(settings_.presets.size()) - 1;
    edited_preset_ = dup;
    unsaved_changes_ = false;
    
    Logger::info("Duplicated preset: {}", dup.name);
}

void PresetsPanel::delete_current_preset() {
    if (selected_preset_idx_ < 0 || selected_preset_idx_ >= static_cast<int>(settings_.presets.size())) {
        return;
    }
    
    std::string deleted_name = settings_.presets[selected_preset_idx_].name;
    settings_.presets.erase(settings_.presets.begin() + selected_preset_idx_);
    
    if (selected_preset_idx_ >= static_cast<int>(settings_.presets.size()) && !settings_.presets.empty()) {
        --selected_preset_idx_;
    }
    
    if (!settings_.presets.empty()) {
        edited_preset_ = settings_.presets[selected_preset_idx_];
    } else {
        edited_preset_ = {};
        selected_preset_idx_ = -1;
    }
    
    unsaved_changes_ = false;
    Logger::info("Deleted preset: {}", deleted_name);
}

void PresetsPanel::apply_edited_preset() {
    if (selected_preset_idx_ < 0 || selected_preset_idx_ >= static_cast<int>(settings_.presets.size())) {
        return;
    }
    
    settings_.presets[selected_preset_idx_] = edited_preset_;
    unsaved_changes_ = false;
    Logger::info("Applied changes to preset: {}", edited_preset_.name);
}

void PresetsPanel::reset_edited_preset() {
    if (selected_preset_idx_ < 0 || selected_preset_idx_ >= static_cast<int>(settings_.presets.size())) {
        return;
    }
    
    edited_preset_ = settings_.presets[selected_preset_idx_];
    unsaved_changes_ = false;
    Logger::info("Reset to saved preset");
}

void PresetsPanel::save_to_config() {
    // Apply current edits first
    if (unsaved_changes_) {
        apply_edited_preset();
    }
    
    // Save settings to file
    if (auto result = ConfigManager::save(settings_)) {
        Logger::info("Saved {} presets to config.json", settings_.presets.size());
    } else {
        Logger::error("Failed to save config: {}", result.error());
    }
}

} // namespace aw::spoofiq::ui
