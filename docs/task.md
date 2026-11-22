You are a Senior C++23 Developer and Desktop Application Architect specializing in Dear ImGui.

### Project Context
We are building **spoofiq** — a native C++23 desktop utility with a Dear ImGui interface for batch processing, converting, and spoofing media file metadata (images and videos) to mimic authentic Apple iPhone assets.

**IMPORTANT:** At this stage, **DO NOT write the final implementation code**. Your task is to audit my existing project template, my custom C++23 libraries (`tpl` and `tobus`), and the original Python script, evaluate the required dependencies for `CMake FetchContent`, and produce a detailed, step-by-step architectural implementation plan.

### Input Paths
1. **Path to my existing C++23 / CMake / ImGui / Catch2 project template:** `[INSERT_PROJECT_PATH]`
2. **Path to the original Python script:** `[INSERT_SCRIPT_PATH.py]`
3. **My custom C++23 libraries (integrate via `FetchContent` or local paths — specify the best approach):**
   - **`tpl`** (`https://github.com` or `[PATH_TO_TPL]`) — C++23 coroutines, `Task<T>`, `when_all`, `when_any`, `CancellationToken`.
   - **`tobus`** (`https://github.com` or `[PATH_TO_TOBUS]`) — zero-copy in-process event bus built on C++23 coroutines (`aw::tobus::EventBusFactory::create()`).

### Core Stack & Architectural Requirements
- **Language Standard:** Strict **C++23** (use `std::expected` for error handling instead of exceptions, `std::format` for `IMG_{:04d}` naming and date formatting, `std::filesystem`).
- **Concurrency & Decoupling (Critical Requirement):**
  - All batch file processing must be built on C++23 coroutines using **`tpl`** (`Task<T>`, `when_all` for parallel execution, and `CancellationToken` to abort processing via the UI "Stop" button).
  - Communication between the backend (`spoofiq::core`) and frontend (`spoofiq::ui`) must be strictly decoupled using the **`tobus`** event bus (publishing events for progress updates, file completion, and batch status) so the UI remains completely agnostic of background execution details.
- **Build System & Testing:** All missing dependencies must be fetched via `CMake FetchContent` without requiring manual system package installations. Unit tests for the core layer must be written using **Catch2 v3**.

### Target Libraries to Verify & Integrate
Check compatibility with my existing `CMakeLists.txt`, specify pinned stable release tags, and provide CMake cache flags to disable unnecessary tests/examples:
1. **Configuration & Presets:** `nlohmann/json` — for persisting app settings and custom device presets in JSON.
2. **Logging:** `gabime/spdlog` — for structured logging to both a file (`spoofiq.log`) and a custom thread-safe `ImGuiConsoleSink` connected to the UI.
3. **Image Processing:** `nothings/stb` (`stb_image` + `stb_image_write`) — for decoding input images (JPG, PNG, BMP, TIF, etc.) and re-encoding them into clean JPEGs with proper alpha-channel handling (RGBA -> RGB).
4. **JPEG EXIF Metadata:** `Exiv2/exiv2` (or a lighter alternative / raw APP1 EXIF segment builder — evaluate trade-offs and choose the fastest, most reliable option to build) for injecting camera, lens, timestamp, and GPS tags.
5. **Video Metadata (MP4/MOV):** `taglib/taglib` — for stripping existing tags and writing QuickTime atoms (`©nam`, `©day`, `©too`, `©xyz`).
6. **Native File Dialogs:** `btzy/nativefiledialog-extended` (NFD) — for OS-native folder selection dialogs (if not already present in the template).
7. **UI / Testing:** `ocornut/imgui`, `glfw/glfw`, `catchorg/Catch2` (verify what is already in my template to avoid duplication).

### Required Steps

#### Step 1: Audit Existing Codebase & Python Script
- Inspect `CMakeLists.txt`, directory layout, and entry point in my project template.
- Inspect the API and CMake targets of **`tpl`** and **`tobus`** to ensure seamless integration.
- Analyze the original Python script: document supported file extensions, binary EXIF tag structure (0th, Exif, GPS), QuickTime/MP4 atoms, and `IMG_XXXX` naming rules.

#### Step 2: Architecture & Config Design (Event-Driven + Coroutines)
- Separate the codebase into `spoofiq::core` (headless business logic) and `spoofiq::ui` (Dear ImGui layer).
- Design **`tobus`** event contracts (e.g., `ProgressEvent`, `FileFinishedEvent`, `BatchCompletedEvent`) and a thread-safe mechanism to consume/render state updates within the ImGui frame loop.
- Design the **`tpl`** coroutine pipeline supporting `CancellationToken` and `when_all` (ensure thread-safe atomic allocation of unique `IMG_XXXX` indices during concurrent execution).
- Design `ConfigManager` (`nlohmann/json`): automatic load/save on startup and shutdown (last input directory, subfolder export toggle, GPS/timestamp randomization settings) and a device preset system (iPhone model, iOS version, lens specs, focal length, aperture, ISO, base GPS). Generate a default config with popular iPhone presets if none exists.

#### Step 3: Detailed Implementation Plan (Milestones)
Break down development into clear, sequential milestones:
1. **Dependency Integration:** Updating `CMakeLists.txt` (`FetchContent` for `tpl`, `tobus`, `nlohmann/json`, `spdlog`, `stb`, `TagLib`, `NFD`, and EXIF library) and verifying a clean build.
2. **Core Layer & Persistence (`spoofiq::core`):** Implementing `ConfigManager`, image pipeline, video pipeline, `spdlog` custom sink, and fixing flaws from the original script (preventing accidental overwrites of existing `IMG_XXXX` files via auto-indexing or subfolder export, gracefully skipping unsupported formats like `.heic`).
3. **Async Orchestrator (`tpl` + `tobus`):** Implementing the coroutine-based batch processor with `tobus` event publishing and `CancellationToken` support.
4. **Unit Testing (Catch2 v3):** Test plan covering EXIF/MP4 metadata generation, JSON config serialization, index collision avoidance, and cancellation.
5. **UI Layer (`spoofiq::ui`):** Implementing the Dear ImGui interface (details to be provided in the UI Design specification).

### Output Format
Deliver a structured Markdown document containing technical justifications for library choices, potential pitfalls (thread-safety between `tobus`/`spdlog` and ImGui, concurrent index allocation in `tpl::when_all`, MP4 atom manipulation, UTF-8 Windows paths), and a step-by-step checklist. Ask for my confirmation before proceeding to Milestone 1.
