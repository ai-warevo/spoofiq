#include <aw/spoofiq/proc/media_file.hpp>
#include <algorithm>
#include <cctype>

namespace aw::spoofiq::proc {

namespace {
    constexpr std::string_view IMAGE_EXTENSIONS[] = {".jpg", ".jpeg", ".png", ".bmp", ".tif", ".tiff", ".webp"};
    constexpr std::string_view VIDEO_EXTENSIONS[] = {".mp4", ".mov", ".mkv", ".avi", ".webm"};

    std::string to_lower(const std::string& s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(),
                      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return result;
    }

    bool is_in_list(const std::string& ext, const std::string_view list[], size_t count) {
        std::string ext_lower = to_lower(ext);
        for (size_t i = 0; i < count; ++i) {
            if (ext_lower == list[i]) {
                return true;
            }
        }
        return false;
    }
}

/// Internal implementation for image files.
class ImageFile : public MediaFile {
private:
    std::filesystem::path path_;

public:
    explicit ImageFile(const std::filesystem::path& p) : path_(p) {}

    [[nodiscard]] const std::filesystem::path& path() const override {
        return path_;
    }

    [[nodiscard]] std::string media_type() const override {
        return "image";
    }

    [[nodiscard]] bool is_image() const override {
        return true;
    }

    [[nodiscard]] bool is_video() const override {
        return false;
    }
};

/// Internal implementation for video files.
class VideoFile : public MediaFile {
private:
    std::filesystem::path path_;

public:
    explicit VideoFile(const std::filesystem::path& p) : path_(p) {}

    [[nodiscard]] const std::filesystem::path& path() const override {
        return path_;
    }

    [[nodiscard]] std::string media_type() const override {
        return "video";
    }

    [[nodiscard]] bool is_image() const override {
        return false;
    }

    [[nodiscard]] bool is_video() const override {
        return true;
    }
};

std::unique_ptr<MediaFile> MediaFileFactory::create(const std::filesystem::path& path) {
    std::string ext = path.extension().string();

    if (is_in_list(ext, IMAGE_EXTENSIONS, std::size(IMAGE_EXTENSIONS))) {
        return std::make_unique<ImageFile>(path);
    }

    if (is_in_list(ext, VIDEO_EXTENSIONS, std::size(VIDEO_EXTENSIONS))) {
        return std::make_unique<VideoFile>(path);
    }

    return nullptr;
}

} // namespace aw::spoofiq::proc
