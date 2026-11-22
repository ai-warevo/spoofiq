#include <aw/spoofiq/ui/panels/console_panel.hpp>
#include <aw/spoofiq/logger.hpp>
#include <aw/spoofiq/event_bus_manager.hpp>
#include <aw/spoofiq/core/events.hpp>
#include <aw/tpl/threading/tasks/task.hpp>
#include <imgui.h>
#include <format>

namespace aw::spoofiq::ui {

ConsolePanel::ConsolePanel() {
    // Subscribe to events from event bus
    auto bus = EventBusManager::get();
    
    progress_sub_ = bus->subscribe<core::ProgressEvent>(
        [this](const core::ProgressEvent& event) -> aw::tpl::threading::tasks::Task<void> {
            on_progress_event(event);
            co_return;
        });
    
    file_finished_sub_ = bus->subscribe<core::FileFinishedEvent>(
        [this](const core::FileFinishedEvent& event) -> aw::tpl::threading::tasks::Task<void> {
            on_file_finished_event(event);
            co_return;
        });
    
    batch_completed_sub_ = bus->subscribe<core::BatchCompletedEvent>(
        [this](const core::BatchCompletedEvent& event) -> aw::tpl::threading::tasks::Task<void> {
            on_batch_completed_event(event);
            co_return;
        });
}

ConsolePanel::~ConsolePanel() = default;

PanelId ConsolePanel::id() const {
    return PanelId::Console;
}

void ConsolePanel::on_progress_event(const core::ProgressEvent& event) {
    log_buffer_.emplace_back(LogEntry{
        .message = std::format("[Progress] File {} of {} | {:.0f}%",
                              event.processed_count + 1,
                              event.total_files,
                              event.progress_percent),
        .level = 0  // info
    });
    unread_count_++;
}

void ConsolePanel::on_file_finished_event(const core::FileFinishedEvent& event) {
    if (event.success) {
        log_buffer_.emplace_back(LogEntry{
            .message = std::format("✓ Processed: {} → {}",
                                  event.input_filename,
                                  event.output_filename),
            .level = 0  // info
        });
    } else {
        log_buffer_.emplace_back(LogEntry{
            .message = std::format("✗ Failed: {} - {}",
                                  event.input_filename,
                                  event.error_message),
            .level = 2  // error
        });
    }
    unread_count_++;
}

void ConsolePanel::on_batch_completed_event(const core::BatchCompletedEvent& event) {
    log_buffer_.emplace_back(LogEntry{
        .message = std::format("Batch completed: {} images, {} videos, {} errors",
                              event.total_images,
                              event.total_videos,
                              event.total_errors),
        .level = event.total_errors > 0 ? 1 : 0  // warn or info
    });
    unread_count_++;
}

void ConsolePanel::render() {
    // Mark as read when user views this tab
    unread_count_ = 0;
    
    ImGui::Text("Event Log");
    
    ImGui::SameLine();
    if (ImGui::Button("Clear", ImVec2(70, 0))) {
        log_buffer_.clear();
        unread_count_ = 0;
    }
    
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &auto_scroll_);
    
    ImGui::Separator();
    
    ImGui::BeginChild("LogRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    for (const auto& entry : log_buffer_) {
        ImVec4 color;
        switch (entry.level) {
            case 1: // warn
                color = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);  // yellow
                break;
            case 2: // error
                color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);   // red
                break;
            default: // info
                color = ImVec4(0.4f, 0.9f, 0.4f, 1.0f);   // green
        }
        ImGui::TextColored(color, "%s", entry.message.c_str());
    }
    
    if (auto_scroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    
    ImGui::EndChild();
}

} // namespace aw::spoofiq::ui
