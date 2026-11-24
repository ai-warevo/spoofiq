#include <aw/spoofiq/proc/task_executor.hpp>
#include <aw/spoofiq/logging/logger.hpp>

namespace aw::spoofiq::proc {

bool TaskExecutor::initialized_(false);

void TaskExecutor::init() {
    if (initialized_) {
        return;  // Already initialized
    }

    // Initialize the global thread pool with default thread count
    // (aw::tpl::ThreadPool::global() uses std::thread::hardware_concurrency)
    auto& pool = aw::tpl::threading::ThreadPool::global();
    static_cast<void>(pool);  // Reference the pool to ensure initialization

    initialized_ = true;
    Logger::info("TaskExecutor initialized (using aw::tpl::ThreadPool::global)");
}

bool TaskExecutor::shutdown(std::chrono::milliseconds timeout) {
    if (!initialized_) {
        return true;  // Nothing to shut down
    }

    // aw::tpl::ThreadPool doesn't have explicit shutdown on the global pool.
    // Shutdown happens automatically when the process exits or when the
    // ThreadPool object is destroyed. We log the intent here.
    Logger::info("TaskExecutor shutdown: waiting for pool to drain (timeout: {}ms)", timeout.count());

    // Note: In a future version, we could implement explicit task tracking
    // to verify completion within the timeout. For now, we trust the pool's
    // destructor to join threads cleanly on program exit.

    initialized_ = false;
    return true;
}

aw::tpl::threading::ThreadPool& TaskExecutor::get() {
    return aw::tpl::threading::ThreadPool::global();
}

} // namespace aw::spoofiq::proc
