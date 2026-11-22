# Spoofiq Implementation Plan: C++23 Desktop Metadata Forge

## Executive Summary
Build a native C++23 desktop application using Dear ImGui to batch process media files (images and videos) and spoof their metadata to mimic authentic Apple iPhone assets. The architecture separates concerns into `spoofiq::core` (business logic) and `spoofiq::ui` (ImGui layer), decoupled via the `tobus` event bus, with concurrent batch processing via `tpl` coroutines.

## Success Criteria
- Clean build with all FetchContent dependencies resolved (no manual system installations required)
- Event-driven architecture with zero coupling between UI and batch processing backend
- Coroutine-based batch processor supporting cancellation via `CancellationToken`
- Catch2 v3 unit tests for metadata generation, config serialization, and edge cases
- Three-tab UI (Batch Spoofer, Presets & Devices, Console) fully functional with spdlog integration

## Project Context Analysis

### Existing Codebase Structure
- **CMakeLists.txt**: Already fetches `tpl` (v3.1.0), `tobus` (main), GLFW (3.4), ImGui (v1.91.8-docking), Catch2 v3 via `aw_cmake_modules`
- **spoofiq_core** (static lib): Pure C++23, includes config, panel_registry, overlay_canvas; linked with `aw::tpl`, `aw::tobus`, Threads
- **spoofiq_ui** (static lib): ImGui layer; linked with spoofiq_core, imgui
- **spoofiq_app** (executable): GLFW/OpenGL host; linked with spoofiq_ui, glfw, OpenGL
- **spoofiq_overlay** (shared lib): Embeddable overlay (platform-specific win32/posix support)
- **Tests**: Catch2 v3 framework already integrated; config_tests, panel_registry_tests, overlay_canvas_tests exist

### Python Script Analysis
- **Input formats**: `.jpg`, `.jpeg`, `.png`, `.tif`, `.tiff`, `.bmp`, `.heic` (images); `.mp4`, `.mov` (videos)
- **Output naming**: `IMG_XXXX.{jpg|mp4}` (zero-padded 4-digit index)
- **EXIF metadata** (via piexif): 0th IFD (Make, Model, Software, DateTime), Exif IFD (ExifVersion, ComponentsConfiguration, DateTimes, FNumber=1.78, ISO=50, FocalLength=6.77mm, Lens specs), GPS IFD (Lat/Lon)
- **QuickTime atoms** (via mutagen.mp4): `©nam`, `©day`, `©too`, `©xyz` (name, date, software, geolocation)
- **Image workflow**: Convert RGBA/Palette to RGB; encode as clean JPEG with EXIF payload (via PIL)
- **Video workflow**: Copy file, strip existing metadata, inject QuickTime atoms (via MP4 file manipulation)
- **Thread-safety concern**: Concurrent `IMG_XXXX` index allocation must be atomic to prevent collisions

### Custom Libraries Assessment
- **`aw::tpl`**: Provides `Task<T>`, `when_all(Task...)`, `CancellationToken` for structured async/await; already integrated
- **`aw::tobus`**: Event bus factory (`EventBusFactory::create()`); thread-safe event publishing/subscribing; already integrated

---

## Architectural Design

### 1. High-Level Subsystem Overview
```
spoofiq_core (Business Logic Layer)
├── ConfigManager (nlohmann/json persistence)
├── ImageProcessor (stb_image + stb_image_write + EXIF library)
├── VideoProcessor (TagLib or MP4 atom manipulation)
├── BatchOrchestrator (tpl::Task, tpl::when_all, CancellationToken)
└── Event Bus Consumer (tobus integration for spdlog + config updates)

spoofiq_ui (ImGui Layer)
├── TabBar (3 tabs: Batch Spoofer, Presets & Devices, Console)
├── BatchSpooferPanel (file selection, preset selection, progress)
├── PresetsPanel (master-detail JSON editor)
├── ConsolePanel (spdlog log viewer + filters)
└── Event Bus Publisher (progress updates → BatchOrchestrator)

Shared Data Flow
├── tobus event contracts (ProgressEvent, FileFinishedEvent, etc.)
├── spdlog logger (custom ImGuiConsoleSink)
└── tpl coroutine executor (background worker threads)
```

### 2. Event Bus Architecture (tobus)

**Event Contracts** (defined in `spoofiq::core::events`):

```cpp
struct ProgressEvent {
    int total_files;
    int processed_count;
    std::string current_filename;
    float progress_percent;
};

struct FileFinishedEvent {
    std::string input_filename;
    std::string output_filename;
    bool success;
    std::string error_message;  // Empty if success
};

struct BatchCompletedEvent {
    int total_images_processed;
    int total_videos_processed;
    int total_errors;
    std::string summary;
};

struct BatchCancelledEvent {};
```

**Consumer Pattern (UI thread → spdlog, state updates)**:
- UI thread subscribes to events on startup
- BatchOrchestrator publishes events after processing each file
- ImGuiConsoleSink listens to spdlog entries and displays in Console tab
- UI state machine transitions based on event receipts

### 3. Coroutine Pipeline (tpl)

**Batch Processing Flow**:
```cpp
auto batch_task = tpl::when_all(
    process_image_1(cancellation_token),
    process_image_2(cancellation_token),
    ...
    process_video_N(cancellation_token)
);
```

**Concurrency Guarantees**:
- Atomic counter for `IMG_XXXX` index allocation (using `std::atomic<int>`)
- Each coroutine task calls `atomic_fetch_add()` to claim next index
- CancellationToken stops new tasks from being spawned; in-flight tasks poll `token.is_cancelled()`
- Event publishing from coroutine must be thread-safe (tobus handles this)

### 4. Configuration Manager (nlohmann/json)

**Default `config.json` Structure**:
```json
{
  "last_input_directory": "/home/user/Desktop",
  "export_to_subfolder": true,
  "subfolder_name": "spoofiq_out",
  "skip_existing_img_xxxx": false,
  "auto_detect_index": true,
  "start_index": 1,
  "timestamp_mode": "current_time",
  "custom_datetime": "2024-11-23T12:30:00",
  "timestamp_shift_enabled": true,
  "timestamp_shift_seconds": 3,
  "embed_gps": true,
  "gps_randomize": true,
  "gps_randomize_radius_m": 50.0,
  "device_presets": [
    {
      "id": "iphone_16_pro_max",
      "name": "iPhone 16 Pro Max",
      "make": "Apple",
      "model": "iPhone 16 Pro Max",
      "software": "iOS 18.1",
      "lens_model": "iPhone 16 Pro Max back triple camera 6.77mm f/1.78",
      "focal_length": 6.77,
      "focal_length_35mm": 24,
      "f_number": 1.78,
      "iso": 50,
      "base_latitude": 37.3349,
      "base_longitude": -122.0090
    }
  ]
}
```

**ConfigManager API**:
```cpp
namespace spoofiq::core {
  class ConfigManager {
    static bool load_from_disk();
    static bool save_to_disk();
    static DevicePreset get_preset(const std::string& id);
    static void set_preset(const DevicePreset& preset);
    static std::string get_config_path();
  };
}
```

### 5. Image & Video Processors

**Image Processing Pipeline**:
1. Load via `stb_image`
2. Convert RGBA/Palette → RGB if needed
3. Build EXIF payload (via Exiv2 or raw APP1 segment builder)
4. Encode as JPEG via `stb_image_write`
5. Return success/error to BatchOrchestrator

**Video Processing Pipeline**:
1. Copy file to destination (atomic write to temp, then rename)
2. Use TagLib or raw MP4 atom manipulation to:
   - Strip existing metadata atoms
   - Inject `©nam`, `©day`, `©too`, `©xyz` QuickTime atoms
3. Return success/error

**Library Choice Trade-offs**:
- **EXIF**: Exiv2 (heavyweight but battle-tested) vs. raw APP1 segment builder (lightweight, custom)
  - **Decision**: Exiv2 (v0.28.x stable) for reliability; disable examples/tests in CMake
- **Video**: TagLib (lightweight, v1.14+ stable) vs. raw MP4 atom manipulation
  - **Decision**: TagLib; supports QuickTime atoms natively via MP4::File

### 6. Logging Architecture (spdlog + Custom ImGuiConsoleSink)

**Integration Points**:
1. Create `ImGuiConsoleSink` class inheriting from `spdlog::sinks::sink`
2. Main logger instance with dual sinks: file (spoofiq.log) + ImGuiConsoleSink
3. ImGuiConsoleSink stores formatted log entries in a thread-safe ring buffer
4. ConsolePanel reads buffer on each frame and renders with color coding

**Thread-Safety**:
- spdlog ensures sink methods called from worker threads are serialized
- ImGui frame loop runs on main thread only
- Verify tobus event handler runs on UI thread context (may need async dispatch wrapper)

---

## Dependency Integration Plan

### New FetchContent Declarations (CMakeLists.txt additions)

| Library | Purpose | Tag/Version | Cache Flags |
|---------|---------|-------------|------------|
| `nlohmann/json` | JSON config persistence | v3.14.1 | None (header-only) |
| `gabime/spdlog` | Structured logging | v1.14.1 | `SPDLOG_BUILD_TESTS=OFF`, `SPDLOG_BUILD_EXAMPLES=OFF` |
| `nothings/stb` | Image I/O (stb_image, stb_image_write) | master (header snapshot) | Manual vendoring (no CMake) |
| `Exiv2/exiv2` | EXIF metadata generation | v0.28.3 | `EXIV2_BUILD_TESTS=OFF`, `EXIV2_BUILD_SAMPLES=OFF`, `EXIV2_BUILD_PO=OFF` |
| `taglib/taglib` | QuickTime atom manipulation | v1.14 | `BUILD_TESTS=OFF`, `BUILD_EXAMPLES=OFF` |
| `btzy/nativefiledialog-extended` | OS-native folder dialogs | v1.1.1 | `NFD_BUILD_TESTS=OFF` |

### CMake Integration Details

**stb (special handling)**:
- No CMake support; vendor as `src/vendor/stb/` or create thin wrapper library
- Or use unofficial stb CMake packages; evaluate for size/complexity

**New Library Targets** (add to spoofiq_core):
```cmake
target_link_libraries(spoofiq_core PUBLIC
  aw::tpl aw::tobus Threads::Threads
  nlohmann_json::nlohmann_json
  spdlog::spdlog
  Exiv2::Exiv2
  taglib::tag
  nfd::nfd)
```

---

## Implementation Milestones

### Milestone 1: Dependency Integration & Build Verification
**Deliverables**:
- Updated CMakeLists.txt with all FetchContent declarations
- Verify clean build: `cmake -B build && cmake --build build`
- Thin wrapper library for stb (if needed)
- BUILD_TESTS=ON verification

**Estimated Tasks**: 4–6 hours

### Milestone 2: Core Layer - ConfigManager & Persistence
**Deliverables**:
- `spoofiq::core::ConfigManager` with load/save via nlohmann/json
- Default `config.json` generation with 4–6 iPhone presets
- Unit tests: JSON serialization round-trip, preset lookups, default values
- spdlog logger singleton setup + ImGuiConsoleSink base class

**Estimated Tasks**: 6–8 hours

### Milestone 3: Image & Video Processors
**Deliverables**:
- `ImageProcessor::process()`: Load, convert, inject EXIF, encode JPEG
- `VideoProcessor::process()`: Copy, strip metadata, inject QuickTime atoms
- Error handling via `std::expected<T, std::string>`
- Unit tests: EXIF generation correctness, RGB conversion, metadata injection, failure modes (corrupted files, unsupported codecs)

**Estimated Tasks**: 10–12 hours

### Milestone 4: Async Orchestrator & CancellationToken Integration
**Deliverables**:
- `BatchOrchestrator` class using `tpl::Task<Result>` and `tpl::when_all()`
- Atomic index allocation for `IMG_XXXX` naming
- tobus event publishing (ProgressEvent, FileFinishedEvent, BatchCompletedEvent, BatchCancelledEvent)
- CancellationToken integration: coroutine polling and graceful task shutdown
- Unit tests: concurrent index allocation (stress test with 100 parallel tasks), cancellation correctness, event emission ordering

**Estimated Tasks**: 8–10 hours

### Milestone 5: UI Layer - Three-Tab Interface
**Deliverables**:
- **Tab 1 (Batch Spoofer)**:
  - Input folder selection (ImGui::InputText + "Browse..." button via nfd)
  - Drag-and-drop folder support
  - Export subfolder checkbox + name input
  - Preset selector (ImGui::Combo from ConfigManager)
  - Timestamp mode radio buttons + custom date picker
  - GPS checkbox + randomize radius
  - Start button (transitions to Stop on batch start, publishes cancel event on click)
  - Progress bar + filename + error count
- **Tab 2 (Presets & Devices)**:
  - Master-detail layout: preset list + detail form
  - Add/Duplicate/Delete buttons
  - Form fields: Name, Make, Model, Software, LensModel, FocalLength, FNumber, ISO, Latitude, Longitude
  - Save & Reset buttons
- **Tab 3 (Console)**:
  - Level filter checkboxes (Info, Warn, Error)
  - Live search bar (ImGuiTextFilter)
  - Monospace log viewport with auto-scroll
  - Color-coded severity (white/green info, yellow warn, red error)
  - Unread count badge in tab label

- **ImGuiConsoleSink Implementation**:
  - Thread-safe ring buffer (capacity: 10,000 entries max)
  - Formatted log message storage with timestamp + level
  - ConsolePanel reads buffer each frame and renders only visible lines

**Estimated Tasks**: 12–16 hours

### Milestone 6: Integration & End-to-End Testing
**Deliverables**:
- Full workflow test: load folder with mixed media → process batch → verify outputs
- Error recovery: corrupted MP4, unsupported .heic, permission denied on output folder
- UI responsiveness: cancel during batch processing, verify no crash/freeze
- Catch2 integration tests (spawn real batch, verify file outputs)
- Stress test: 1000+ files with concurrent processing

**Estimated Tasks**: 6–8 hours

---

## Potential Pitfalls & Mitigations

### 1. Thread-Safety: tobus Event Publishing from Coroutines
**Risk**: Worker thread publishing events → UI thread consuming in ImGui frame loop → data race
**Mitigation**:
- tobus uses internal mutex for thread-safe event dispatch
- ImGui frame loop runs on main thread only
- Verify tobus event handler runs on UI thread context (may need async dispatch wrapper)

### 2. EXIF Metadata Correctness
**Risk**: Generated EXIF tags don't match real iPhone output; tools detect spoofing
**Mitigation**:
- Validate generated EXIF bytes against reference iPhone photos (use `exiftool` for comparison)
- Include GPS coordinate randomization (±50m radius default) to avoid fingerprinting
- Test with standard iOS metadata analysis tools

### 3. MP4 Atom Manipulation Safety
**Risk**: Corrupted MP4 container if atom injection fails; unplayable output
**Mitigation**:
- Write to temporary file first; rename only on success
- Use TagLib's high-level MP4::File API (less error-prone than raw atom parsing)
- Validate output with `ffprobe`; fail-safe fallback: copy unmodified if injection fails

### 4. Unicode/UTF-8 Path Handling on Windows
**Risk**: Non-ASCII paths in input folder → stb_image/TagLib fail with encoding mismatches
**Mitigation**:
- Use `std::filesystem::path` throughout (handles UTF-8 internally)
- stb_image requires `FILE*` on Windows; convert path via `std::ifstream` or `fopen` wrapper
- TagLib: verify UTF-8 support in v1.14; test with Chinese/Japanese paths

### 5. Index Collision Under High Concurrency
**Risk**: Two coroutines claim same `IMG_XXXX` index; file overwrite
**Mitigation**:
- Atomic `std::atomic<int> next_index` initialized from ConfigManager
- Each coroutine calls `next_index.fetch_add(1, std::memory_order_acquire)`
- Pre-flight check: scan output folder for existing IMG_XXXX files; initialize counter to max+1
- Unit test: spawn 100 concurrent tasks, verify no collisions in output filenames

### 6. ImGui Responsiveness During Batch Processing
**Risk**: 1000+ file batch blocks ImGui frame; unresponsive UI
**Mitigation**:
- Batch processing runs on worker threads (tpl coroutines in separate executor)
- UI thread polls tobus events (non-blocking) each frame
- Render progress bar + current filename without waiting for batch completion
- Implement "frame budget": limit event processing per frame (e.g., 50 events max)

### 7. spdlog Ring Buffer Overflow
**Risk**: 10,000-entry buffer fills during large batch; oldest logs discarded without warning
**Mitigation**:
- Ring buffer supports circular overwrite (acceptable for logs)
- ConsolePanel UI shows "… (truncated)" indicator when scrollback is culled
- File sink (spoofiq.log) is unlimited; full history persisted to disk

### 8. .heic Format Unsupported
**Risk**: User attempts to process .heic images → skipped silently or hard error
**Mitigation**:
- stb_image does not support HEIC
- Gracefully skip .heic files with WARN-level log message
- Document supported formats in UI tooltip + console message

---

## Testing Strategy

### Unit Tests (Catch2 v3)

**ConfigManager Tests** (`config_manager_tests.cpp`):
- Load empty/missing config file → generates defaults
- Save and load round-trip → equality check
- Preset CRUD operations (add, update, delete, lookup)
- Invalid JSON → error handling

**ImageProcessor Tests** (`image_processor_tests.cpp`):
- Load PNG, JPEG, BMP, TIF → verify pixel integrity
- RGBA → RGB conversion → no alpha channel in output
- EXIF injection → extract and verify tags match input preset
- Output JPEG quality baseline

**VideoProcessor Tests** (`video_processor_tests.cpp`):
- Copy MP4 → verify file integrity (filesize > 0)
- Inject QuickTime atoms → extract and verify (using exiftool or MP4 parser)
- Corrupted MP4 → error handling (not crash)
- Strip existing metadata → verify atoms removed (before inject)

**BatchOrchestrator Tests** (`batch_orchestrator_tests.cpp`):
- Concurrent index allocation: 100 tasks → verify IMG_0001 to IMG_0100 (no gaps, no collisions)
- CancellationToken: spawn 50 tasks, cancel after 10 → verify remaining tasks exit gracefully
- Event emission: process 5 files → verify 5 FileFinishedEvents published + 1 BatchCompletedEvent
- Empty folder → BatchCompletedEvent with zero counts

**Integration Tests** (`integration_tests.cpp`):
- Full workflow: create temp folder with mixed media (3 images, 2 videos) → process batch → verify output files exist + metadata correct
- Error recovery: include 1 corrupted MP4 → verify processing continues, error logged, final count accurate

### Manual Testing Checklist
- [ ] Launch app → load config.json (or generate defaults)
- [ ] Tab 1: Select folder with media files → verify file count displayed
- [ ] Tab 1: Choose preset → verify device metadata fields populated
- [ ] Tab 1: Click Start → progress bar animates, Console logs entries
- [ ] Tab 1: Click Stop during processing → verify batch cancels, no orphaned processes
- [ ] Tab 2: Add new preset → save config → restart app → verify preset persists
- [ ] Tab 3: Filter logs by level → verify only selected levels shown
- [ ] Tab 3: Copy to clipboard → paste in text editor
- [ ] Output files: verify EXIF tags (exiftool), QuickTime atoms (ffprobe), file sizes reasonable

---

## Public API & Schema Changes

### New Core Headers
- `include/aw/spoofiq/config_manager.hpp` (ConfigManager, DevicePreset, AppSettings)
- `include/aw/spoofiq/events.hpp` (ProgressEvent, FileFinishedEvent, etc.)
- `include/aw/spoofiq/image_processor.hpp` (ImageProcessor)
- `include/aw/spoofiq/video_processor.hpp` (VideoProcessor)
- `include/aw/spoofiq/batch_orchestrator.hpp` (BatchOrchestrator)
- `include/aw/spoofiq/logger.hpp` (ImGuiConsoleSink, logger singleton)

### Data Flow Contracts
- **tobus Event Bus**: Events published on worker threads, consumed on UI thread
- **CancellationToken**: Passed by reference into coroutine tasks; polled via `token.is_cancelled()`
- **ConfigManager**: Static singleton; thread-safe access via mutex-protected save/load
- **spdlog Logger**: Global singleton; thread-safe (spdlog internals)

### Error Handling
- All file I/O returns `std::expected<T, std::string>` (C++23)
- Batch errors logged but don't halt processing (graceful failure)
- UI displays error count + summary on completion

---

## Assumptions & Explicit Decisions

1. **Networking**: No cloud upload; all processing local to disk
2. **Batch Size**: Support up to 10,000 files per batch (configurable via AppSettings if needed)
3. **Concurrency Level**: Default 4 worker threads (tpl executor default); configurable via environment variable
4. **Output Location**: Subfolder mode (default) or same directory (opt-in)
5. **Timestamps**: Incremental shift per file (±1 to N seconds) to avoid all images having identical timestamps
6. **GPS**: Randomize within configurable radius (default 50m) or use fixed coordinates from preset
7. **JPEG Quality**: Fixed 85% (PIL JPEG encoder default); can be exposed as config knob later
8. **Video Codecs**: Passthrough (copy video stream); no re-encoding (preserve original bitrate/codec)
9. **File Permissions**: Inherit from source files; apply umask on output files
10. **Logging**: INFO level by default; configurable via config.json or CLI flag

---

## Sign-Off Checklist
- [ ] Confirm all 10 dependencies can be fetched via CMake FetchContent
- [ ] Approve EXIF library choice (Exiv2 vs. raw APP1 builder)
- [ ] Approve video metadata strategy (TagLib vs. raw MP4 atoms)
- [ ] Approve default iPhone device presets (request custom presets if needed)
- [ ] Confirm three-tab UI layout matches design.md requirements
- [ ] Approve test coverage targets (unit + integration + manual testing)
