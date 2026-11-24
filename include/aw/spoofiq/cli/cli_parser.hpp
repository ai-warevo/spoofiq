#ifndef AW_SPOOFIQ_CLI_CLI_PARSER_HPP
#define AW_SPOOFIQ_CLI_CLI_PARSER_HPP

#include <string>
#include <filesystem>
#include <expected>
#include <optional>

namespace aw::spoofiq::cli {

/// CLI configuration parsed from command-line arguments.
struct CliConfig {
    std::filesystem::path input_folder;
    std::filesystem::path output_dir;
    std::string preset_id;
    int start_index = 1;
    bool skip_existing_files = false;
    bool randomize_gps = false;
    float gps_radius_meters = 0.0f;
    bool use_current_time = true;
    std::string custom_datetime;      // ISO 8601 format, if not using current time
    bool shift_timestamp_per_file = false;
    int shift_seconds = 0;
    std::optional<std::filesystem::path> report_path;  // JSON report output
};

/// Parse command-line arguments into CliConfig.
/// Returns error if required args are missing or invalid.
std::expected<CliConfig, std::string> parse_cli_args(int argc, char* argv[]);

/// Check if CLI mode is enabled (--cli flag present).
bool is_cli_mode(int argc, char* argv[]);

/// Check if help is requested (--help or -h).
bool is_help_requested(int argc, char* argv[]);

/// Check if version is requested (--version or -v).
bool is_version_requested(int argc, char* argv[]);

/// Print help text to stdout.
void print_help(const char* program_name);

/// Print version and exit.
void print_version();

} // namespace aw::spoofiq::cli

#endif // AW_SPOOFIQ_CLI_CLI_PARSER_HPP
