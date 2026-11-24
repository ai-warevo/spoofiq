#ifndef AW_SPOOFIQ_CLI_CLI_RUNNER_HPP
#define AW_SPOOFIQ_CLI_CLI_RUNNER_HPP

#include <aw/spoofiq/cli/cli_parser.hpp>
#include <aw/spoofiq/config/config_manager.hpp>

namespace aw::spoofiq::cli {

/// Execute batch processing in CLI mode.
/// Returns exit code: 0 on success, non-zero on error.
int run_cli(const CliConfig& config, const AppSettings& app_settings);

} // namespace aw::spoofiq::cli

#endif // AW_SPOOFIQ_CLI_CLI_RUNNER_HPP
