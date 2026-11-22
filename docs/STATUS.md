# spoofiq Development Summary

## Current Status: Alpha 1 (v0.2.0) + UI Polish

### Releases Completed

#### ✅ MVP (v0.1.0) - Proof of Concept
- Single-image processing with hardcoded iPhone metadata
- Basic CLI interface
- EXIF injection verified
- **Tag**: `git tag v0.1.0`

#### ✅ Alpha 1 (v0.2.0) - Modular Architecture
- Complete restructuring into clean modules (core/, io/, proc/)
- Independent compilation targets for fast incremental builds
- Dependencies integrated: json, spdlog (available)
- Documentation: `docs/ARCHITECTURE.md`
- **Tag**: `git tag v0.2.0`

#### ✅ UI Polish (Post v0.2.0)
- Fullscreen seamless UI filling entire OS window
- Professional tab-based navigation (Batch Spoofer, Presets, Console)
- Removed all template placeholder panels
- Suppressed background console window
- **Commit**: `be71c86`

---

## Current Capabilities

### Image Processing (Functional)
- Load images: PNG, JPG, BMP, TIF via stb_image
- Convert to RGB automatically
- Inject EXIF metadata (Apple iPhone 16 Pro Max)
- Save as JPEG with metadata
- Output naming: `IMG_0001.jpg`, `IMG_0002.jpg`, etc.

### UI / UX
- Fullscreen desktop application window
- 3-tab interface with clean design
- File path input fields
- Status messages (success/error)
- Next index display
- Design sense styling (dark tonal, cyan accent, pill buttons)

### Project Structure
```
├── vendor/stb/           (vendored headers, committed)
├── include/aw/spoofiq/
│   ├── core/            (types, metadata)
│   ├── io/              (image/metadata I/O abstractions)
│   ├── proc/            (image/video pipelines)
│   ├── config/          (stub for Alpha 2)
│   ├── logging/         (stub for Beta 1)
│   └── ui/              (ImGui panels)
├── src/                 (implementations)
├── tests/               (Catch2 test suite)
├── docs/
│   ├── task.md          (original requirements)
│   ├── design.md        (UI/UX spec)
│   ├── PLAN.md          (versioned roadmap)
│   └── ARCHITECTURE.md  (modular design)
└── build/               (CMake output, .gitignored)
```

---

## Build & Run

### Build Release Binary
```bash
cd spoofiq
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Run Application
```bash
./build/spoofiq_app
```
- Fully functional desktop GUI
- ESC to quit
- No background console window

### Run Tests
```bash
./build/spoofiq_tests
```

---

## Next Steps: Alpha 2 (v0.3.0)

**Estimated**: 6-8 hours

### ConfigManager + JSON Persistence
- Migrate app settings to nlohmann/json
- Create device preset editor UI
- Save/load presets (iPhone models)
- Master-detail UI in "Presets & Devices" tab

### Key Files to Create
- `include/aw/spoofiq/config/config_manager.hpp`
- `src/config/config_manager.cpp`
- `include/aw/spoofiq/ui/panels/presets_panel.hpp`
- `src/ui/panels/presets_panel.cpp`

### No Architectural Changes Needed
- All module structure already in place
- Just consuming nlohmann/json that we already FetchContent'd

---

## Known Limitations

### MVP/Alpha 1 Scope
- ❌ Batch processing (single file at a time)
- ❌ Video file support (video_processor.cpp is stub)
- ❌ Full EXIF/GPS control (hardcoded values only)
- ❌ Logging framework (spdlog available, Console tab is stub)
- ❌ Concurrency (sequential processing only)
- ❌ File browser dialog (manual path entry only)

### Deferred Dependencies (Environment Issues)
- TagLib v2.0.2 disabled (linker bus error) - will re-enable in Alpha 3
- nativefiledialog-extended disabled (needs GTK3) - will integrate in Beta 3
- Exiv2 disabled (complex dependencies) - full EXIF in Alpha 3

---

## Design Decisions

### Architecture
1. **Modular C++ with FetchContent**: Clear separation of concerns, incremental compilation
2. **C++23 strict**: No exceptions, using std::expected for errors
3. **ImGui for UI**: Immediate mode GUI, easy to iterate
4. **Event bus (tobus)**: Zero coupling between core and UI (Beta 2+)
5. **Coroutines (tpl)**: Async batch processing with cancellation (Beta 2+)

### Why This Approach
- **Maintainability**: Modular structure scales to 50K+ LOC without becoming a mess
- **Developer Experience**: Fast incremental builds, clear dependency graph
- **Correctness**: C++23 strict mode catches mistakes early
- **Performance**: Static linking, no runtime dependencies, optimized builds

---

## Versioning & Tags

```
v0.1.0  MVP - Single image processing proof-of-concept
v0.2.0  Alpha 1 - Modular architecture refactoring
        UI Polish - Professional standalone application appearance
→ v0.3.0 Alpha 2 - JSON config system (next)
  v0.4.0 Alpha 3 - Full EXIF/video metadata support
  v0.5.0 Beta 1 - Logging framework
  v0.6.0 Beta 2 - Async batch processing via coroutines
  v0.7.0 Beta 3 - Three-tab UI complete
  v0.8.0 RC 1 - GPS randomization, timestamp controls
  v0.9.0 RC 2 - Polish, error handling
  v0.9.5 RC 3 - Documentation, CI/CD
  v1.0.0 Release - Final verification
```

---

## Development Workflow

### For Each Version
1. Plan scope (see `docs/PLAN.md`)
2. Implement features in modular structure
3. Run tests: `./build/spoofiq_tests`
4. Manual testing: `./build/spoofiq_app`
5. Commit with detailed message
6. Tag release: `git tag vX.Y.Z`
7. Request approval before next version

### Quick Commands
```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release -DSPOOFIQ_BUILD_TESTS=ON

# Build everything
cmake --build build --config Release

# Fast rebuild (after small change)
make -C build spoofiq_app spoofiq_tests

# Run tests
./build/spoofiq_tests

# Run app
./build/spoofiq_app
```

---

## File Locations Reference

### Source Code
- **Core types**: `include/aw/spoofiq/core/types.hpp`
- **Image I/O**: `include/aw/spoofiq/io/image_io.hpp`, `src/io/image_io.cpp`
- **Metadata**: `include/aw/spoofiq/io/metadata_io.hpp`, `src/io/metadata_io.cpp`
- **Image processor**: `include/aw/spoofiq/proc/image_processor.hpp`, `src/proc/image_processor.cpp`
- **Video processor**: `include/aw/spoofiq/proc/video_processor.hpp`, `src/proc/video_processor.cpp` (stub)
- **UI root**: `src/ui.cpp` (fullscreen rendering)
- **Batch panel**: `include/aw/spoofiq/ui/panels/batch_spoofer_panel.hpp`, `src/ui/panels/batch_spoofer_panel.cpp`
- **Main app**: `src/main.cpp`

### Build System
- **Top-level CMake**: `CMakeLists.txt`
  - Modular library targets
  - FetchContent declarations
  - Platform-specific settings

### Documentation
- **Requirements**: `docs/task.md`, `docs/design.md`
- **Implementation Plan**: `docs/PLAN.md`
- **Architecture**: `docs/ARCHITECTURE.md`

---

## Quality Assurance

### Testing
- Catch2 v3 test framework integrated
- Tests location: `tests/*.cpp`
- Run via: `./build/spoofiq_tests`

### Code Style
- C++23 strict mode (no exceptions)
- clang-format ready (config in root)
- Namespaces: `aw::spoofiq::core`, `aw::spoofiq::io`, `aw::spoofiq::proc`, `aw::spoofiq::ui`

### Build Verification
- Clean builds verified on Linux
- All targets compile without warnings
- Release build size: 1.8MB (single executable)

---

**Last Updated**: 2026-11-23T04:24:00+07:00
**Current Version**: v0.2.0 + UI Polish
**Next Target**: Alpha 2 (v0.3.0) - JSON Config System
