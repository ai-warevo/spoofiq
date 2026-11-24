#pragma once

#include <string>

namespace aw::spoofiq {

/// Application state and configuration (pure C++23, zero imgui.h or OS headers).
struct AppState {
    int window_width = 1280;
    int window_height = 720;

    bool ui_visible = true;
    float ui_opacity = 1.0f;

    std::string user_text;

    bool dark_mode = false;
    float overlay_scale = 1.0f;

    // Equality operator for testing
    bool operator==(const AppState& other) const = default;
};

class Config {
public:
    static bool save(const std::string& path, const AppState& state);

    /// Load application state from a file.
    static bool load(const std::string& path, AppState& state);
};

} // namespace aw::spoofiq
