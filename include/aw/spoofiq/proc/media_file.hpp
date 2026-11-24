#ifndef AW_SPOOFIQ_PROC_MEDIA_FILE_HPP
#define AW_SPOOFIQ_PROC_MEDIA_FILE_HPP

#include <filesystem>
#include <string>
#include <memory>

namespace aw::spoofiq::proc {

/// Base class for media file operations (image or video).
/// Provides unified interface for processing different media formats.
class MediaFile {
public:
    virtual ~MediaFile() = default;

    /// Get the file path.
    [[nodiscard]] virtual const std::filesystem::path& path() const = 0;

    /// Get the media type as string ("image" or "video").
    [[nodiscard]] virtual std::string media_type() const = 0;

    /// Check if this is an image file.
    [[nodiscard]] virtual bool is_image() const = 0;

    /// Check if this is a video file.
    [[nodiscard]] virtual bool is_video() const = 0;
};

/// Factory to create appropriate MediaFile for a given path.
class MediaFileFactory {
public:
    /// Create MediaFile from path. Returns nullptr if format not supported.
    static std::unique_ptr<MediaFile> create(const std::filesystem::path& path);
};

} // namespace aw::spoofiq::proc

#endif // AW_SPOOFIQ_PROC_MEDIA_FILE_HPP
