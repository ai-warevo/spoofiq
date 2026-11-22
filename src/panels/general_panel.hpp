#pragma once

#include <aw/spoofiq/core/types.hpp>
#include <aw/spoofiq/config.hpp>

#include <string>

namespace aw::spoofiq {

/// General settings panel with ImGui controls.
class GeneralPanel final : public IPanel {
public:
    explicit GeneralPanel(AppState& state);

    void render() override;
    [[nodiscard]] PanelId id() const override;
    [[nodiscard]] std::string name() const override;

private:
    AppState& state_;
};

} // namespace aw::spoofiq
