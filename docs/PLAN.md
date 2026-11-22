# Spoofiq Implementation Plan: C++23 Desktop Metadata Forge

## Status

**Current Release**: v0.8.0-beta4 ✅
- Cancellation token + Cancel button
- Progress bar with file count and percentage
- Thread-safe atomic updates
- Events still published to console
- Next: RC 1 (v0.8.0) - GPS randomization, timestamps

---

## Executive Summary
Build a native C++23 desktop application using Dear ImGui to batch process media files (images and videos) and spoof their metadata to mimic authentic Apple iPhone assets. The architecture separates concerns into `spoofiq::core` (business logic) and `spoofiq::ui` (ImGui layer), decoupled via the `tobus` event bus, with concurrent batch processing via `tpl` coroutines.

---

## Release Strategy Overview

This project follows an incremental delivery model with clearly scoped versions to minimize complexity and ensure stability at each stage:

- **MVP (v0.1.0)**: Core functionality proof-of-concept (single-threaded, basic UI)
- **Alpha 1-3 (v0.2.x-v0.4.x)**: Foundation layers (dependencies, config, basic processors)
- **Beta 1-3 (v0.5.x-v0.7.x)**: Full feature set (concurrency, event bus, full UI)
- **RC 1-3 (v0.8.x-v0.9.x)**: Polish, testing, bug fixes
- **Release (v1.0.0)**: Production-ready

---

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

## Target Architecture (Final v1.0.0)

### High-Level Subsystem Overview
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

---

# Version Roadmap

## MVP (v0.1.0) - Proof of Concept
**Goal**: Single image processing with hardcoded metadata, minimal UI
**Duration**: 4-6 hours
**Dependencies**: stb_image, stb_image_write only

### Features
- [x] Single-file image processing (PNG/JPG → JPG with hardcoded EXIF)
- [x] Simple single-window UI: file path input + "Process" button
- [x] Hardcoded iPhone 16 Pro Max metadata
- [x] Sequential processing (no concurrency)
- [x] Basic error messages (console output)

### Deliverables
1. `ImageProcessor::process_single()` - loads image, converts to RGB, saves as JPEG
2. Minimal EXIF builder (raw APP1 segment with hardcoded values)
3. Simple ImGui window with text input + button
4. Manual verification: output file has correct EXIF tags (via `exiftool`)

### Testing
- Manual test: process 1 PNG, 1 JPG → verify outputs
- No automated tests yet

### Excluded from MVP
- Video processing
- JSON config
- Batch processing
- Multiple presets
- Event bus
- Logging framework
- Concurrency

---

## Alpha 1 (v0.2.0) - Dependencies & Build Foundation
**Goal**: Integrate all required libraries, establish build system
**Duration**: 4-6 hours
**Focus**: Infrastructure, no new features

### Features
- [x] CMake FetchContent for all dependencies
- [x] Clean build verification
- [x] Library version pinning
- [x] Test framework setup

### Deliverables
1. Updated `CMakeLists.txt`:
   - `nlohmann/json` (v3.11.3)
   - `gabime/spdlog` (v1.14.1)
   - `Exiv2/exiv2` (v0.28.3) OR `libtiff/libtiff` (v4.6.0) for raw EXIF building
   - `taglib/taglib` (v2.0.2)
   - `btzy/nativefiledialog-extended` (v1.2.1)
2. Vendor `stb` headers in `src/vendor/stb/` directory
3. Build verification on target platforms (Linux/Windows/macOS if applicable)
4. CMake cache flags to disable examples/tests/docs for all dependencies

### Testing
- Build test: `cmake -B build -DSPOOFIQ_BUILD_TESTS=ON && cmake --build build`
- Verify no linker errors
- Run existing Catch2 tests

### Excluded
- Using new libraries (integration only, not consumption)
- New features

---

## Alpha 2 (v0.3.0) - ConfigManager & JSON Persistence
**Goal**: Configuration system with device presets
**Duration**: 6-8 hours
**Focus**: Data layer

### Features
- [x] `ConfigManager` class with load/save
- [x] `DevicePreset` struct (Make, Model, Software, lens specs, GPS)
- [x] Default `config.json` generation with 4 iPhone presets
- [x] Unit tests for config serialization

### Deliverables
1. `include/aw/spoofiq/config_manager.hpp`:
   ```cpp
   struct DevicePreset {
       std::string id, name, make, model, software, lens_model;
       float focal_length, focal_length_35mm, f_number;
       int iso;
       double base_latitude, base_longitude;
   };
   
   struct AppSettings {
       std::string last_input_directory;
       bool export_to_subfolder;
       std::string subfolder_name;
       int start_index;
       std::vector<DevicePreset> presets;
   };
   
   class ConfigManager {
       static std::expected<AppSettings, std::string> load();
       static std::expected<void, std::string> save(const AppSettings&);
       static std::filesystem::path get_config_path();
   };
   ```

2. Default presets:
   - iPhone 16 Pro Max (iOS 18.1)
   - iPhone 15 Pro (iOS 17.4)
   - iPhone 14 Pro (iOS 16.6)
   - iPhone 13 Pro (iOS 15.7)

3. Config file location: `~/.spoofiq/config.json` (Linux/macOS) or `%APPDATA%\spoofiq\config.json` (Windows)

4. Unit tests (`tests/config_manager_tests.cpp`):
   - Load missing config → generates defaults
   - Save/load round-trip
   - Invalid JSON → error handling
   - Preset lookup by ID

### Testing
- Unit tests via Catch2
- Manual: delete config, run app, verify defaults created

### Excluded
- UI integration (config used in backend only)
- GPS randomization logic

---

## Alpha 3 (v0.4.0) - Image & Video Processors (Synchronous)
**Goal**: Complete metadata injection for images and videos
**Duration**: 10-12 hours
**Focus**: Core business logic

### Features
- [x] `ImageProcessor` with Exiv2 EXIF injection
- [x] `VideoProcessor` with TagLib QuickTime atom injection
- [x] RGBA → RGB conversion
- [x] Error handling via `std::expected`
- [x] Batch processing (synchronous, no coroutines yet)

### Deliverables
1. `include/aw/spoofiq/image_processor.hpp`:
   ```cpp
   class ImageProcessor {
       static std::expected<void, std::string> process(
           const std::filesystem::path& input,
           const std::filesystem::path& output,
           const DevicePreset& preset
       );
   };
   ```

2. `include/aw/spoofiq/video_processor.hpp`:
   ```cpp
   class VideoProcessor {
       static std::expected<void, std::string> process(
           const std::filesystem::path& input,
           const std::filesystem::path& output,
           const DevicePreset& preset
       );
   };
   ```

3. EXIF metadata generation:
   - 0th IFD: Make, Model, Software, DateTime
   - Exif IFD: FNumber, ISO, FocalLength, LensModel, ComponentsConfiguration
   - GPS IFD: Latitude, Longitude

4. QuickTime atoms:
   - `©nam` (name: "Core Video")
   - `©day` (timestamp ISO 8601)
   - `©too` (software version)
   - `©xyz` (GPS string: "+37.3349-122.0090/")

5. Unit tests (`tests/image_processor_tests.cpp`, `tests/video_processor_tests.cpp`):
   - Process PNG → verify EXIF tags
   - Process RGBA PNG → verify RGB output (no alpha)
   - Process MP4 → verify QuickTime atoms
   - Invalid input → error handling

### Testing
- Unit tests with sample media files in `tests/fixtures/`
- Verification via `exiftool` and `ffprobe`

### Excluded
- Concurrency (synchronous loop)
- Event bus integration
- UI progress updates

---

## Beta 1 (v0.5.0) - Logging & Console UI
**Goal**: Structured logging with ImGui console viewer
**Duration**: 6-8 hours
**Focus**: Observability

### Features
- [x] spdlog logger singleton
- [x] Custom `ImGuiConsoleSink` class
- [x] Console tab in UI
- [x] Thread-safe log buffer

### Deliverables
1. `include/aw/spoofiq/logger.hpp`:
   ```cpp
   class ImGuiConsoleSink : public spdlog::sinks::base_sink<std::mutex> {
       void sink_it_(const spdlog::details::log_msg& msg) override;
       void flush_() override;
       std::vector<std::string> get_logs() const;
   };
   
   std::shared_ptr<spdlog::logger> get_logger();
   ```

2. Dual sinks:
   - File sink: `spoofiq.log` (rotating, 5MB max, 3 backups)
   - ImGui sink: ring buffer (10,000 entries)

3. Console Panel UI:
   - Color-coded log levels (green=info, yellow=warn, red=error)
   - Level filter checkboxes
   - Search bar (`ImGuiTextFilter`)
   - Auto-scroll toggle
   - "Clear" and "Copy to Clipboard" buttons

4. Integration:
   - Replace `std::cout` in processors with `SPDLOG_INFO()`, `SPDLOG_ERROR()`
   - Log batch start/completion, file processing, errors

### Testing
- Unit test: log 100 messages, verify buffer stores them
- Manual: open Console tab, verify log entries appear with correct colors

### Excluded
- Unread badge count (defer to Beta 3)

---

## Beta 2 (v0.6.0) - Event Bus & Async Orchestrator
**Goal**: Coroutine-based batch processing with tobus events
**Duration**: 8-10 hours
**Focus**: Concurrency & decoupling

### Features
- [x] `BatchOrchestrator` with `tpl::Task` and `tpl::when_all`
- [x] tobus event contracts (ProgressEvent, FileFinishedEvent, BatchCompletedEvent)
- [x] CancellationToken integration
- [x] Atomic `IMG_XXXX` index allocation

### Deliverables
1. `include/aw/spoofiq/events.hpp`:
   ```cpp
   struct ProgressEvent {
       int total_files, processed_count;
       std::string current_filename;
       float progress_percent;
   };
   
   struct FileFinishedEvent {
       std::string input_filename, output_filename;
       bool success;
       std::string error_message;
   };
   
   struct BatchCompletedEvent {
       int total_images, total_videos, total_errors;
   };
   
   struct BatchCancelledEvent {};
   ```

2. `include/aw/spoofiq/batch_orchestrator.hpp`:
   ```cpp
   class BatchOrchestrator {
       tpl::Task<void> run_batch(
           const std::vector<std::filesystem::path>& files,
           const DevicePreset& preset,
           const std::filesystem::path& output_dir,
           std::atomic<int>& next_index,
           tpl::CancellationToken token
       );
   };
   ```

3. Concurrency strategy:
   - Use `tpl::when_all()` to process files in parallel
   - Each coroutine claims index via `next_index.fetch_add(1)`
   - Publish `ProgressEvent` after each file
   - Publish `BatchCompletedEvent` when all tasks complete
   - Poll `token.is_cancelled()` before starting each file

4. Unit tests (`tests/batch_orchestrator_tests.cpp`):
   - Process 10 files → verify 10 unique indices
   - Concurrent stress test: 100 files → no index collisions
   - Cancellation: start batch, cancel after 5 files → verify graceful shutdown
   - Event emission: verify ProgressEvent count matches file count

### Testing
- Unit tests with temp directory + sample files
- Verify events published to tobus subscribers

### Excluded
- UI integration (event bus works, but UI doesn't subscribe yet)

---

## Beta 3 (v0.7.0) - Full Three-Tab UI
**Goal**: Complete user interface with all features
**Duration**: 12-16 hours
**Focus**: User experience

### Features
- [x] Tab 1: Batch Spoofer (file selection, progress, controls)
- [x] Tab 2: Presets & Devices (CRUD editor)
- [x] Tab 3: Console (logs with unread badge)
- [x] Event bus subscription in UI
- [x] NFD folder browser

### Deliverables
1. **Tab 1: Batch Spoofer**
   - Folder path input + "Browse..." button (NFD)
   - Drag-and-drop folder (ImGui payload)
   - Export subfolder checkbox + name input
   - Preset selector (`ImGui::Combo`)
   - "Start" button → triggers `BatchOrchestrator::run_batch()`
   - "Stop" button (red, visible during batch) → signals cancellation
   - Progress bar + file count + current filename
   - Status line: "Processing IMG_0042.jpg | Images: 21 | Videos: 3 | Errors: 0"

2. **Tab 2: Presets & Devices**
   - Left pane: preset list (`ImGui::Selectable`)
   - Right pane: form fields (Name, Make, Model, Software, Lens, Focal Length, F-Number, ISO, Lat, Lon)
   - "Add Preset" button → creates empty preset
   - "Duplicate" button → clones selected preset
   - "Delete" button → removes preset (with confirmation modal)
   - "Save" button → calls `ConfigManager::save()`
   - "Reset" button → reverts unsaved changes

3. **Tab 3: Console**
   - Subscribe to `ImGuiConsoleSink` buffer
   - Unread badge in tab label: "Console (12)"
   - Auto-scroll to bottom when new logs arrive
   - Search + level filters

4. Event bus wiring:
   - UI subscribes to `ProgressEvent` → updates progress bar
   - UI subscribes to `FileFinishedEvent` → increments counters
   - UI subscribes to `BatchCompletedEvent` → shows completion modal
   - UI subscribes to `BatchCancelledEvent` → resets state

### Testing
- Manual walkthrough of full workflow
- Load folder → select preset → start batch → verify progress updates
- Cancel batch mid-flight → verify no crash
- Edit preset → save → restart app → verify persistence

### Excluded
- Advanced features (GPS randomization, timestamp shift, custom datetime picker)

---

## RC 1 (v0.8.0) - Advanced Features
**Goal**: GPS randomization, timestamp controls, edge case handling
**Duration**: 6-8 hours
**Focus**: Feature completeness

### Features
- [x] GPS coordinate randomization (radius in meters)
- [x] Timestamp modes (current time vs. custom datetime)
- [x] Timestamp shift per file (incremental N seconds)
- [x] Skip existing `IMG_XXXX` files option
- [x] Auto-detect next free index
- [x] HEIC file graceful skip

### Deliverables
1. GPS randomization:
   - Add `gps_randomize` and `gps_radius_m` to AppSettings
   - Implement Haversine formula to offset lat/lon by random bearing + distance
   - UI: checkbox + radius input in Tab 1

2. Timestamp controls:
   - Radio buttons: "Current Time" vs. "Custom Date/Time"
   - Date/time picker (via ImGui widgets or text input)
   - Checkbox: "Shift timestamp per file" + seconds input
   - Apply incremental shift during batch processing

3. Index management:
   - Pre-flight scan: read output directory, find max `IMG_XXXX` index
   - Initialize `std::atomic<int>` to max+1
   - UI: dropdown for "Auto-detect" vs. "Manual start index"

4. Format handling:
   - Skip `.heic` files with `SPDLOG_WARN()` message
   - UI tooltip: "Supported: JPG, PNG, BMP, TIF, MP4, MOV"

### Testing
- Unit test: GPS randomization → verify coordinates within radius
- Unit test: timestamp shift → verify sequential increments
- Manual: create folder with `IMG_0005.jpg`, set auto-detect → verify starts at `IMG_0006.jpg`

---

## RC 2 (v0.9.0) - Polish & Bug Fixes
**Goal**: UI/UX improvements, error handling, stability
**Duration**: 6-8 hours
**Focus**: Quality

### Features
- [x] Error modal dialogs
- [x] Input validation (folder exists, preset selected)
- [x] UTF-8 path handling on Windows
- [x] Responsive UI during batch (frame budget for events)
- [x] App icon and window title
- [x] Keyboard shortcuts (Ctrl+O for browse, Esc to cancel batch)

### Deliverables
1. Error handling:
   - Modal dialog for critical errors (folder not found, permission denied)
   - Non-blocking error toasts for file-level failures
   - Batch continues on per-file errors

2. Input validation:
   - Disable "Start" button if folder path empty or preset not selected
   - Show warning icon if output folder already has `IMG_XXXX` files

3. Platform-specific:
   - Windows: UTF-8 path conversion for stb_image and TagLib
   - macOS: test with non-ASCII paths (Chinese, Japanese)

4. Performance:
   - Limit event processing per frame (50 events max) to prevent UI stutter
   - Batch processing on dedicated thread pool (not blocking main thread)

5. UI polish:
   - Consistent spacing and alignment
   - Tooltips for all controls
   - Status bar with app version

### Testing
- Stress test: 1000+ file batch
- Error injection: corrupted MP4, unreadable PNG, permission denied
- Platform testing: Windows 10/11, Ubuntu 22.04, macOS 13+

---

## RC 3 (v0.9.5) - Documentation & Integration Testing
**Goal**: User documentation, comprehensive tests, release prep
**Duration**: 4-6 hours
**Focus**: Completeness

### Features
- [x] User guide (README.md)
- [x] Developer documentation (CONTRIBUTING.md)
- [x] Integration test suite
- [x] Build instructions for all platforms

### Deliverables
1. `README.md`:
   - Feature overview
   - Installation instructions (build from source)
   - Usage guide with screenshots
   - Supported formats and limitations

2. `CONTRIBUTING.md`:
   - Build instructions (CMake commands)
   - Code style guide (.clang-format)
   - Testing guidelines
   - Architecture overview (link to PLAN.md)

3. Integration tests (`tests/integration_tests.cpp`):
   - Full workflow: mixed media folder → batch processing → verify output metadata
   - Error recovery: batch with 1 corrupted file → verify batch completes
   - Cancellation: start batch, cancel after 50% → verify graceful shutdown

4. CI/CD (GitHub Actions):
   - Build matrix: Linux (GCC 13, Clang 18), Windows (MSVC 2022), macOS (Clang)
   - Run tests on all platforms
   - Upload artifacts (executables)

### Testing
- Manual user acceptance testing
- Fresh clone → build → run → process sample folder
- Verify no console errors or warnings

---

## Release (v1.0.0) - Production Ready
**Goal**: Stable, polished, ready for distribution
**Duration**: 2-4 hours (final verification)

### Features
- [x] Version bump to 1.0.0
- [x] Release notes (CHANGELOG.md)
- [x] Binary distribution (optional)

### Deliverables
1. Final verification:
   - All tests pass on 3 platforms
   - No known critical bugs
   - Documentation complete

2. `CHANGELOG.md`:
   - List all features from MVP to v1.0.0
   - Breaking changes (if any)
   - Known limitations

3. Release artifacts:
   - Tagged commit: `v1.0.0`
   - GitHub release with binaries (optional)
   - User guide published

### Success Criteria
- Application runs without crashes on typical workflows
- Metadata injection verified with `exiftool` and `ffprobe`
- User can process 1000+ files without errors
- Build instructions work on fresh systems

---

## Implementation Strategy

### Step-by-Step Approach
1. **Complete each version in sequence** (do not skip ahead)
2. **Test before moving to next version** (all tests pass + manual verification)
3. **Git tag each version** (`git tag v0.1.0`, `git tag v0.2.0`, etc.)
4. **Document blockers immediately** (log issues that prevent version completion)
5. **No feature creep** (defer out-of-scope requests to future versions)

### Version Approval Process
At the end of each version:
1. Run all tests (`ctest --output-on-failure`)
2. Manual smoke test (launch app, verify new features work)
3. Commit with message: `Release vX.Y.Z - <description>`
4. Tag commit: `git tag vX.Y.Z`
5. **Ask for approval before proceeding to next version**

### Risk Mitigation
- If a version takes >2x estimated time, stop and reassess scope
- If blocked by external dependency issue, document and propose workaround
- If test failure, fix before proceeding (no "we'll fix it later")

---

## Dependency Matrix

| Library | Version | Used In Version | Purpose |
|---------|---------|-----------------|---------|
| stb | master | MVP (v0.1.0) | Image I/O |
| nlohmann/json | v3.11.3 | Alpha 2 (v0.3.0) | Config persistence |
| spdlog | v1.14.1 | Beta 1 (v0.5.0) | Logging |
| Exiv2 | v0.28.3 | Alpha 3 (v0.4.0) | EXIF metadata |
| TagLib | v2.0.2 | Alpha 3 (v0.4.0) | QuickTime atoms |
| nativefiledialog-extended | v1.2.1 | Beta 3 (v0.7.0) | Folder browser |

---

## Sign-Off Checklist (Before Starting)
- [ ] Approve versioned roadmap (MVP → Alpha → Beta → RC → Release)
- [ ] Confirm step-by-step implementation (no parallel versions)
- [ ] Approve version approval process (test → commit → tag → ask)
- [ ] Confirm starting point: MVP (v0.1.0)
- [ ] Ready to begin implementation
