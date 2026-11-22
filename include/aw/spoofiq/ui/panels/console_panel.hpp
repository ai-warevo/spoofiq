#pragma once

#include <aw/spoofiq/core/types.hpp>
#include <aw/tobus/subscription.hpp>
#include <vector>
#include <string>

// Forward declarations
namespace aw::spoofiq::core {
    struct ProgressEvent;
    struct FileFinishedEvent;
    struct BatchCompletedEvent;
}

namespace aw::spoofiq::ui {

struct LogEntry {
    std::string message;
    int level = 0; // 0=info, 1=warn, 2=error
};

class ConsolePanel : public IPanel {
public:
    ConsolePanel();
    ~ConsolePanel();
    void render() override;
    PanelId id() const override;
    std::string name() const override { return "Console"; }
    
    int get_unread_count() const { return unread_count_; }
    void mark_read() { unread_count_ = 0; }

private:
    bool auto_scroll_ = true;
    std::vector<LogEntry> log_buffer_;
    int unread_count_ = 0;
    
    // Event subscriptions
    aw::tobus::Subscription progress_sub_;
    aw::tobus::Subscription file_finished_sub_;
    aw::tobus::Subscription batch_completed_sub_;
    
    void on_progress_event(const aw::spoofiq::core::ProgressEvent& event);
    void on_file_finished_event(const aw::spoofiq::core::FileFinishedEvent& event);
    void on_batch_completed_event(const aw::spoofiq::core::BatchCompletedEvent& event);
};

} // namespace aw::spoofiq::ui
