#include <aw/spoofiq/logger.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/pattern_formatter.h>

namespace aw::spoofiq {

ImGuiConsoleSink::ImGuiConsoleSink() = default;

void ImGuiConsoleSink::sink_it_(const spdlog::details::log_msg& msg) {
    std::string formatted = fmt::format("[{}] {}", spdlog::level::to_string_view(msg.level), msg.payload);
    logs_.push_back(formatted);
    if (logs_.size() > MAX_LOGS) {
        logs_.erase(logs_.begin());
    }
}

void ImGuiConsoleSink::flush_() {}

std::vector<std::string> ImGuiConsoleSink::get_logs() const {
    return logs_;
}

void ImGuiConsoleSink::clear_logs() {
    logs_.clear();
}

std::shared_ptr<spdlog::logger> Logger::logger_;
std::shared_ptr<ImGuiConsoleSink> Logger::console_sink_;

void Logger::init() {
    if (logger_) return;
    
    console_sink_ = std::make_shared<ImGuiConsoleSink>();
    std::vector<spdlog::sink_ptr> sinks{console_sink_};
    
    logger_ = std::make_shared<spdlog::logger>("spoofiq", sinks.begin(), sinks.end());
    logger_->set_level(spdlog::level::info);
    logger_->set_pattern("%Y-%m-%d %H:%M:%S");
    spdlog::register_logger(logger_);
}

std::shared_ptr<spdlog::logger> Logger::get() {
    if (!logger_) init();
    return logger_;
}

std::shared_ptr<ImGuiConsoleSink> Logger::console_sink() {
    if (!console_sink_) init();
    return console_sink_;
}

} // namespace aw::spoofiq
