# spoofiq Architecture

## Module Structure

```
spoofiq_app (executable)
    ├── spoofiq_ui (static library)
    │   ├── ui_root.cpp
    │   └── panels/
    │       └── batch_spoofer_panel.cpp
    │
    └── spoofiq_core (static library)
        ├── spoofiq_core_types (interface library)
        │   └── core/types.hpp (ExifData, GpsCoords, PanelId, etc.)
        │
        ├── spoofiq_core_io_metadata (static library)
        │   └── io/metadata_writer.cpp (stub)
        │
        ├── spoofiq_core_io_image (static library)
        │   └── io/image_reader.cpp (stub)
        │
        ├── spoofiq_core_proc_image (static library)
        │   └── processors/image_processor.cpp (stub)
        │
        ├── spoofiq_core_proc_video (static library)
        │   └── processors/video_processor.cpp (stub)
        │
        └── spoofiq_core_legacy (static library)
            ├── config.cpp
            ├── overlay_canvas.cpp
            └── panel_registry.cpp
```

## Dependency Graph

```
spoofiq_app
    ↓
spoofiq_ui ← (imgui, glfw, OpenGL)
    ↓
spoofiq_core
    ├── spoofiq_core_types
    ├── spoofiq_core_io_metadata ← (manual EXIF building; TagLib deferred)
    ├── spoofiq_core_io_image ← (stb_image via FetchContent)
    ├── spoofiq_core_proc_image ← (spdlog)
    ├── spoofiq_core_proc_video ← (stub; TagLib deferred)
    ├── spoofiq_core_config ← (nlohmann/json)
    ├── spoofiq_core_logging ← (spdlog, imgui)
    ├── spoofiq_core_event_bus ← (aw::tobus)
    └── spoofiq_core_state ← (core types)
```

## Module Responsibilities

### spoofiq_core_types
- Pure C++23 types (DrawCommand, PanelId, PanelRegistry, OverlayCanvas)
- No external dependencies

### spoofiq_core_io_metadata
- Manual EXIF structure building (TIFF/EXIF spec compliance)
- Atomic IMG_XXXX sequence generation
- **Stub**: QuickTime atoms (requires TagLib, deferred)

### spoofiq_core_io_image
- Image loading via stb_image (PNG/JPG/BMP/TIF) via FetchContent
- Image saving via stb_image_write (JPG only) via FetchContent

### spoofiq_core_proc_image
- ImageProcessor: batch image processing pipeline
- EXIF injection for iPhone metadata
- Thread-safe with progress tracking

### spoofiq_core_proc_video
- **Stub**: VideoProcessor skeleton
- Deferred implementation (requires TagLib for QuickTime)

### spoofiq_core_config
- JSON persistence for app settings and device presets
- ConfigManager with thread-safe caching

### spoofiq_core_logging
- spdlog logger wrapper with custom ImGuiConsoleSink
- Thread-safe structured logging

### spoofiq_core_event_bus
- Event bus singleton using aw::tobus
- Decouples batch processing from UI updates

### spoofiq_core_state
- AppState: window and UI configuration persistence
- Config: app settings and device presets
- OverlayCanvas: drawing command storage

### spoofiq_ui
- UiRoot: ImGui window management
- Three-tab interface (Batch Spoofer, Presets & Devices, Console)
- Event-driven updates from background processing

### spoofiq_app
- main.cpp: GLFW/OpenGL initialization
- Integrates UI and core layers

## Build System

- **CMake FetchContent** for all dependencies (stb, nlohmann/json, spdlog, imgui, glfw, Catch2, aw::tpl, aw::tobus)
- **Incremental compilation**: modular OBJECT libraries for fast builds
- **Platform support**: Linux (clang/gcc), Windows (MSVC), macOS (clang)

## Current Version (v0.5.0 - Beta 2)

**What works:**
- ✅ Image batch processing (PNG/JPG/BMP/TIF → JPEG with EXIF)
- ✅ Device preset management with JSON persistence
- ✅ 3-tab professional UI
- ✅ Real-time progress tracking with cancellation
- ✅ Event bus integration for loosely-coupled architecture
- ✅ spdlog-based console logging
- ✅ 46 passing tests (Catch2 v3)

**Known limitations:**
- ❌ Video processing (stub; requires TagLib)
- ❌ File browser (stub; requires nativefiledialog-extended)
- ❌ GPS randomization (stub; planned for future)
- ❌ Timestamp randomization (stub; planned for future)

## Threading Model (Beta 2+)

```
UI Thread (main)
    ├── ImGui rendering
    ├── tobus event subscription
    └── spawns worker coroutines
         ↓
Worker Pool (aw::tpl::Executor)
    ├── ImageProcessor coroutines
    ├── VideoProcessor coroutines
    └── publishes progress via tobus
```

## Data Flow (Beta 2+)

```
User clicks "Start Batch"
    ↓
BatchSpooferPanel emits BatchStartEvent via tobus
    ↓
BatchOrchestrator::process_batch() coroutine
    ├── for each file: co_await ImageProcessor::process()
    ├── publishes ProgressEvent(file_index, total) via tobus
    └── publishes BatchCompleteEvent via tobus
         ↓
BatchSpooferPanel receives events, updates UI
```

## Version Milestones

- **MVP (v0.1.0)**: Monolithic single-image processing
- **Alpha 1 (v0.2.0)**: Modular architecture + dependency integration ← **Current**
- **Alpha 2 (v0.3.0)**: JSON config + device presets
- **Alpha 3 (v0.4.0)**: EXIF + QuickTime atom injection
- **Beta 1 (v0.5.0)**: spdlog + Console tab
- **Beta 2 (v0.6.0)**: Coroutines + event bus
- **Beta 3 (v0.7.0)**: Three-tab UI complete
- **RC 1 (v0.8.0)**: GPS randomization + timestamp controls
- **RC 2 (v0.9.0)**: Polish + error handling
- **RC 3 (v0.9.5)**: Documentation + CI/CD
- **Release (v1.0.0)**: Final verification

## Key Design Decisions

1. **C++23 strict mode**: No exceptions (`std::expected` for errors), `std::format`, `std::filesystem`
2. **Coroutines via aw::tpl**: Async/await style batch processing with cancellation
3. **Event bus via aw::tobus**: Zero coupling between core and UI
4. **FetchContent for all deps**: No system package requirements
5. **Modular libraries**: Incremental compilation, clear ownership
6. **Static linking**: Single binary distribution (no runtime .so dependencies)
