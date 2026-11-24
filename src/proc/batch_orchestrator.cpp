#include <aw/spoofiq/proc/batch_orchestrator.hpp>
#include <aw/spoofiq/proc/image_processor.hpp>
#include <aw/spoofiq/proc/video_processor.hpp>
#include <aw/spoofiq/logging/logger.hpp>
#include <format>

namespace aw::spoofiq::proc {

int BatchOrchestrator::find_next_index(const std::filesystem::path& output_dir) {
    if (!std::filesystem::exists(output_dir)) {
        return 1;
    }

    int max_index = 0;
    for (const auto& entry : std::filesystem::directory_iterator(output_dir)) {
        if (!entry.is_regular_file()) continue;

        std::string filename = entry.path().stem().string();
        if (filename.starts_with("IMG_") && filename.length() == 8) {
            if (int idx = std::stoi(filename.substr(4)); idx > max_index) {
                max_index = idx;
            }
        }
    }

    return max_index + 1;
}

std::expected<BatchOrchestrator::BatchResult, std::string> BatchOrchestrator::process_batch(
    const std::vector<std::filesystem::path>& input_files,
    const std::filesystem::path& output_dir,
    const DevicePreset& device_preset,
    int start_index,
    std::atomic<bool>& cancel_flag,
    std::atomic<int>& processed_count) {

    BatchResult result;
    std::atomic<int> atomic_index{start_index};

    Logger::info("Batch started: {} files, preset: {}, start index: {}", 
                 input_files.size(), device_preset.name, start_index);

    for (size_t i = 0; i < input_files.size(); ++i) {
        if (cancel_flag.load()) {
            Logger::warn("Batch cancelled after {} files", i);
            break;
        }

        const auto& input_path = input_files[i];
        int file_index = atomic_index.fetch_add(1);

        std::string error_msg;
        if (process_file(input_path, output_dir, device_preset, file_index, error_msg)) {
            ++result.success_count;
            Logger::info("Processed: {} -> IMG_{:04d}.jpg", 
                         input_path.filename().string(), file_index);
        } else {
            ++result.error_count;
            std::string full_error = std::format("Failed: {} | {}", 
                                                input_path.filename().string(), 
                                                error_msg);
            result.error_messages.push_back(full_error);
            Logger::error("{}", full_error);
        }

        processed_count.store(static_cast<int>(i + 1));
    }

    Logger::info("Batch finished: {} success, {} errors", result.success_count, result.error_count);
    return result;
}

bool BatchOrchestrator::process_file(
    const std::filesystem::path& input_path,
    const std::filesystem::path& output_dir,
    const DevicePreset& device_preset,
    int output_index,
    std::string& out_error) {

    std::string output_filename = std::format("IMG_{:04d}.jpg", output_index);
    auto output_path = output_dir / output_filename;

    if (!std::filesystem::exists(input_path)) {
        out_error = std::format("File not found: {}", input_path.string());
        return false;
    }

    // Build metadata from preset
    core::ExifData exif;
    exif.make = device_preset.make;
    exif.model = device_preset.model;
    exif.software = device_preset.software;
    exif.lens_model = device_preset.lens_model;
    exif.focal_length = device_preset.focal_length;
    exif.fnumber = device_preset.f_number;
    exif.iso_speed = static_cast<uint16_t>(device_preset.iso);
    exif.latitude = static_cast<float>(device_preset.base_latitude);
    exif.longitude = static_cast<float>(device_preset.base_longitude);

    // For now, assume image file (video processing deferred to future)
    auto result = ImageProcessor::process(input_path, output_path, exif);
    if (!result) {
        out_error = result.error();
        return false;
    }

    return true;
}

} // namespace aw::spoofiq::proc
