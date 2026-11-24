#ifndef AW_SPOOFIQ_UI_PANELS_PRESETS_PANEL_HPP
#define AW_SPOOFIQ_UI_PANELS_PRESETS_PANEL_HPP

#include <aw/spoofiq/core/types.hpp>
#include <aw/spoofiq/config/config_manager.hpp>
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
    AppSettings settings_;
    
    int selected_preset_idx_ = 0;
    bool unsaved_changes_ = false;
    
    DevicePreset edited_preset_;
    
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

#endif // AW_SPOOFIQ_UI_PANELS_PRESETS_PANEL_HPP
