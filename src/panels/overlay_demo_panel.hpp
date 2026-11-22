#pragma once

#include <aw/spoofiq/types.hpp>

namespace aw::spoofiq {

/// Overlay demo panel that draws shapes to the overlay canvas.
class OverlayDemoPanel final : public IPanel {
public:
    explicit OverlayDemoPanel(OverlayCanvas& canvas);

    void render() override;
    [[nodiscard]] PanelId id() const override;
    [[nodiscard]] std::string name() const override;

private:
    OverlayCanvas& canvas_;
};

} // namespace aw::spoofiq
