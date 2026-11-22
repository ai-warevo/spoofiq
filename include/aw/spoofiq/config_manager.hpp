#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <expected>
#include <nlohmann/json_fwd.hpp>

namespace aw::spoofiq {

struct DevicePreset {
    std::string id;
    std::string name;
    std::string make;
    std::string model;
    std::string software;
    std::string lens_model;
    float focal_length;
    float focal_length_35mm;
    float f_number;
    int iso;
    double base_latitude;
    double base_longitude;
};

// JSON serialization (ADL)
void to_json(nlohmann::json& j, const DevicePreset& p);
void from_json(const nlohmann::json& j, DevicePreset& p);

struct AppSettings {
    std::string last_input_directory;
    bool export_to_subfolder;
    std::string subfolder_name;
    bool skip_existing_files;
    bool auto_detect_index;
    int manual_start_index;
    
    // Active preset
    std::string selected_preset_id;
    
    // Timestamp settings
    bool use_current_time;
    std::string custom_datetime; // ISO 8601 format
    bool shift_timestamp_per_file;
    int shift_seconds;
    
    // GPS settings
    bool embed_gps;
    bool randomize_gps;
    float gps_radius_meters;
    
    // Saved presets
    std::vector<DevicePreset> presets;
};

// JSON serialization (ADL)
void to_json(nlohmann::json& j, const AppSettings& s);
void from_json(const nlohmann::json& j, AppSettings& s);

class ConfigManager {
public:
    static std::expected<AppSettings, std::string> load();
    static std::expected<void, std::string> save(const AppSettings& settings);
    static std::filesystem::path get_config_path();

private:
    static std::filesystem::path get_config_dir();
    static AppSettings create_defaults();
};

} // namespace aw::spoofiq
