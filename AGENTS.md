# AGENTS.md

Rules for working with spoofiq to prevent common mistakes.

## Before making changes

- **NO PUSH without explicit permission.** Only commit locally and ask before pushing.
- **NO TAG CREATION.** CI creates all tags. Never create tags manually.
- **Ask before changing workflows.** Release workflow is fragile; discuss changes first.

## Git workflow

- Commit locally with clear messages (Conventional Commits: `feat:`, `fix:`, `docs:`, etc.)
- Stage only relevant files: `git add <file>` (never `git add .`)
- Do NOT push to `origin` (ai-warevo). Push to fork or ask first.
- Create tags only when CI instructs, never manually.

## CMake and builds

- **Always test locally** before claiming it works:
  - Linux: `cmake --preset clang18-libcxx && cmake --build build/clang18-libcxx && ctest --preset clang18-libcxx`
  - Windows: `cmake --preset Release-msvc && cmake --build build/Release-msvc`
- **Release builds skip tests:** `-DSPOOFIQ_BUILD_TESTS=OFF` to speed up builds
- **FetchContent for dependencies.** Never vendored code. CMake modules from template_cpp via FetchContent.
- **Use correct cmake-build action parameters:** `source_dir`, `build_dir`, `build_type`, `config`, `extra_args`, `parallel`, `install` (no `build_preset`).

## Windows compatibility

- Replace `popen`/`pclose` with `_popen`/`_pclose` using platform macros
- Replace `getenv` with safe `_dupenv_s` on Windows
- HOME environment variable: use `$HOME` on Unix, `%USERPROFILE%` on Windows
- Test both platforms before merging (CI will catch it, but test locally first)
- OpenGL context must handle both GLFW (Unix) and Windows native window
- ImGui rendering path may differ on Windows vs Linux

## Release workflow

- **CHANGELOG.md is the source of truth.** Release Notes come from it.
- **Update CHANGELOG under `## [Unreleased]`** for each change:
  - `### Added` — new features
  - `### Changed` — refactoring, build system changes
  - `### Fixed` — bug fixes
- **When releasing:**
  1. Update CHANGELOG: rename `## [Unreleased]` to `## [X.Y.Z] - YYYY-MM-DD`
  2. Commit: `git commit -m "docs: update CHANGELOG for vX.Y.Z"`
  3. Wait for CI or ask — CI creates the tag and builds binaries
- **Release Notes generation:**
  - `what-included` and `quick-start` defaults are overridden to empty strings (no boilerplate)
  - Only CHANGELOG content appears in GitHub Release

## Common mistakes to avoid

1. **Pushing without permission** — Ask first
2. **Creating tags manually** — CI does this
3. **Testing only on one platform** — Test Linux + Windows
4. **Forgetting to update CHANGELOG** — It's required for releases
5. **Using `build_preset` in cmake-build action** — Use `build_type` and `config` instead
6. **Copying boilerplate from ai-warevo/.github examples** — Customize for spoofiq (remove defaults)
7. **Assembling test artifacts in release** — Use `-DSPOOFIQ_BUILD_TESTS=OFF`
8. **Platform-specific includes** — Use `#ifdef _WIN32` / `#ifdef __POSIX__` with guards

## File locations and responsibilities

- `CMakeLists.txt` — Root CMake config (uses FetchContent for aw_cmake_modules)
- `CMakePresets.json` — Build presets (copied from templates/_common/files/, same across all projects)
- `.github/workflows/release.yml` — Build binaries + publish release (customize `what-included`/`quick-start`)
- `CHANGELOG.md` — Release notes source (required format: `## [X.Y.Z] - YYYY-MM-DD`)
- `AGENTS.md` — This file (project-specific rules)
- `src/` — Implementation files (all platform-safe)
- `src/overlay_*.cpp` — Platform-specific overlay implementations (overlay_posix.cpp, overlay_win32.cpp)
- `src/main.cpp` — Entry point and GUI initialization

## Testing

```bash
# Configure
cmake --preset clang18-libcxx

# Build
cmake --build build/clang18-libcxx

# Test
ctest --preset clang18-libcxx --output-on-failure
```

Tests cover:
- Configuration loading and parsing
- Core spoofing logic
- I/O operations
- UI integration

## Cross-platform gotchas

- **POSIX vs Windows paths:** Use `std::filesystem::path` (platform-agnostic)
- **Environment variables:** Always use the safe wrapper or platform macros
- **Process execution:** Use abstracted process runner with platform-specific backends
- **Window overlay:** Different implementations for Windows (window hooks) vs Unix (X11/Wayland)
- **OpenGL context:** May require different initialization on different platforms
- **ImGui rendering:** Ensure consistent rendering across platforms (use uniform rendering backend)

## GUI-specific considerations

- **ImGui theme consistency** — Theme should work across platforms
- **Overlay integration** — Must not break window manager on any platform
- **Input handling** — Coordinate systems must be normalized across platforms
- **Resource loading** — Use relative paths from executable or packaged resources
- **Font rendering** — Ensure fonts are available on target platforms

## C++ Header Guards
- Headers use standard C++ include guards (`#ifndef <GUARD>` / `#define <GUARD>` / `#endif // <GUARD>`) with unique `UPPER_SNAKE_CASE` macro names derived from the project and relative file path (e.g., `PROJECT_MODULE_FILE_HPP`).

## C++ Comments

- **Explain why, not what:** Delete redundant comments that narrate code (e.g., `// increment i`, section markers). Assume reader knows C++.
- **Keep valuable comments:** Explain intent, business logic, non-obvious edge cases, workarounds, or architectural constraints.
- **Use block comments for multi-line:** Prefer `/* */` or `/** */` (Doxygen) over stacked `//` lines. Use `//` only for ultra-short inline notes and tool directives (`// NOLINT`).

## Documentation

- Update README.md if behavior changes
- Keep this file (AGENTS.md) up-to-date with new rules
- Link to code examples when rules are complex
- Document platform-specific behavior in relevant modules

## Async Architecture: aw::tobus and aw::tpl

**Use aw::tobus (event bus) for UI ↔ Core communication instead of direct coupling.**

- **Never pass window handles, callbacks, or UI state to core/business logic.** This creates tight coupling and makes code hard to test.
- **Define events in `include/aw/spoofiq/core/events.hpp`.** UI changes (theme, settings) and core updates (progress, completion) go through the event bus.
- **Publishers emit events; subscribers react.** Use `EventBusManager::get()->publish(event)` for sends and `event_bus->subscribe<EventType>(handler)` for receives.
- **Example:** Instead of `AppearancePanel::set_window_handle(window)` + direct DWM calls, emit `ThemeChangeEvent` and subscribe in `main.cpp` to handle platform-specific updates.

**Use aw::tpl for background work instead of blocking the UI thread.**

- **Never block ImGui's render loop** with long-running operations (file I/O, image processing, batch work).
- **Delegate to `TaskExecutor`** via `aw::tpl` coroutines. Publish completion events when done.
- **Example:** Batch processing publishes `ProgressEvent`, `FileFinishedEvent`, `BatchCompletedEvent` instead of calling UI methods directly.

**Benefits:**
- ✅ Core logic has zero dependency on UI/platform code
- ✅ Testable: mock the event bus instead of UI fixtures
- ✅ Responsive: long work doesn't freeze the frame
- ✅ Scalable: add new subscribers without changing publishers
- ✅ Platform-agnostic: Windows DWM updates, Linux DDE, macOS NSApp all react to the same event


