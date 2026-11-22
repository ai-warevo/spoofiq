#pragma once

#include <aw/spoofiq/core/types.hpp>

namespace aw::spoofiq::ui {

class ConsolePanel : public IPanel {
public:
    ConsolePanel();
    void render() override;
    PanelId id() const override;

private:
    bool auto_scroll_ = true;
};

} // namespace aw::spoofiq::ui
