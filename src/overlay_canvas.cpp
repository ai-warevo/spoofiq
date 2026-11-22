#include <aw/spoofiq/types.hpp>

namespace aw::spoofiq {

    void
    OverlayCanvas::push_command(const DrawCommand& cmd) {
    commands_.push_back(cmd);
}

const std::vector<DrawCommand>& OverlayCanvas::commands() const {
    return commands_;
}

void OverlayCanvas::clear() {
    commands_.clear();
}

} // namespace aw::spoofiq
