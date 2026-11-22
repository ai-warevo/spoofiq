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

private:
    static std::shared_ptr<spdlog::logger> logger_;
    static std::shared_ptr<ImGuiConsoleSink> console_sink_;
};

} // namespace aw::spoofiq
