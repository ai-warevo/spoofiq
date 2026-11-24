#include <aw/spoofiq/proc/async_batch_processor.hpp>
#include <aw/spoofiq/logging/logger.hpp>
#include <format>

namespace aw::spoofiq::proc {

aw::tpl::threading::tasks::Task<BatchOrchestrator::BatchResult> AsyncBatchProcessor::process_batch_async(
    const std::vector<std::filesystem::path>& input_files,
    const std::filesystem::path& output_dir,
    const DevicePreset& device_preset,
    int start_index,
    std::atomic<bool>& cancel_flag,
    std::shared_ptr<aw::tobus::IEventBus> event_bus) {

    BatchOrchestrator::BatchResult result;
    std::atomic<int> processed_count{0};
    int total_files = static_cast<int>(input_files.size());

    Logger::info("Async batch started: {} files, preset: {}", total_files, device_preset.name);

    // Publish initial progress
    publish_event(event_bus, core::ProgressEvent{
        .total_files = total_files,
        .processed_count = 0,
        .current_filename = "",
        .progress_percent = 0.0f
    });

    // Execute batch processing (synchronous for now; can be parallelized with when_all)
    auto batch_result = BatchOrchestrator::process_batch(
        input_files,
        output_dir,
        device_preset,
        start_index,
        cancel_flag,
        processed_count,
        total_files
    );

    if (!batch_result) {
        std::string error_msg = batch_result.error();
        Logger::error("Batch processing failed: {}", error_msg);
        
        // Publish error context in event
        publish_event(event_bus, core::BatchCompletedEvent{
            .total_images = 0,
            .total_videos = 0,
            .total_errors = 0,
            .error_code = 1,
            .error_message = error_msg
        });
        co_return result;
    }

    result = std::move(*batch_result);

    // Publish completion event with error details
    std::string aggregate_errors;
    if (!result.error_messages.empty()) {
        // Aggregate first 3 errors for display
        aggregate_errors = result.error_messages[0];
        if (result.error_messages.size() > 1) {
            aggregate_errors += std::format(" (+{} more)", result.error_messages.size() - 1);
        }
    }

    publish_event(event_bus, core::BatchCompletedEvent{
        .total_images = result.success_count,
        .total_videos = 0,
        .total_errors = result.error_count,
        .error_code = result.error_count > 0 ? 1 : 0,
        .error_message = aggregate_errors
    });

    Logger::info("Async batch completed: {} success, {} errors", result.success_count, result.error_count);

    co_return result;
}

} // namespace aw::spoofiq::proc
