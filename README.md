# Spoofiq

**Desktop batch metadata editor for media files**

Spoofiq is a C++23 native desktop application for batch-spoofing media metadata (EXIF, GPS, timestamps) to mimic authentic device outputs. Built with Dear ImGui for a responsive, fullscreen dockable interface.

---

## Features

- **Batch processing**: multi-file input with atomic IMG_XXXX sequence generation
- **Device presets**: manage camera/phone metadata profiles with JSON persistence
- **Real-time logging**: event bus integration with live console display
- **Progress tracking**: cancel-able batch operations with progress bar
- **Input validation**: folder existence, preset selection, file format checks
- **C++23 strict**: std::expected, std::format, std::filesystem (no exceptions)

---

## UI Overview

Four-tab fullscreen dockspace:

1. **Batch Spoofer**: Select folder, choose preset, process files
2. **Presets & Devices**: Add/edit/delete/duplicate device profiles
3. **Appearance**: Theme selector (Dark/Light/Midnight), accent color picker, UI scale slider
4. **Console**: Real-time log output with auto-scroll

Press **ESC** to quit. Press **F12** (debug builds only) to toggle ImGui style editor.

---

## Build

**Requirements:**
- C++23 compiler (GCC 13+, Clang 16+, MSVC 19.36+)
- CMake 3.28+
- OpenGL + GLFW dependencies

**Linux:**
```bash
sudo apt install libgl1-mesa-dev libglu1-mesa-dev libglfw3-dev
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/spoofiq_app
```

**Windows:**
```cmd
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
.\build\Release\spoofiq_app.exe
```

**Tests:**
```bash
./build/tests/spoofiq_tests
```

---

## Architecture

- **Core modules**: types, io (image/metadata), proc (image/video), config, logging, event_bus
- **UI layer**: ImGui panels (batch_spoofer, presets, console)
- **Dependencies (FetchContent)**:
  - **UI**: imgui (v1.91.8-docking), GLFW
  - **Config**: nlohmann/json (v3.11.3)
  - **Logging**: spdlog (v1.14.1)
  - **ai-warevo libraries**: aw_tpl (task scheduling), aw_tobus (event bus)
- **CMake modules**: aw_cmake_modules from local `../template_cpp/cmake`
- **Image I/O**: stb_image + stb_image_write (vendored headers)
- **EXIF**: Manual TIFF/EXIF structure building (no external library)

See [ARCHITECTURE.md](docs/ARCHITECTURE.md) for module DAG and design decisions.

---

## Usage

1. Launch `spoofiq_app`
2. Go to **Presets & Devices** tab → verify default "iPhone 16 Pro Max" preset exists
3. Go to **Batch Spoofer** tab:
   - Enter folder path (one per line, or single folder)
   - Select preset from dropdown
   - Click **Process Batch**
4. Output files: `IMG_0001.jpg`, `IMG_0002.jpg`, etc. in same folder
5. Check **Console** tab for detailed logs

---

## Configuration

Settings stored in:
- **Linux/macOS**: `~/.config/spoofiq/config.json`
- **Windows**: `%APPDATA%\spoofiq\config.json`

**AppSettings structure:**
```json
{
  "last_input_directory": "/path/to/images",
  "export_to_subfolder": false,
  "subfolder_name": "spoofed",
  "manual_start_index": 1,
  "theme": "Dark",
  "accent_color": [0.22, 0.74, 0.97],
  "ui_scale": 1.0,
  "presets": [
    {
      "id": "iphone_16_pro_max",
      "name": "iPhone 16 Pro Max",
      "make": "Apple",
      "model": "iPhone 16 Pro Max",
      "software": "iOS 18.1",
      "lens_model": "Main: ƒ/1.78 mm",
      "focal_length": 6.77,
      "focal_length_35mm": 24.0,
      "fnumber": 1.78,
      "iso": 50,
      "gps_latitude": 37.3349,
      "gps_longitude": -122.009
    }
  ]
}
```

**Appearance settings**:
- `theme`: "Dark", "Light", or "Midnight"
- `accent_color`: RGB array [0.0–1.0] for button/header tints
- `ui_scale`: 0.8–1.5 for DPI-friendly scaling

---

## Supported Formats

**Input:**
- Images: `.jpg`, `.jpeg`, `.png`, `.tif`, `.tiff`, `.bmp`
- Video: `.mp4`, `.mov` (stub for v1.1+)

**Output:**
- Images: `.jpg` (JPEG with EXIF)
- Video: `.mp4` (QuickTime atoms, v1.1+)

---

## Roadmap

- **v1.1**: GPS randomization, timestamp jitter
- **v1.2**: Drag-and-drop file input
- **v1.3**: Video processing (QuickTime atoms)
- **v1.4**: Bulk preset import/export

See [PLAN.md](docs/PLAN.md) for full versioned roadmap.

---

## License

MIT License - see [LICENSE](LICENSE) for details.

---

## Dependencies

**ai-warevo** provides core libraries:
- **aw_tpl**: Task scheduling and async/await (C++23) — fetched via FetchContent
- **aw_tobus**: Event bus for real-time logging and inter-module communication — fetched via FetchContent
- **aw_cmake_modules**: CMake build helpers — loaded from local `../template_cpp/cmake`

**Credits**

- **ImGui**: [ocornut/imgui](https://github.com/ocornut/imgui)
- **stb**: [nothings/stb](https://github.com/nothings/stb)
- **nlohmann/json**: [nlohmann/json](https://github.com/nlohmann/json)
- **spdlog**: [gabime/spdlog](https://github.com/gabime/spdlog)
- **ai-warevo**: Task scheduling (aw_tpl), event bus (aw_tobus), CMake modules (template_cpp)
