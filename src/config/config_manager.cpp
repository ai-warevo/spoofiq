#include <aw/spoofiq/config_manager.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>

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
        {"start_index", s.start_index},
        {"presets", s.presets}
    };
}

void from_json(const nlohmann::json& j, AppSettings& s) {
    s.last_input_directory = j.value("last_input_directory", ".");
    s.export_to_subfolder = j.value("export_to_subfolder", false);
    s.subfolder_name = j.value("subfolder_name", "spoofiq_output");
    s.start_index = j.value("start_index", 1);
    if (j.contains("presets") && j["presets"].is_array()) {
        s.presets = j["presets"].get<std::vector<DevicePreset>>();
    }
}

std::filesystem::path ConfigManager::get_config_dir() {
#ifdef _WIN32
    const char* appdata = std::getenv("APPDATA");
    if (!appdata) {
        const char* userprofile = std::getenv("USERPROFILE");
        return std::filesystem::path(userprofile ? userprofile : ".") / "AppData" / "Roaming";
    }
    return std::filesystem::path(appdata);
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
    settings.start_index = 1;
    
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
