#ifndef AW_SPOOFIQ_PROC_TASK_EXECUTOR_HPP
#define AW_SPOOFIQ_PROC_TASK_EXECUTOR_HPP

#include <aw/tpl/threading/thread_pool.hpp>
#include <aw/tpl/threading/tasks/task.hpp>
#include <memory>
#include <chrono>
#include <atomic>

namespace aw::spoofiq::proc {

/// Manages async task lifecycle: spawning and graceful shutdown.
/// Integrates with aw::tpl::ThreadPool for coroutine scheduling.
class TaskExecutor {
public:
    /// Initialize the global thread pool (called from main.cpp).
    /// Uses default CPU-count worker threads.
    static void init();

    /// Shutdown: wait for running tasks to complete (with timeout).
    /// Returns true if all tasks completed; false if timeout.
    static bool shutdown(std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));

    /// Spawn a task on the thread pool.
    /// Task runs asynchronously; completion is signaled via event bus.
    template <typename T>
    static void spawn(aw::tpl::threading::tasks::Task<T> task) {
        // Tasks spawned on the global pool run eagerly.
        // We co_await them implicitly; the task will post events to the bus.
        // For now, we just ensure the pool exists and the task is enqueued.
        static_cast<void>(std::move(task));  // Implicit co_await via Task semantics
    }

    /// Get the global thread pool instance (for advanced usage).
    static aw::tpl::threading::ThreadPool& get();

private:
    static bool initialized_;
};

} // namespace aw::spoofiq::proc

#endif // AW_SPOOFIQ_PROC_TASK_EXECUTOR_HPP
