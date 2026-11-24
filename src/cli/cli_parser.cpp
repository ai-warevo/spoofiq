#include <aw/spoofiq/cli/cli_parser.hpp>
#include <algorithm>
#include <sstream>
#include <iostream>

namespace aw::spoofiq::cli {

bool is_cli_mode(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--cli") {
            return true;
        }
    }
    return false;
}

static std::string get_arg_value(int argc, char* argv[], const std::string& flag) {
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == flag && i + 1 < argc) {
            return std::string(argv[i + 1]);
        }
        if (arg.starts_with(flag + "=")) {
            return arg.substr(flag.length() + 1);
        }
    }
    return "";
}

static bool has_flag(int argc, char* argv[], const std::string& flag) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == flag) {
            return true;
        }
    }
    return false;
}

std::expected<CliConfig, std::string> parse_cli_args(int argc, char* argv[]) {
    CliConfig config;

    // Input folder (required)
    std::string input_folder_str = get_arg_value(argc, argv, "--path-to-folder");
    if (input_folder_str.empty()) {
        return std::unexpected("Missing required argument: --path-to-folder <path>");
    }
    config.input_folder = std::filesystem::path(input_folder_str);
    if (!std::filesystem::exists(config.input_folder)) {
        return std::unexpected("Input folder does not exist: " + input_folder_str);
    }
    if (!std::filesystem::is_directory(config.input_folder)) {
        return std::unexpected("Input path is not a directory: " + input_folder_str);
    }

    // Output directory (required)
    std::string output_dir_str = get_arg_value(argc, argv, "--out-dir");
    if (output_dir_str.empty()) {
        return std::unexpected("Missing required argument: --out-dir <path>");
    }
    config.output_dir = std::filesystem::path(output_dir_str);
    // Create output directory if it doesn't exist
    if (!std::filesystem::exists(config.output_dir)) {
        std::error_code ec;
        std::filesystem::create_directories(config.output_dir, ec);
        if (ec) {
            return std::unexpected("Failed to create output directory: " + ec.message());
        }
    }

    // Preset ID (required)
    std::string preset_id_str = get_arg_value(argc, argv, "--preset");
    if (preset_id_str.empty()) {
        return std::unexpected("Missing required argument: --preset=<preset_id>");
    }
    config.preset_id = preset_id_str;

    // Start index (optional)
    std::string start_index_str = get_arg_value(argc, argv, "--start-index");
    if (!start_index_str.empty()) {
        try {
            config.start_index = std::stoi(start_index_str);
            if (config.start_index < 1) {
                return std::unexpected("Start index must be >= 1");
            }
        } catch (...) {
            return std::unexpected("Invalid start index: " + start_index_str);
        }
    }

    // Skip existing files (optional flag)
    config.skip_existing_files = has_flag(argc, argv, "--skip-existing");

    // GPS randomization (optional flag)
    config.randomize_gps = has_flag(argc, argv, "--randomize-gps");

    // GPS radius (optional)
    std::string gps_radius_str = get_arg_value(argc, argv, "--gps-radius");
    if (!gps_radius_str.empty()) {
        try {
            config.gps_radius_meters = std::stof(gps_radius_str);
            if (config.gps_radius_meters < 0.0f) {
                return std::unexpected("GPS radius must be >= 0");
            }
        } catch (...) {
            return std::unexpected("Invalid GPS radius: " + gps_radius_str);
        }
    }

    // Custom datetime (optional)
    config.custom_datetime = get_arg_value(argc, argv, "--custom-datetime");
    if (!config.custom_datetime.empty()) {
        config.use_current_time = false;
    }

    // Timestamp shift per file (optional flag)
    config.shift_timestamp_per_file = has_flag(argc, argv, "--shift-timestamp");

    // Shift seconds (optional)
    std::string shift_seconds_str = get_arg_value(argc, argv, "--shift-seconds");
    if (!shift_seconds_str.empty()) {
        try {
            config.shift_seconds = std::stoi(shift_seconds_str);
        } catch (...) {
            return std::unexpected("Invalid shift seconds: " + shift_seconds_str);
        }
    }

    // Report output path (optional)
    std::string report_str = get_arg_value(argc, argv, "--report");
    if (!report_str.empty()) {
        config.report_path = std::filesystem::path(report_str);
    }

    return config;
}

bool is_help_requested(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "--help" || arg == "-h") {
            return true;
        }
    }
    return false;
}

bool is_version_requested(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "--version" || arg == "-v") {
            return true;
        }
    }
    return false;
}

void print_help(const char* program_name) {
    std::cout << "Spoofiq - Professional image batch processor for iOS metadata spoofing\n\n";
    std::cout << "USAGE:\n";
    std::cout << "  " << program_name << " [--help | --version]\n";
    std::cout << "  " << program_name << " [GUI mode - no arguments]\n";
    std::cout << "  " << program_name << " --cli --path-to-folder <PATH> --out-dir <PATH> --preset=<ID> [OPTIONS]\n\n";
    std::cout << "REQUIRED ARGUMENTS (CLI mode only):\n";
    std::cout << "  --cli                           Enable CLI mode (headless, no GUI)\n";
    std::cout << "  --path-to-folder <PATH>         Input folder containing images\n";
    std::cout << "  --out-dir <PATH>                Output directory (created if missing)\n";
    std::cout << "  --preset=<PRESET_ID>            Device preset ID (e.g., iphone_16_pro_max)\n\n";
    std::cout << "OPTIONAL ARGUMENTS:\n";
    std::cout << "  --start-index <N>               Starting index for IMG_XXXX sequence (default: 1)\n";
    std::cout << "  --skip-existing                 Skip files that already exist in output dir\n";
    std::cout << "  --randomize-gps                 Randomize GPS coordinates\n";
    std::cout << "  --gps-radius <METERS>           Radius for GPS randomization (default: 0.0)\n";
    std::cout << "  --custom-datetime <ISO-8601>    Custom EXIF timestamp (e.g., 2024-09-30T14:23:45)\n";
    std::cout << "  --shift-timestamp               Shift timestamp per file\n";
    std::cout << "  --shift-seconds <N>             Seconds to shift per file\n";
    std::cout << "  --report <PATH>                 Write JSON report to file\n\n";
    std::cout << "EXIT CODES:\n";
    std::cout << "  0                               Success\n";
    std::cout << "  1                               Error (invalid args, preset not found, batch failed)\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "  " << program_name << " --help\n";
    std::cout << "  " << program_name << " --version\n";
    std::cout << "  " << program_name << " --cli --path-to-folder ./images --out-dir ./output --preset=iphone_16_pro_max\n";
    std::cout << "  " << program_name << " --cli --path-to-folder ./images --out-dir ./output --preset=iphone_16_pro_max \\\n";
    std::cout << "    --start-index=100 --randomize-gps --gps-radius=500 --report=report.json\n";
}

void print_version() {
#ifdef APP_VERSION
    std::cout << "Spoofiq " << APP_VERSION << "\n";
#else
    std::cout << "Spoofiq (version unknown)\n";
#endif
}

} // namespace aw::spoofiq::cli
