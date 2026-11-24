#pragma once

#include <aw/spoofiq/proc/batch_orchestrator.hpp>
#include <aw/spoofiq/core/events.hpp>
#include <aw/tpl/threading/tasks/task.hpp>
#include <aw/tobus/tobus.hpp>
#include <memory>
#include <vector>
#include <filesystem>
#include <atomic>

namespace aw::spoofiq::proc {

/// Async batch processor using aw::tpl coroutines and aw::tobus for event publishing.
/// Separates batch processing from UI thread while maintaining thread-safe communication.
class AsyncBatchProcessor {
public:
    /// Start async batch processing.
    /// Returns a Task that can be awaited or spawned on executor.
    ///
    /// Events published to event bus:
    /// - ProgressEvent (each file or batch of files)
    /// - FileFinishedEvent (per file)
    /// - BatchCompletedEvent (at end)
    /// - BatchCancelledEvent (if cancelled)
    static aw::tpl::threading::tasks::Task<BatchOrchestrator::BatchResult> process_batch_async(
        const std::vector<std::filesystem::path>& input_files,
        const std::filesystem::path& output_dir,
        const DevicePreset& device_preset,
        int start_index,
        std::atomic<bool>& cancel_flag,
        std::shared_ptr<aw::tobus::IEventBus> event_bus);

private:
    /// Publish event to event bus (safe no-op if bus is null).
    template <typename Event>
    static void publish_event(std::shared_ptr<aw::tobus::IEventBus> bus, const Event& event) {
        if (bus) {
            bus->publish(event);
        }
    }
};

} // namespace aw::spoofiq::proc
