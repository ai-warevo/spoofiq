#include <aw/spoofiq/ui/ui.hpp>
#include <aw/spoofiq/core/config.hpp>
#include <aw/spoofiq/core/types.hpp>
#include <aw/spoofiq/ui/panels/console_panel.hpp>
#include <aw/spoofiq/ui/error_dialog.hpp>
#include <format>

#include <imgui.h>

namespace aw::spoofiq {

    UiRoot::UiRoot(AppState& state, PanelRegistry& registry)
    : state_(state),
      registry_(registry) {}

void UiRoot::render() {
    // Create fullscreen window that fills the entire OS window
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    // Fullscreen window flags: no decorations, no moving, no scrollbars
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                                    ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    if (ImGui::Begin("##MainWindow", nullptr, window_flags)) {
        ImGui::PopStyleVar(3);

        // Add padding back for content
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 8.0f));

        // Application title bar (since we removed OS title bar decorations)
        ImGui::PushFont(ImGui::GetFont()); // Use default font
        ImGui::Text("spoofiq");
        ImGui::SameLine();
        ImGui::TextDisabled("—");
        ImGui::SameLine();
        ImGui::TextDisabled("Batch Media Metadata Spoofer");
        ImGui::PopFont();

        ImGui::Separator();
        ImGui::Spacing();

        // Main tab bar with 4 tabs
        if (ImGui::BeginTabBar("MainTabs", ImGuiTabBarFlags_None)) {
            // Tab 1: Batch Spoofer
            if (ImGui::BeginTabItem("Batch Spoofer")) {
                // Find and render BatchSpooferPanel
                for (const auto& panel : registry_.panels()) {
                    if (panel->name() == "Batch Spoofer") {
                        panel->render();
                        break;
                    }
                }
                ImGui::EndTabItem();
            }

            // Tab 2: Presets & Devices
            if (ImGui::BeginTabItem("Presets & Devices")) {
                if (auto presets_panel = registry_.get_panel(PanelId::PresetsDevices)) {
                    presets_panel->render();
                } else {
                    ImGui::TextWrapped("Presets & Devices panel not registered.");
                }
                ImGui::EndTabItem();
            }

            // Tab 3: Appearance
            if (ImGui::BeginTabItem("Appearance")) {
                if (auto appearance_panel = registry_.get_panel(PanelId::Appearance)) {
                    appearance_panel->render();
                } else {
                    ImGui::TextWrapped("Appearance panel not registered.");
                }
                ImGui::EndTabItem();
            }

            // Tab 4: Console (with unread badge)
            int console_unread = 0;
            if (auto console_panel = registry_.get_panel(PanelId::Console)) {
                if (auto* cp = dynamic_cast<aw::spoofiq::ui::ConsolePanel*>(console_panel.get())) {
                    console_unread = cp->get_unread_count();
                }
            }
            
            std::string console_label = console_unread > 0 
                ? std::format("Console ({})", console_unread)
                : "Console";
            
            if (ImGui::BeginTabItem(console_label.c_str())) {
                if (auto console_panel = registry_.get_panel(PanelId::Console)) {
                    // Mark as read when viewing
                    if (auto* cp = dynamic_cast<aw::spoofiq::ui::ConsolePanel*>(console_panel.get())) {
                        cp->mark_read();
                    }
                    console_panel->render();
                } else {
                    ImGui::TextWrapped("Console panel not registered.");
                }
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::PopStyleVar(2);
    } else {
        ImGui::PopStyleVar(3);
    }
    ImGui::End();
    
    // Render error dialog
    ui::ErrorDialog::render();
}

} // namespace aw::spoofiq
