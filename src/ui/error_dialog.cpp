#include <aw/spoofiq/ui/error_dialog.hpp>
#include <imgui.h>

namespace aw::spoofiq::ui {

void ErrorDialog::show(const std::string& title, const std::string& message) {
    title_ = title;
    message_ = message;
    open_ = true;
}

bool ErrorDialog::is_open() {
    return open_;
}

void ErrorDialog::render() {
    if (!open_) return;
    
    ImGui::OpenPopup("Error");
    
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("%s", message_.c_str());
        ImGui::Separator();
        
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            open_ = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

} // namespace aw::spoofiq::ui
