#include <aw/spoofiq/cli/cli_runner.hpp>
#include <aw/spoofiq/proc/batch_orchestrator.hpp>
#include <aw/spoofiq/logging/logger.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <atomic>
#include <format>

namespace aw::spoofiq::cli {

static std::vector<std::filesystem::path> discover_image_files(const std::filesystem::path& folder) {
    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        if (!entry.is_regular_file()) continue;

        auto ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || 
            ext == ".bmp" || ext == ".tif" || ext == ".tiff") {
            files.push_back(entry.path());
        }
    }
    return files;
}

static std::optional<DevicePreset> find_preset_by_id(const std::vector<DevicePreset>& presets, 
                                                       const std::string& preset_id) {
    for (const auto& preset : presets) {
        if (preset.id == preset_id) {
            return preset;
        }
    }
    return std::nullopt;
}

static void write_json_report(const std::filesystem::path& report_path,
                              int total_files,
                              int success_count,
                              int error_count,
                              const std::vector<std::string>& error_messages) {
    nlohmann::json report;
    report["total_files"] = total_files;
    report["success_count"] = success_count;
    report["error_count"] = error_count;
    report["errors"] = nlohmann::json::array();
    
    for (const auto& msg : error_messages) {
        report["errors"].push_back(msg);
    }

    std::ofstream file(report_path);
    file << report.dump(2);
}

int run_cli(const CliConfig& config, const AppSettings& app_settings) {
    Logger::info("CLI mode: Starting batch processing");
    Logger::info("  Input folder: {}", config.input_folder.string());
    Logger::info("  Output dir: {}", config.output_dir.string());
    Logger::info("  Preset ID: {}", config.preset_id);
    Logger::info("  Start index: {}", config.start_index);

    // Discover image files
    auto input_files = discover_image_files(config.input_folder);
    if (input_files.empty()) {
        std::cerr << "Error: No supported image files found in folder\n";
        Logger::error("No supported image files found in: {}", config.input_folder.string());
        return 1;
    }

    Logger::info("Found {} image file(s)", input_files.size());

    // Find preset by ID
    auto preset_opt = find_preset_by_id(app_settings.presets, config.preset_id);
    if (!preset_opt) {
        std::cerr << "Error: Preset not found: " << config.preset_id << "\n";
        Logger::error("Preset not found: {}", config.preset_id);
        return 1;
    }

    DevicePreset preset = *preset_opt;
    Logger::info("Using preset: {} ({})", preset.name, preset.model);

    // Create output directory if needed
    std::error_code ec;
    if (!std::filesystem::exists(config.output_dir)) {
        std::filesystem::create_directories(config.output_dir, ec);
        if (ec) {
            std::cerr << "Error: Failed to create output directory: " << ec.message() << "\n";
            Logger::error("Failed to create output directory: {}", ec.message());
            return 1;
        }
    }

    // Run batch processing
    std::atomic<bool> cancel_flag{false};
    std::atomic<int> processed_count{0};

    auto result = proc::BatchOrchestrator::process_batch(
        input_files,
        config.output_dir,
        preset,
        config.start_index,
        cancel_flag,
        processed_count
    );

    if (!result) {
        std::cerr << "Error: Batch processing failed: " << result.error() << "\n";
        Logger::error("Batch processing failed: {}", result.error());
        return 1;
    }

    auto batch_result = *result;
    int total = static_cast<int>(input_files.size());

    // Log summary
    Logger::info("Batch processing completed:");
    Logger::info("  Total files: {}", total);
    Logger::info("  Successful: {}", batch_result.success_count);
    Logger::info("  Failed: {}", batch_result.error_count);

    // Print to stdout for user
    std::cout << "Batch processing completed:\n";
    std::cout << "  Total files: " << total << "\n";
    std::cout << "  Successful: " << batch_result.success_count << "\n";
    std::cout << "  Failed: " << batch_result.error_count << "\n";

    // Write JSON report if requested
    if (config.report_path) {
        try {
            write_json_report(*config.report_path, total, 
                            batch_result.success_count,
                            batch_result.error_count,
                            batch_result.error_messages);
            Logger::info("Report written to: {}", config.report_path->string());
            std::cout << "Report written to: " << config.report_path->string() << "\n";
        } catch (const std::exception& e) {
            Logger::error("Failed to write report: {}", e.what());
            std::cerr << "Warning: Failed to write report: " << e.what() << "\n";
        }
    }

    // Print error details if any
    if (!batch_result.error_messages.empty()) {
        std::cerr << "\nErrors encountered:\n";
        for (const auto& msg : batch_result.error_messages) {
            std::cerr << "  - " << msg << "\n";
        }
    }

    return batch_result.error_count > 0 ? 1 : 0;
}

} // namespace aw::spoofiq::cli
