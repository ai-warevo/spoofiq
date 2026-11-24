#pragma once

#include <aw/spoofiq/core/types.hpp>
#include <aw/spoofiq/config/config_manager.hpp>

namespace aw::spoofiq::ui {

class AppearancePanel : public IPanel {
public:
    AppearancePanel();
    PanelId id() const override { return PanelId::Appearance; }
    std::string name() const override { return "Appearance"; }
    void render() override;

private:
    AppSettings settings_;
    
    void apply_theme(const std::string& theme_name);
    void apply_accent_color();
    void apply_ui_scale();
    void save_settings();
};

} // namespace aw::spoofiq::ui
