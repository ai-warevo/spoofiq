#include <catch2/catch_all.hpp>
#include <aw/spoofiq/config_manager.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace aw::spoofiq;

TEST_CASE("DevicePreset JSON serialization", "[config]") {
    DevicePreset preset{
        "test_id", "Test Device", "TestMake", "TestModel",
        "TestOS", "TestLens", 50.0f, 100.0f, 2.0f, 100,
        37.0, -122.0
    };
    
    json j = preset;
    REQUIRE(j["id"] == "test_id");
    REQUIRE(j["make"] == "TestMake");
    REQUIRE(j["focal_length"] == 50.0f);
    
    DevicePreset loaded = j.get<DevicePreset>();
    REQUIRE(loaded.id == "test_id");
    REQUIRE(loaded.model == "TestModel");
    REQUIRE(loaded.iso == 100);
}

TEST_CASE("AppSettings JSON serialization", "[config]") {
    AppSettings settings;
    settings.last_input_directory = "/test";
    settings.export_to_subfolder = true;
    settings.subfolder_name = "out";
    settings.start_index = 42;
    settings.presets = {
        {"id1", "Name1", "Make1", "Model1", "OS1", "Lens1", 1.0f, 2.0f, 3.0f, 50, 37.0, -122.0}
    };
    
    json j = settings;
    REQUIRE(j["start_index"] == 42);
    REQUIRE(j["export_to_subfolder"] == true);
    REQUIRE(j["presets"].size() == 1);
    
    AppSettings loaded = j.get<AppSettings>();
    REQUIRE(loaded.start_index == 42);
    REQUIRE(loaded.subfolder_name == "out");
    REQUIRE(loaded.presets[0].id == "id1");
}

TEST_CASE("ConfigManager::get_config_path returns valid", "[config]") {
    auto path = ConfigManager::get_config_path();
    REQUIRE(!path.empty());
    REQUIRE(path.filename() == "config.json");
}

TEST_CASE("iPhone 16 Pro Max preset defaults", "[config]") {
    // Test that the defaults structure is correct
    DevicePreset iphone16{
        "iphone_16_pro_max", "iPhone 16 Pro Max", "Apple", "iPhone 16 Pro Max",
        "iOS 18.1", "Main: ƒ/1.78 mm", 6.77f, 24.0f, 1.78f, 50, 37.3349, -122.0090
    };
    
    REQUIRE(iphone16.name == "iPhone 16 Pro Max");
    REQUIRE(iphone16.make == "Apple");
    REQUIRE(iphone16.f_number == 1.78f);
    REQUIRE(iphone16.iso == 50);
    REQUIRE(iphone16.focal_length == 6.77f);
}
