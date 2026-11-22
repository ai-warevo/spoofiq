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
spoofiq_ui ← (imgui, glfw)
    ↓
spoofiq_core
    ├── spoofiq_core_types
    ├── spoofiq_core_io_metadata ← (Exiv2, stb_image_write - Alpha 3)
    ├── spoofiq_core_io_image ← (stb_image - Alpha 3)
    ├── spoofiq_core_proc_image ← (spdlog - Beta 1)
    ├── spoofiq_core_proc_video ← (TagLib - Alpha 3)
    └── spoofiq_core_legacy ← (nlohmann/json - Alpha 2)
```

## Module Responsibilities

### spoofiq_core_types
- Pure C++23 types (ExifData, GpsCoords, DevicePreset, etc.)
- No dependencies beyond STL
- Header-only interface library

### spoofiq_core_io_metadata
- EXIF writing via Exiv2 (images)
- QuickTime atom writing via TagLib (videos)
- Output naming strategy (IMG_XXXX with atomic index)

### spoofiq_core_io_image
- Image loading via stb_image (PNG/JPG/BMP/TIF)
- Image saving via stb_image_write (JPG only)

### spoofiq_core_proc_image
- ImageProcessor: single-image processing pipeline
- Coordinates with metadata_writer and image I/O

### spoofiq_core_proc_video
- VideoProcessor: single-video processing pipeline
- Coordinates with TagLib for QuickTime atoms

### spoofiq_core_legacy
- Config: JSON persistence for app settings and device presets
- PanelRegistry: UI panel management (will be refactored in Beta 3)
- OverlayCanvas: Drawing commands (out of scope for spoofiq)

### spoofiq_ui
- UiRoot: ImGui window management and event loop
- BatchSpooferPanel: main UI with device selection, file I/O, batch controls

### spoofiq_app
- main.cpp: GLFW/OpenGL initialization and render loop
- Wires spoofiq_ui and spoofiq_core together

## Build System

- **CMake FetchContent** for all dependencies (no manual installation)
- **Incremental compilation**: changes to one module only rebuild dependent modules
- **vendor/ directory**: CMake build cache (`.gitignore`d)
- **Release builds**: -O3 optimization, no debug symbols

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
