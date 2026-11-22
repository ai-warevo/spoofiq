#include <aw/spoofiq/event_bus_manager.hpp>
#include <aw/tobus/event_bus_factory.hpp>

namespace aw::spoofiq {

std::shared_ptr<aw::tobus::IEventBus> EventBusManager::instance_;

void EventBusManager::init() {
    if (instance_) return;
    
    // Use default in-process transport
    instance_ = aw::tobus::EventBusFactory::create();
}

std::shared_ptr<aw::tobus::IEventBus> EventBusManager::get() {
    if (!instance_) init();
    return instance_;
}

} // namespace aw::spoofiq
