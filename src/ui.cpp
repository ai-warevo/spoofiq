#include <aw/spoofiq/ui.hpp>
#include <aw/spoofiq/config.hpp>
#include <aw/spoofiq/core/types.hpp>

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

        // Main tab bar with only our 3 tabs
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

            // Tab 2: Presets & Devices (stub for Alpha 2)
            if (ImGui::BeginTabItem("Presets & Devices")) {
                ImGui::TextWrapped("Device preset management coming in Alpha 2 (v0.3.0)");
                ImGui::Spacing();
                ImGui::Bullet(); ImGui::Text("Create/edit/delete device presets");
                ImGui::Bullet(); ImGui::Text("iPhone 16 Pro Max (default)");
                ImGui::Bullet(); ImGui::Text("Custom EXIF/GPS profiles");
                ImGui::EndTabItem();
            }

            // Tab 3: Console (stub for Beta 1)
            if (ImGui::BeginTabItem("Console")) {
                ImGui::TextWrapped("Real-time logging console coming in Beta 1 (v0.5.0)");
                ImGui::Spacing();
                ImGui::Bullet(); ImGui::Text("spdlog integration");
                ImGui::Bullet(); ImGui::Text("ImGuiConsoleSink with ring buffer");
                ImGui::Bullet(); ImGui::Text("Filterable log levels");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::PopStyleVar(2);
    } else {
        ImGui::PopStyleVar(3);
    }
    ImGui::End();
}

} // namespace aw::spoofiq
