#ifndef AW_SPOOFIQ_EVENT_BUS_EVENT_BUS_MANAGER_HPP
#define AW_SPOOFIQ_EVENT_BUS_EVENT_BUS_MANAGER_HPP

#include <aw/tobus/event_bus.hpp>
#include <memory>

namespace aw::spoofiq {

/// Global event bus singleton for core ↔ UI communication
class EventBusManager {
public:
    /// Initialize the event bus (called from main.cpp)
    static void init();

    /// Get the global event bus instance
    static std::shared_ptr<aw::tobus::IEventBus> get();

private:
    static std::shared_ptr<aw::tobus::IEventBus> instance_;
};

} // namespace aw::spoofiq

#endif // AW_SPOOFIQ_EVENT_BUS_EVENT_BUS_MANAGER_HPP
