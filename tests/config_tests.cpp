#include <aw/spoofiq/config.hpp>
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

TEST_CASE("Config save and load round-trip", "[config]") {
    const auto temp_path = fs::temp_directory_path() / "spoofiq_test_config.txt";

    // Create test state
    aw::spoofiq::AppState original;
    original.window_width = 1920;
    original.window_height = 1080;
    original.ui_visible = false;
    original.ui_opacity = 0.75f;
    original.user_text = "Test input";
    original.dark_mode = true;
    original.overlay_scale = 1.5f;

    // Save and load
    REQUIRE(aw::spoofiq::Config::save(temp_path.string(), original));

    aw::spoofiq::AppState loaded;
    REQUIRE(aw::spoofiq::Config::load(temp_path.string(), loaded));

    // Verify
    CHECK(loaded == original);

    // Cleanup
    fs::remove(temp_path);
}

TEST_CASE("Config load from nonexistent file returns false", "[config]") {
    aw::spoofiq::AppState state;
    const auto nonexistent = fs::temp_directory_path() / "spoofiq_nonexistent.txt";

    REQUIRE_FALSE(aw::spoofiq::Config::load(nonexistent.string(), state));
}

TEST_CASE("Config save to invalid path returns false", "[config]") {
    aw::spoofiq::AppState state;
    const auto invalid_path = "/this/path/does/not/exist/config.txt";

    REQUIRE_FALSE(aw::spoofiq::Config::save(invalid_path, state));
}

TEST_CASE("Config handles empty user_text", "[config]") {
    const auto temp_path = fs::temp_directory_path() / "spoofiq_empty_text_config.txt";

    aw::spoofiq::AppState original;
    original.user_text = "";

    REQUIRE(aw::spoofiq::Config::save(temp_path.string(), original));

    aw::spoofiq::AppState loaded;
    REQUIRE(aw::spoofiq::Config::load(temp_path.string(), loaded));

    CHECK(loaded.user_text == "");

    fs::remove(temp_path);
}
