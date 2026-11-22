#include <iostream>
#include <filesystem>
#include <array>
#include "image_processor.hpp"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: spoofiq-mvp <input_image> [output_dir]\n";
        std::cerr << "Example: spoofiq-mvp photo.jpg ./output\n";
        return 1;
    }
    
    fs::path input_path = argv[1];
    fs::path output_dir = argc > 2 ? fs::path(argv[2]) : fs::current_path() / "spoofiq_output";
    
    // Validate input
    if (!fs::exists(input_path)) {
        std::cerr << "Error: Input file not found: " << input_path << "\n";
        return 1;
    }
    
    // Create output directory
    fs::create_directories(output_dir);
    
    // Process image
    spoofiq::mvp::ExifData iphone_metadata;
    auto result = spoofiq::mvp::ImageProcessor::process(input_path, output_dir, iphone_metadata, 1);
    
    if (!result) {
        std::cerr << "Error: " << result.error() << "\n";
        return 1;
    }
    
    std::cout << "Success! Output: " << result.value() << "\n";
    return 0;
}
