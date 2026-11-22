#pragma once

#include <aw/spoofiq/core/types.hpp>
#include <aw/spoofiq/config_manager.hpp>
#include <memory>
#include <string>
#include <vector>

namespace aw::spoofiq::ui {

class PresetsPanel : public IPanel {
public:
    explicit PresetsPanel();
    ~PresetsPanel();
    
    void render() override;
    PanelId id() const override { return PanelId::PresetsDevices; }
    std::string name() const override { return "Presets & Devices"; }

private:
    // Settings (loaded from config)
    AppSettings settings_;
    
    // UI state
    int selected_preset_idx_ = 0;
    bool unsaved_changes_ = false;
    
    // Edited preset (copy of selected, modified in UI)
    DevicePreset edited_preset_;
    
    // Buffers for new preset creation
    char new_preset_name_[128] = {0};
    
    void render_preset_list();
    void render_preset_editor();
    
    void create_new_preset();
    void duplicate_current_preset();
    void delete_current_preset();
    void apply_edited_preset();
    void reset_edited_preset();
    void save_to_config();
};

} // namespace aw::spoofiq::ui
