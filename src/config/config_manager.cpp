#include <aw/spoofiq/config/config_manager.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace aw::spoofiq {

void to_json(nlohmann::json& j, const DevicePreset& p) {
    j = nlohmann::json{
        {"id", p.id},
        {"name", p.name},
        {"make", p.make},
        {"model", p.model},
        {"software", p.software},
        {"lens_model", p.lens_model},
        {"focal_length", p.focal_length},
        {"focal_length_35mm", p.focal_length_35mm},
        {"f_number", p.f_number},
        {"iso", p.iso},
        {"base_latitude", p.base_latitude},
        {"base_longitude", p.base_longitude}
    };
}

void from_json(const nlohmann::json& j, DevicePreset& p) {
    p.id = j.value("id", "");
    p.name = j.value("name", "");
    p.make = j.value("make", "");
    p.model = j.value("model", "");
    p.software = j.value("software", "");
    p.lens_model = j.value("lens_model", "");
    p.focal_length = j.value("focal_length", 0.0f);
    p.focal_length_35mm = j.value("focal_length_35mm", 0.0f);
    p.f_number = j.value("f_number", 0.0f);
    p.iso = j.value("iso", 0);
    p.base_latitude = j.value("base_latitude", 0.0);
    p.base_longitude = j.value("base_longitude", 0.0);
}

void to_json(nlohmann::json& j, const AppSettings& s) {
    j = nlohmann::json{
        {"last_input_directory", s.last_input_directory},
        {"export_to_subfolder", s.export_to_subfolder},
        {"subfolder_name", s.subfolder_name},
        {"skip_existing_files", s.skip_existing_files},
        {"auto_detect_index", s.auto_detect_index},
        {"manual_start_index", s.manual_start_index},
        {"selected_preset_id", s.selected_preset_id},
        {"use_current_time", s.use_current_time},
        {"custom_datetime", s.custom_datetime},
        {"shift_timestamp_per_file", s.shift_timestamp_per_file},
        {"shift_seconds", s.shift_seconds},
        {"embed_gps", s.embed_gps},
        {"randomize_gps", s.randomize_gps},
        {"gps_radius_meters", s.gps_radius_meters},
        {"theme", s.theme},
        {"accent_color", nlohmann::json::array({s.accent_color[0], s.accent_color[1], s.accent_color[2]})},
        {"ui_scale", s.ui_scale},
        {"presets", s.presets}
    };
}

void from_json(const nlohmann::json& j, AppSettings& s) {
    s.last_input_directory = j.value("last_input_directory", ".");
    s.export_to_subfolder = j.value("export_to_subfolder", false);
    s.subfolder_name = j.value("subfolder_name", "spoofiq_output");
    s.skip_existing_files = j.value("skip_existing_files", false);
    s.auto_detect_index = j.value("auto_detect_index", true);
    s.manual_start_index = j.value("manual_start_index", 1);
    s.selected_preset_id = j.value("selected_preset_id", "iphone_16_pro_max");
    s.use_current_time = j.value("use_current_time", true);
    s.custom_datetime = j.value("custom_datetime", "");
    s.shift_timestamp_per_file = j.value("shift_timestamp_per_file", false);
    s.shift_seconds = j.value("shift_seconds", 0);
    s.embed_gps = j.value("embed_gps", true);
    s.randomize_gps = j.value("randomize_gps", false);
    s.gps_radius_meters = j.value("gps_radius_meters", 0.0f);
    s.theme = j.value("theme", "Dark");
    s.ui_scale = j.value("ui_scale", 1.0f);
    
    if (j.contains("accent_color") && j["accent_color"].is_array() && j["accent_color"].size() == 3) {
        s.accent_color[0] = j["accent_color"][0];
        s.accent_color[1] = j["accent_color"][1];
        s.accent_color[2] = j["accent_color"][2];
    } else {
        s.accent_color[0] = 0.22f;  // Default cyan (#38BDF8)
        s.accent_color[1] = 0.74f;
        s.accent_color[2] = 0.97f;
    }
    
    if (j.contains("presets") && j["presets"].is_array()) {
        s.presets = j["presets"].get<std::vector<DevicePreset>>();
    }
}

std::filesystem::path ConfigManager::get_config_dir() {
#ifdef _WIN32
    char* appdata = nullptr;
    size_t len = 0;
    if (_dupenv_s(&appdata, &len, "APPDATA") == 0 && appdata != nullptr) {
        std::filesystem::path result(appdata);
        free(appdata);
        return result;
    }
    
    char* userprofile = nullptr;
    if (_dupenv_s(&userprofile, &len, "USERPROFILE") == 0 && userprofile != nullptr) {
        std::filesystem::path result = std::filesystem::path(userprofile) / "AppData" / "Roaming";
        free(userprofile);
        return result;
    }
    
    return std::filesystem::path(".") / "AppData" / "Roaming";
#else
    const char* home = std::getenv("HOME");
    return std::filesystem::path(home ? home : ".");
#endif
}

std::filesystem::path ConfigManager::get_config_path() {
#ifdef _WIN32
    return get_config_dir() / "spoofiq" / "config.json";
#else
    return get_config_dir() / ".spoofiq" / "config.json";
#endif
}

AppSettings ConfigManager::create_defaults() {
    AppSettings settings;
    settings.last_input_directory = ".";
    settings.export_to_subfolder = false;
    settings.subfolder_name = "spoofiq_output";
    settings.skip_existing_files = false;
    settings.auto_detect_index = true;
    settings.manual_start_index = 1;
    settings.selected_preset_id = "iphone_16_pro_max";
    settings.use_current_time = true;
    settings.custom_datetime = "";
    settings.shift_timestamp_per_file = false;
    settings.shift_seconds = 0;
    settings.embed_gps = true;
    settings.randomize_gps = false;
    settings.gps_radius_meters = 0.0f;
    
    settings.presets = {
        {
            "iphone_16_pro_max", "iPhone 16 Pro Max", "Apple", "iPhone 16 Pro Max",
            "iOS 18.1", "Main: ƒ/1.78 mm; Ultra Wide: ƒ/2.2 mm; Telephoto: ƒ/2.8 mm",
            6.77f, 24.0f, 1.78f, 50, 37.3349, -122.0090
        },
        {
            "iphone_15_pro", "iPhone 15 Pro", "Apple", "iPhone 15 Pro",
            "iOS 17.4", "Main: ƒ/1.78 mm; Ultra Wide: ƒ/2.2 mm; Telephoto: ƒ/2.8 mm",
            6.77f, 24.0f, 1.78f, 50, 37.3349, -122.0090
        },
        {
            "iphone_14_pro", "iPhone 14 Pro", "Apple", "iPhone 14 Pro",
            "iOS 16.6", "Main: ƒ/1.78 mm; Ultra Wide: ƒ/2.2 mm; Telephoto: ƒ/2.8 mm",
            6.77f, 24.0f, 1.78f, 50, 37.3349, -122.0090
        },
        {
            "iphone_13_pro", "iPhone 13 Pro", "Apple", "iPhone 13 Pro",
            "iOS 15.7", "Main: ƒ/1.5 mm; Ultra Wide: ƒ/2.4 mm; Telephoto: ƒ/2.8 mm",
            6.0f, 24.0f, 1.5f, 50, 37.3349, -122.0090
        }
    };
    
    return settings;
}

std::expected<AppSettings, std::string> ConfigManager::load() {
    auto config_path = get_config_path();
    
    if (!std::filesystem::exists(config_path)) {
        auto settings = create_defaults();
        if (auto res = save(settings); !res) {
            return std::unexpected(res.error());
        }
        return settings;
    }
    
    std::ifstream file(config_path);
    if (!file.is_open()) {
        return std::unexpected("Failed to open config file: " + config_path.string());
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    auto j = nlohmann::json::parse(buffer.str(), nullptr, false);
    if (j.is_discarded()) {
        return std::unexpected("JSON parsing error");
    }
    
    AppSettings settings;
    from_json(j, settings);
    return settings;
}

std::expected<void, std::string> ConfigManager::save(const AppSettings& settings) {
    auto config_path = get_config_path();
    auto config_dir = config_path.parent_path();
    
    std::error_code ec;
    if (!std::filesystem::exists(config_dir)) {
        std::filesystem::create_directories(config_dir, ec);
        if (ec) {
            return std::unexpected("Failed to create config directory: " + ec.message());
        }
    }
    
    std::ofstream file(config_path);
    if (!file.is_open()) {
        return std::unexpected("Failed to open config file for writing: " + config_path.string());
    }
    
    nlohmann::json j;
    to_json(j, settings);
    file << j.dump(4) << std::endl;
    file.close();
    
    return {};
}

} // namespace aw::spoofiq
