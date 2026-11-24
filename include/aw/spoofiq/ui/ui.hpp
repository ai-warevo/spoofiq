#ifndef AW_SPOOFIQ_UI_UI_HPP
#define AW_SPOOFIQ_UI_UI_HPP

#include <memory>

namespace aw::spoofiq {

class PanelRegistry;

/// Root UI class that orchestrates all panels and rendering.
class UiRoot {
public:
    /// Construct with panel registry.
    explicit UiRoot(PanelRegistry& registry);

    /// Render the UI (called each frame within ImGui context).
    void render();

private:
    PanelRegistry& registry_;
};

} // namespace aw::spoofiq

#endif // AW_SPOOFIQ_UI_UI_HPP
