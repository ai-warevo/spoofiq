#ifndef AW_SPOOFIQ_UI_PANELS_BATCH_SPOOFER_PANEL_HPP
#define AW_SPOOFIQ_UI_PANELS_BATCH_SPOOFER_PANEL_HPP

#include <aw/spoofiq/core/types.hpp>
#include <aw/spoofiq/core/metadata.hpp>
#include <aw/spoofiq/core/events.hpp>
#include <aw/spoofiq/config/config_manager.hpp>
#include <aw/tpl/threading/tasks/task.hpp>
#include <aw/tobus/tobus.hpp>
#include <filesystem>
#include <string>
#include <vector>
#include <atomic>
#include <memory>

namespace aw::spoofiq::ui {

class BatchSpooferPanel : public IPanel {
public:
    explicit BatchSpooferPanel();
    ~BatchSpooferPanel();
    
    PanelId id() const override { return PanelId::BatchSpoofer; }
    std::string name() const override { return "Batch Spoofer"; }
    void render() override;

private:
    AppSettings settings_;
    
    std::string input_folder_;
    std::vector<std::string> available_presets_; // cached preset names for combo
    int selected_preset_idx_ = 0;
    std::string status_message_;
    std::atomic<bool> processing_{false};
    
    std::atomic<int> total_files_{0};
    std::atomic<int> processed_count_{0};
    std::atomic<float> progress_percent_{0.0f};
    
    std::atomic<bool> cancel_flag_{false};
    
    // Event bus for receiving updates
    std::shared_ptr<aw::tobus::IEventBus> event_bus_;
    
    void refresh_preset_list();
    void start_batch_processing();
    void handle_progress_event(const core::ProgressEvent& event);
    void handle_file_finished_event(const core::FileFinishedEvent& event);
    void handle_batch_completed_event(const core::BatchCompletedEvent& event);
};

} // namespace aw::spoofiq::ui

#endif // AW_SPOOFIQ_UI_PANELS_BATCH_SPOOFER_PANEL_HPP

