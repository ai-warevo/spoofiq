# Spoofiq

**Professional image batch processor for iOS metadata spoofing**

Spoofiq is a C++23 native desktop application for batch-processing images and embedding authentic Apple iPhone EXIF metadata. Built with Dear ImGui for a responsive, fullscreen dockable interface.

**Current Release**: v0.5.0-Beta2 — Stable image batch processor with event-driven architecture

**Build Status** (Phase 1 — Stability & Polish):
- ✅ Linux GCC (Debug/Release): 0 warnings, all tests pass
- ✅ Linux Clang (Debug): 0 warnings, all tests pass
- ✅ Smoke test: 5/5 images processed, EXIF injected correctly
- 🚧 Windows MSVC: Presets configured, awaiting CI validation

---

## Features

✅ **Working:**
- Batch image processing (PNG/JPG/BMP/TIF → JPEG with EXIF)
- Device preset management (Add/Edit/Delete/Duplicate)
- JSON persistence for settings and presets
- Real-time logging with live console viewer
- Cancel-able batch operations with atomic progress tracking
- Event bus for loosely-coupled architecture
- C++23 strict (std::expected, std::format, std::filesystem)
- 46 passing tests (Catch2 v3)

⏱️ **Planned (Future Releases):**
- Video processing (MP4/MOV metadata injection)
- File browser with native OS dialogs
- GPS coordinate randomization
- Timestamp randomization and shifting

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

**Three-tab fullscreen interface:**

1. **Batch Spoofer**: Select files, choose device preset, process with progress tracking
2. **Presets & Devices**: Manage camera/phone metadata profiles
3. **Console**: Real-time log viewer with filtering and search

Press **ESC** to quit.

---

## Build

**Requirements:**
- C++23 compiler (GCC 13+, Clang 16+, MSVC 19.36+)
- CMake 3.28+
- OpenGL + GLFW dependencies

**Linux (GCC or Clang):**
```bash
# Install dependencies
sudo apt install libgl1-mesa-dev libglu1-mesa-dev libglfw3-dev

# Configure (choose one preset)
cmake --preset debug-gcc                    # GCC Debug (with tests)
# or
cmake --preset debug-clang                  # Clang Debug (with tests)
# or
cmake --preset release-gcc                  # GCC Release (optimized, no tests)

# Build
cmake --build build/Debug-gcc -j$(nproc)    # or build/Debug-clang or build/Release-gcc
./build/Debug-gcc/spoofiq_app               # Run the app
```

**Windows (MSVC 2026+):**
```cmd
# Configure
cmake --preset debug-msvc                   # Requires Visual Studio 18 2026
# or
cmake --preset release-msvc

# Build
cmake --build build/Debug-msvc --config Debug
.\build\Debug-msvc\Debug\spoofiq_app.exe
```

**Tests (Linux):**
```bash
cmake --preset debug-gcc
cmake --build build/Debug-gcc
ctest --preset debug-gcc --output-on-failure
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
- **Image I/O**: stb_image + stb_image_write (via FetchContent)
- **EXIF**: Manual TIFF/EXIF structure building (no external library)

See [ARCHITECTURE.md](docs/ARCHITECTURE.md) for module DAG and design decisions.

---

## Usage

### Help and Version

```bash
spoofiq_app --help          # Show help text
spoofiq_app -h              # Short form
spoofiq_app --version       # Show version
spoofiq_app -v              # Short form
```

### GUI Mode (Default)

Launch `spoofiq_app` with no arguments to open the GUI:

1. Go to **Presets & Devices** tab → verify default "iPhone 16 Pro Max" preset exists
2. Go to **Batch Spoofer** tab:
   - Enter folder path (one per line, or single folder)
   - Select preset from dropdown
   - Click **Process Batch**
3. Output files: `IMG_0001.jpg`, `IMG_0002.jpg`, etc. in same folder
4. Check **Console** tab for detailed logs

### CLI Mode (Agents & Automation)

Run `spoofiq_app` with `--cli` flag for headless batch processing without GUI:

**Basic usage:**
```bash
spoofiq_app --cli --path-to-folder /path/to/images --out-dir /path/to/output --preset=iphone_16_pro_max
```

**Command-line arguments:**
- `--cli` *(required)* — Enable CLI mode (no GUI/GLFW window)
- `--path-to-folder <path>` *(required)* — Input folder containing images
- `--out-dir <path>` *(required)* — Output directory (created if missing)
- `--preset=<preset_id>` *(required)* — Device preset ID (e.g., `iphone_16_pro_max`)
- `--start-index <N>` *(optional, default: 1)* — Starting index for IMG_XXXX sequence
- `--skip-existing` *(optional flag)* — Skip files that already exist in output dir
- `--randomize-gps` *(optional flag)* — Randomize GPS coordinates
- `--gps-radius <meters>` *(optional, default: 0.0)* — Radius for GPS randomization
- `--custom-datetime <ISO-8601>` *(optional)* — Custom EXIF timestamp (e.g., `2024-09-30T14:23:45`)
- `--shift-timestamp` *(optional flag)* — Shift timestamp per file
- `--shift-seconds <N>` *(optional)* — Seconds to shift per file
- `--report <path>` *(optional)* — Write JSON report to file

**Exit codes:**
- `0` — Success, all files processed
- `1` — Error (preset not found, folder doesn't exist, batch processing failed, etc.)

**Example with report:**
```bash
spoofiq_app --cli \
  --path-to-folder ./images \
  --out-dir ./output \
  --preset=iphone_16_pro_max \
  --start-index=100 \
  --report=./report.json
```

**JSON Report Format:**
```json
{
  "total_files": 42,
  "success_count": 42,
  "error_count": 0,
  "errors": []
}
```

---

## Troubleshooting

| Issue | Cause | Solution |
|-------|-------|----------|
| `cmake: command not found` | CMake not installed | `sudo apt install cmake` (Linux) or download from cmake.org (Windows) |
| `Could NOT find OpenGL` | GL headers missing | `sudo apt install libgl1-mesa-dev libglu1-mesa-dev` |
| `Failed to find wayland-scanner` | Wayland build attempted on fresh Linux | Preset already passes `-DGLFW_BUILD_WAYLAND=OFF`; just reconfigure |
| `The C compiler is not able to compile a simple test program` | C compiler flags conflict (clang-libcxx presets) | Use standard `debug-clang` preset instead; clang-libcxx requires `-stdlib=libc++` on C compiler too |
| Config directory read-only | Sandbox or permission issue | Set `HOME=/tmp` env var before running, e.g., `HOME=/tmp ./spoofiq_app` |
| Images fail to process ("Invalid image data") | stbi_load succeeded but data not copied to vector | Fixed in Phase 1; rebuild with latest code |
| Tests don't run on Windows | Catch2 integration incomplete | Windows CI will run tests; manual testing uses GUI or CLI modes |
| EXIF metadata not visible | exiftool not installed | Install `exiftool` package; metadata is correctly embedded (file info confirms iPhone model) |

---

## Development

**Compiler warnings:**
- Global flag: `-DSPOOFIQ_WARNINGS_AS_ERRORS=ON` (enabled in CMakeLists.txt)
- All presets build with 0 warnings on Linux (GCC/Clang)
- Windows MSVC validation pending

**Code style:**
- C++23 strict (std::expected, std::format, std::filesystem — no exceptions)
- Modular OBJECT libraries for incremental builds
- Event-driven via aw::tobus; async tasks via aw::tpl

**Testing:**
- Catch2 v3.4.0 with 46 test assertions
- Run via `ctest --preset debug-gcc` or `./build/Debug-gcc/spoofiq_tests`

---

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
