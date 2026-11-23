#include <aw/spoofiq/ui/panels/console_panel.hpp>
#include <aw/spoofiq/logger.hpp>
#include <aw/spoofiq/event_bus_manager.hpp>
#include <aw/spoofiq/core/events.hpp>
#include <aw/tpl/threading/tasks/task.hpp>
#include <imgui.h>
#include <format>
#include <ctime>
#include <iomanip>
#include <sstream>

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

std::string ConsolePanel::format_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now = {};
#ifdef _WIN32
    localtime_s(&tm_now, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_now);
#endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%H:%M:%S");
    return oss.str();
}

void ConsolePanel::add_log(const std::string& message, int level) {
    log_buffer_.emplace_back(LogEntry{
        .message = message,
        .timestamp = format_timestamp(),
        .level = level
    });
    unread_count_++;
}

void ConsolePanel::on_progress_event(const core::ProgressEvent& event) {
    add_log(std::format("[Progress] File {} of {} | {:.0f}%",
                       event.processed_count + 1,
                       event.total_files,
                       event.progress_percent), 0);
}

void ConsolePanel::on_file_finished_event(const core::FileFinishedEvent& event) {
    if (event.success) {
        add_log(std::format("✓ Processed: {} → {}",
                           event.input_filename,
                           event.output_filename), 0);
    } else {
        add_log(std::format("✗ Failed: {} - {}",
                           event.input_filename,
                           event.error_message), 2);
    }
}

void ConsolePanel::on_batch_completed_event(const core::BatchCompletedEvent& event) {
    add_log(std::format("Batch completed: {} images, {} videos, {} errors",
                       event.total_images,
                       event.total_videos,
                       event.total_errors),
           event.total_errors > 0 ? 1 : 0);
}

void ConsolePanel::render() {
    // Mark as read when user views this tab
    unread_count_ = 0;
    
    // Top toolbar: filters + search + buttons
    ImGui::Text("Filters:");
    ImGui::SameLine();
    ImGui::Checkbox("Info", &show_info_);
    ImGui::SameLine();
    ImGui::Checkbox("Warn", &show_warn_);
    ImGui::SameLine();
    ImGui::Checkbox("Error", &show_error_);
    
    ImGui::SameLine(0, 20);
    ImGui::Text("Search:");
    ImGui::SameLine();
    search_filter_.Draw("##search", 200.0f);
    
    ImGui::SameLine(0, 20);
    if (ImGui::Button("Clear", ImVec2(60, 0))) {
        log_buffer_.clear();
        unread_count_ = 0;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Copy All", ImVec2(80, 0))) {
        std::string clipboard_text;
        for (const auto& entry : log_buffer_) {
            clipboard_text += std::format("[{}] {}\n", entry.timestamp, entry.message);
        }
        ImGui::SetClipboardText(clipboard_text.c_str());
    }
    
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &auto_scroll_);
    
    ImGui::Separator();
    
    // Log display region
    ImGui::BeginChild("LogRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    for (const auto& entry : log_buffer_) {
        // Apply level filter
        if ((entry.level == 0 && !show_info_) ||
            (entry.level == 1 && !show_warn_) ||
            (entry.level == 2 && !show_error_)) {
            continue;
        }
        
        // Apply search filter
        std::string full_line = std::format("[{}] {}", entry.timestamp, entry.message);
        if (!search_filter_.PassFilter(full_line.c_str())) {
            continue;
        }
        
        // Color based on level
        ImVec4 color;
        switch (entry.level) {
            case 1: // warn
                color = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);  // yellow
                break;
            case 2: // error
                color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);   // red
                break;
            default: // info
                color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);   // light gray
        }
        
        ImGui::TextColored(color, "[%s] %s", entry.timestamp.c_str(), entry.message.c_str());
    }
    
    if (auto_scroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    
    ImGui::EndChild();
}

} // namespace aw::spoofiq::ui
