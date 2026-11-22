#include <aw/spoofiq/ui/panels/console_panel.hpp>
#include <aw/spoofiq/logger.hpp>
#include <imgui.h>

namespace aw::spoofiq::ui {

ConsolePanel::ConsolePanel() = default;

PanelId ConsolePanel::id() const {
    return PanelId::Console;
}

void ConsolePanel::render() {
    auto sink = Logger::console_sink();
    auto logs = sink->get_logs();
    
    if (ImGui::Button("Clear")) {
        sink->clear_logs();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &auto_scroll_);
    
    ImGui::Separator();
    
    ImGui::BeginChild("LogRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& log : logs) {
        ImGui::TextUnformatted(log.c_str());
    }
    if (auto_scroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
}

} // namespace aw::spoofiq::ui
