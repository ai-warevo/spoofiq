#include <aw/spoofiq/ui/panels/appearance_panel.hpp>
#include <imgui.h>
#include <aw/spoofiq/logging/logger.hpp>

namespace aw::spoofiq::ui {

AppearancePanel::AppearancePanel() {
    auto result = ConfigManager::load();
    if (result.has_value()) {
        settings_ = result.value();
    }
}

void AppearancePanel::apply_theme(const std::string& theme_name) {
    ImGuiStyle& style = ImGui::GetStyle();
    
    style.WindowRounding = 8.0f;
    style.FrameRounding = 999.0f;  // Pill-shaped buttons
    style.ScrollbarRounding = 999.0f;
    style.GrabRounding = 999.0f;
    style.TabRounding = 4.0f;
    
    ImVec4* colors = style.Colors;
    
    if (theme_name == "Dark") {
        ImGui::StyleColorsDark();
        colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.05f, 0.07f, 1.00f);       // #0A0D12
        colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.08f, 0.11f, 1.00f);        // #0F131C
        colors[ImGuiCol_FrameBg] = ImVec4(0.09f, 0.11f, 0.17f, 1.00f);        // #161D2B
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.16f, 0.21f, 1.00f); // #1E2636
        colors[ImGuiCol_TitleBg] = ImVec4(0.02f, 0.03f, 0.05f, 1.00f);        // #05070C
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.04f, 0.05f, 0.07f, 1.00f);  // #0A0D12
    } else if (theme_name == "Light") {
        ImGui::StyleColorsLight();
    } else if (theme_name == "Midnight") {
        ImGui::StyleColorsDark();
        colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.04f, 1.00f);       // Very dark blue
        colors[ImGuiCol_ChildBg] = ImVec4(0.03f, 0.03f, 0.06f, 1.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.10f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.08f, 0.08f, 0.15f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.01f, 0.01f, 0.02f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.02f, 0.02f, 0.04f, 1.00f);
    }
    
    ImVec4 accent(settings_.accent_color[0], settings_.accent_color[1], settings_.accent_color[2], 1.0f);
    colors[ImGuiCol_Button] = ImVec4(accent.x, accent.y, accent.z, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(accent.x, accent.y, accent.z, 0.60f);
    colors[ImGuiCol_ButtonActive] = ImVec4(accent.x, accent.y, accent.z, 0.80f);
    colors[ImGuiCol_Header] = ImVec4(accent.x, accent.y, accent.z, 0.30f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(accent.x, accent.y, accent.z, 0.50f);
    colors[ImGuiCol_HeaderActive] = ImVec4(accent.x, accent.y, accent.z, 0.70f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(accent.x, accent.y, accent.z, 0.30f);
}

void AppearancePanel::apply_accent_color() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    ImVec4 accent(settings_.accent_color[0], settings_.accent_color[1], settings_.accent_color[2], 1.0f);
    
    colors[ImGuiCol_Button] = ImVec4(accent.x, accent.y, accent.z, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(accent.x, accent.y, accent.z, 0.60f);
    colors[ImGuiCol_ButtonActive] = ImVec4(accent.x, accent.y, accent.z, 0.80f);
    colors[ImGuiCol_Header] = ImVec4(accent.x, accent.y, accent.z, 0.30f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(accent.x, accent.y, accent.z, 0.50f);
    colors[ImGuiCol_HeaderActive] = ImVec4(accent.x, accent.y, accent.z, 0.70f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(accent.x, accent.y, accent.z, 0.30f);
}

void AppearancePanel::apply_ui_scale() {
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = settings_.ui_scale;
}

void AppearancePanel::save_settings() {
    auto result = ConfigManager::save(settings_);
    if (!result.has_value()) {
        Logger::error("Failed to save appearance settings: {}", result.error());
    }
}

void AppearancePanel::render() {
    ImGui::Text("User Appearance Settings");
    ImGui::Separator();
    
    static const char* themes[] = {"Dark", "Light", "Midnight"};
    static int theme_idx = 0;
    
    for (int i = 0; i < 3; ++i) {
        if (settings_.theme == themes[i]) {
            theme_idx = i;
            break;
        }
    }
    
    if (ImGui::Combo("Theme", &theme_idx, themes, 3)) {
        settings_.theme = themes[theme_idx];
        apply_theme(settings_.theme);
        save_settings();
    }
    
    ImGui::Spacing();
    
    ImGui::Text("Accent Color");
    if (ImGui::ColorEdit3("##accent", settings_.accent_color, ImGuiColorEditFlags_PickerHueWheel)) {
        apply_accent_color();
        save_settings();
    }
    
    ImGui::Spacing();
    
    ImGui::Text("UI Scale");
    if (ImGui::SliderFloat("##scale", &settings_.ui_scale, 0.8f, 1.5f, "%.1fx")) {
        apply_ui_scale();
        save_settings();
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Changes are automatically saved to config.json");
}

} // namespace aw::spoofiq::ui
