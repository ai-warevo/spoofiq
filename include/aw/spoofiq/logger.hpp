#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <memory>
#include <vector>
#include <mutex>
#include <string>

namespace aw::spoofiq {

class ImGuiConsoleSink : public spdlog::sinks::base_sink<std::mutex> {
public:
    ImGuiConsoleSink();
    std::vector<std::string> get_logs() const;
    void clear_logs();

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override;
    void flush_() override;

private:
    static constexpr size_t MAX_LOGS = 1000;
    std::vector<std::string> logs_;
};

class Logger {
public:
    static void init();
    static std::shared_ptr<spdlog::logger> get();
    static std::shared_ptr<ImGuiConsoleSink> console_sink();
    
    // Convenience functions
    template <typename... Args>
    static void info(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (auto logger = get()) {
            logger->info(fmt, std::forward<Args>(args)...);
        }
    }
    
    template <typename... Args>
    static void warn(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (auto logger = get()) {
            logger->warn(fmt, std::forward<Args>(args)...);
        }
    }
    
    template <typename... Args>
    static void error(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (auto logger = get()) {
            logger->error(fmt, std::forward<Args>(args)...);
        }
    }
    
    template <typename... Args>
    static void debug(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (auto logger = get()) {
            logger->debug(fmt, std::forward<Args>(args)...);
        }
    }

private:
    static std::shared_ptr<spdlog::logger> logger_;
    static std::shared_ptr<ImGuiConsoleSink> console_sink_;
};

} // namespace aw::spoofiq
