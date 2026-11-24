#pragma once

#include <cstdint>
#include <string>

namespace aw::spoofiq::core {

/// EXIF metadata for image files.
struct ExifData {
    std::string make = "Apple";
    std::string model = "iPhone 16 Pro Max";
    std::string software = "18.0";
    std::string datetime = "2024:09:30 14:23:45";

    std::uint32_t exif_version = 0x30323330;  // "0230"
    std::string datetime_original = "2024:09:30 14:23:45";
    std::string datetime_digitized = "2024:09:30 14:23:45";
    
    // Camera settings
    float fnumber = 1.78f;
    std::uint16_t iso_speed = 50;
    float focal_length = 6.77f;  // mm
    std::string lens_make = "Apple";
    std::string lens_model = "iPhone 16 Pro Max back triple camera 6.77mm f/1.78";

    float latitude = 37.334722f;   // degrees (N positive, S negative)
    float longitude = -122.008889f; // degrees (E positive, W negative)
    float altitude = 10.0f;         // meters
    std::string gps_datetime = "2024:09:30 14:23:45";
};

/// Video metadata for QuickTime/MP4 files.
struct VideoMetadata {
    // QuickTime atoms
    std::string name = "iPhone 16 Pro Max";         // ©nam atom
    std::string creation_date = "2024-09-30 14:23:45"; // ©day atom
    std::string software = "18.0";                  // ©too atom
    
    // GPS coordinates (stored as ISO 6709 string in ©xyz atom)
    float latitude = 37.334722f;
    float longitude = -122.008889f;
    float altitude = 10.0f;
};

} // namespace aw::spoofiq::core
