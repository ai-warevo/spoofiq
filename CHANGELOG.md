# Changelog

All notable changes to Spoofiq are documented here.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

---

## [0.5.0] - Beta 2

### Added - Beta 2 (v0.5.0)
- **Production-ready image batch processor**
- Three-tab UI: Batch Spoofer, Presets & Devices, Console
- Device preset management with add/edit/delete/duplicate
- JSON persistence for settings and presets
- Real-time event bus integration (tobus)
- Batch processing with multi-file input (newline-separated paths)
- Progress tracking with cancel button
- Error dialogs for input validation
- spdlog-based logging system with live console viewer
- 46 passing test assertions (Catch2 v3)

### Technical
- C++23 strict (std::expected, std::format, std::filesystem)
- CMake FetchContent: imgui v1.91.8-docking, nlohmann/json v3.11.3, spdlog v1.14.1, aw_tpl v3.1.0, aw_tobus, stb
- Modular OBJECT libraries for incremental compilation
- stb_image/stb_image_write for image I/O via FetchContent
- Manual EXIF structure building (TIFF/EXIF spec)
- Atomic IMG_XXXX sequence generation
- Thread-safe batch processing with CancellationToken

### Known Limitations
- Video processing deferred (requires TagLib integration)
- File browser deferred (requires nativefiledialog-extended)
- GPS randomization deferred (manual coordinates only)
- Timestamp randomization deferred (current time only)
- Image format conversion deferred (RGB images fully supported)

---

## [0.4.0] - Alpha 3

### Added - Alpha 3 (v0.4.0)
- Presets & Devices panel
- Add/edit/delete device presets
- JSON preset persistence via ConfigManager
- Hardcoded iPhone 16 Pro Max default

---

## [0.3.0] - Alpha 2

### Added - Alpha 2 (v0.3.0)
- ConfigManager with nlohmann/json
- AppSettings structure (last_input_dir, export settings, presets)
- JSON load/save to ~/.config/spoofiq/config.json

---

## [0.2.0] - Alpha 1

### Added - Alpha 1 (v0.2.0)
- Modular architecture: 8 OBJECT libraries
- core/types: PanelId, DrawCommand, IPanel, PanelRegistry, OverlayCanvas
- io: image_io (stb), metadata_io (EXIF builder)
- proc: image_processor, video_processor stubs
- UI fullscreen dockspace with 3 tabs

---

## [0.1.0] - MVP

### Added - MVP (v0.1.0)
- Initial Dear ImGui desktop app
- GLFW/OpenGL host
- Basic window with ESC to quit
- CMake build system with FetchContent
- Catch2 v3 test framework

---

[0.5.0]: https://github.com/yourusername/spoofiq/releases/tag/v0.5.0
[0.4.0]: https://github.com/yourusername/spoofiq/releases/tag/v0.4.0
[0.3.0]: https://github.com/yourusername/spoofiq/releases/tag/v0.3.0
[0.2.0]: https://github.com/yourusername/spoofiq/releases/tag/v0.2.0
[0.1.0]: https://github.com/yourusername/spoofiq/releases/tag/v0.1.0
