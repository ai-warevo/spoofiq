cmake_minimum_required(VERSION 3.23)

include(FetchContent)

# aw_cmake_modules: prefer local checkout, fallback to GitHub
set(AW_CMAKE_MODULES_LOCAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../template_cpp/cmake")
if(EXISTS "${AW_CMAKE_MODULES_LOCAL_PATH}/AWProject.cmake")
    message(STATUS "Using local aw_cmake_modules from ../template_cpp")
    list(APPEND CMAKE_MODULE_PATH "${AW_CMAKE_MODULES_LOCAL_PATH}")
else()
    message(STATUS "Local aw_cmake_modules not found, fetching from GitHub")
    FetchContent_Declare(
        aw_cmake_modules
        GIT_REPOSITORY https://github.com/adembudak/template_cpp.git
        GIT_TAG main
        GIT_SHALLOW ON
        SOURCE_SUBDIR cmake)
    FetchContent_MakeAvailable(aw_cmake_modules)
    list(APPEND CMAKE_MODULE_PATH "${aw_cmake_modules_SOURCE_DIR}/cmake")
endif()

include(AWProject)
aw_project_prepare(NAME spoofiq NAMESPACE aw::spoofiq)
project(${AW_PROJECT_NAME} VERSION ${AW_PROJECT_VERSION} LANGUAGES CXX)
aw_project_setup(PREFIX SPOOFIQ BUILD_TESTS BUILD_APP BUILD_OVERLAY)

set(SPOOFIQ_WARNINGS_AS_ERRORS ON CACHE BOOL "Treat warnings as errors")

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
aw_check_cxx23_runtime()

include(DependencyCache)

# ── Core Dependencies ──────────────────────────────────────────────────────────
# aw::tpl (TPL: Task Parallel Library)
if(NOT TARGET aw::tpl)
    aw_fetch_dependency(
        NAME aw_tpl
        GIT_REPOSITORY https://github.com/ai-warevo/tpl.git
        GIT_TAG main
        OPTIONS
            AW_TPL_BUILD_TESTS OFF
            AW_TPL_BUILD_EXAMPLES OFF
            AW_TPL_INSTALL OFF)
endif()

# aw::tobus (Event Bus)
if(NOT TARGET aw::tobus)
    aw_fetch_dependency(
        NAME aw_tobus
        GIT_REPOSITORY https://github.com/ai-warevo/tobus.git
        GIT_TAG main
        OPTIONS
            TOBUS_BUILD_TESTS OFF
            TOBUS_BUILD_EXAMPLES OFF
            TOBUS_INSTALL OFF)
endif()

# GLFW (windowing + input)
if(NOT TARGET glfw)
    # Platform-specific GLFW options
    set(_glfw_options
        GLFW_BUILD_DOCS OFF
        GLFW_BUILD_TESTS OFF
        GLFW_BUILD_EXAMPLES OFF
        GLFW_INSTALL OFF)
    
    if(UNIX AND NOT APPLE)
        # Linux: use X11 only (disable Wayland to avoid wayland-scanner requirement)
        list(APPEND _glfw_options GLFW_BUILD_X11 ON GLFW_BUILD_WAYLAND OFF)
    endif()
    
    aw_fetch_dependency(
        NAME glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        OPTIONS ${_glfw_options})
endif()

# Dear ImGui (docking branch)
if(NOT TARGET imgui)
    aw_fetch_dependency(
        NAME imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.91.8-docking)

    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)

    target_include_directories(imgui SYSTEM PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends)

    target_link_libraries(imgui PUBLIC glfw OpenGL::GL)
    set_target_properties(imgui PROPERTIES CXX_STANDARD 23)
endif()

# ── Future Dependencies (Alpha 2+) ─────────────────────────────────────────────
# nlohmann/json (v3.11.3) - Used in Alpha 2
if(NOT TARGET nlohmann_json)
    aw_fetch_dependency(
        NAME nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        OPTIONS
            JSON_BuildTests OFF
            JSON_Install OFF)
endif()

# spdlog (v1.14.1) - Used in Beta 1
if(NOT TARGET spdlog)
    aw_fetch_dependency(
        NAME spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.14.1
        OPTIONS
            SPDLOG_BUILD_TESTS OFF
            SPDLOG_BUILD_EXAMPLE OFF
            SPDLOG_BUILD_SHARED OFF
            SPDLOG_INSTALL OFF)
endif()

# STB (single-header image library)
if(NOT TARGET stb)
    aw_fetch_dependency(
        NAME stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master)
    
    # Create interface library for STB (header-only)
    add_library(stb INTERFACE)
    target_include_directories(stb SYSTEM INTERFACE ${stb_SOURCE_DIR})
endif()

# ── Catch2 (v3) - Testing framework ────────────────────────────────────────────
if(SPOOFIQ_BUILD_TESTS AND NOT TARGET Catch2::Catch2WithMain)
    aw_fetch_dependency(
        NAME Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.6.0
        OPTIONS
            CATCH_INSTALL_DOCS OFF
            CATCH_INSTALL_EXTRAS OFF)
endif()

# ── Modular spoofiq_core: Core Types (zero external deps) ───────────────────────
aw_add_library(TARGET spoofiq_core_types TYPE OBJECT
    SOURCES
        src/core/types.cpp
    PUBLIC_INCLUDE_DIRS include)

# ── Modular spoofiq_core: I/O Metadata Layer ──────────────────────────────────
aw_add_library(TARGET spoofiq_core_io_metadata TYPE OBJECT
    SOURCES
        src/io/metadata_io.cpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS spoofiq_core_types)

# ── Modular spoofiq_core: I/O Image Layer ───────────────────────────────────────
aw_add_library(TARGET spoofiq_core_io_image TYPE OBJECT
    SOURCES
        src/io/image_io.cpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS spoofiq_core_types spoofiq_core_io_metadata stb)

# ── Modular spoofiq_core: Processing - Image ──────────────────────────────────
aw_add_library(TARGET spoofiq_core_proc_image TYPE OBJECT
    SOURCES
        src/proc/image_processor.cpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS spoofiq_core_types spoofiq_core_io_image spoofiq_core_io_metadata spoofiq_core_logging)

# ── Modular spoofiq_core: Processing - Video (stub) ──────────────────────────
aw_add_library(TARGET spoofiq_core_proc_video TYPE OBJECT
    SOURCES
        src/proc/video_processor.cpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS spoofiq_core_types)

# ── Modular spoofiq_core: Media File Abstraction ────────────────────────────────
aw_add_library(TARGET spoofiq_core_media_file TYPE OBJECT
    SOURCES
        src/proc/media_file.cpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS spoofiq_core_types)

# ── Modular spoofiq_core: Batch Orchestrator ──────────────────────────────────────
aw_add_library(TARGET spoofiq_core_batch_orchestrator TYPE OBJECT
    SOURCES
        src/proc/batch_orchestrator.cpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS spoofiq_core_types spoofiq_core_proc_image spoofiq_core_proc_video spoofiq_core_logging spoofiq_core_config)

# ── Modular spoofiq_core: Async Batch Processor (aw::tpl + aw::tobus integration) ──
aw_add_library(TARGET spoofiq_core_async_batch_processor TYPE OBJECT
    SOURCES
        src/proc/async_batch_processor.cpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS spoofiq_core_types spoofiq_core_batch_orchestrator spoofiq_core_logging aw::tpl aw::tobus)

# ── Modular spoofiq_core: Task Executor (aw::tpl lifecycle management) ──────────────
aw_add_library(TARGET spoofiq_core_task_executor TYPE OBJECT
    SOURCES
        src/proc/task_executor.cpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS spoofiq_core_types spoofiq_core_logging aw::tpl)

# ── Modular spoofiq_core: Config Manager (Alpha 2) ───────────────────────────
aw_add_library(TARGET spoofiq_core_config TYPE OBJECT
    SOURCES
        src/config/config_manager.cpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS spoofiq_core_types nlohmann_json::nlohmann_json)

# ── Modular spoofiq_core: CLI Parser and Runner ──────────────────────────────
aw_add_library(TARGET spoofiq_core_cli TYPE OBJECT
    SOURCES
        src/cli/cli_parser.cpp
        src/cli/cli_runner.cpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS spoofiq_core_types spoofiq_core_config spoofiq_core_batch_orchestrator spoofiq_core_logging nlohmann_json::nlohmann_json)

# Pass version from VERSION file as compile definition
target_compile_definitions(spoofiq_core_cli PRIVATE
    APP_VERSION="${PROJECT_VERSION}")

# ── Modular spoofiq_core: Logging (Beta 1) ───────────────────────────────────
aw_add_library(TARGET spoofiq_core_logging TYPE OBJECT
    SOURCES
        src/logging/logger.cpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS spoofiq_core_types spdlog::spdlog imgui)

# ── Modular spoofiq_core: Event Bus Manager (Beta 3) ──────────────────────────
aw_add_library(TARGET spoofiq_core_event_bus TYPE OBJECT
    SOURCES
        src/event_bus/event_bus_manager.cpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS spoofiq_core_types aw::tobus)

# ── Modular spoofiq_core: Canvas (Overlay drawing state) ────────────────────────
aw_add_library(TARGET spoofiq_core_state TYPE OBJECT
    SOURCES
        src/core/overlay_canvas.cpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS spoofiq_core_types)

# ── Aggregate: spoofiq_core (static lib) ───────────────────────────────────────
aw_add_library(TARGET spoofiq_core TYPE STATIC ALIAS aw::spoofiq::core
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS
        spoofiq_core_types
        spoofiq_core_io_metadata
        spoofiq_core_io_image
        spoofiq_core_proc_image
        spoofiq_core_proc_video
        spoofiq_core_media_file
        spoofiq_core_batch_orchestrator
        spoofiq_core_async_batch_processor
        spoofiq_core_task_executor
        spoofiq_core_config
        spoofiq_core_cli
        spoofiq_core_logging
        spoofiq_core_event_bus
        spoofiq_core_state
        aw::tpl aw::tobus Threads::Threads stb)

if(SPOOFIQ_WARNINGS_AS_ERRORS)
    aw_configure_compiler(TARGET spoofiq_core STRICT_WARNINGS)
endif()

# ── Target 2: spoofiq_ui (static lib, ImGui + spoofiq_core) ──────────────────────
aw_add_library(TARGET spoofiq_ui TYPE STATIC ALIAS aw::spoofiq::ui
    SOURCES
        src/ui/ui.cpp
        src/ui/error_dialog.cpp
        src/ui/panels/batch_spoofer_panel.cpp
        src/ui/panels/presets_panel.cpp
        src/ui/panels/console_panel.cpp
        src/ui/panels/appearance_panel.cpp
    HEADERS
        include/aw/spoofiq/ui/ui.hpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS aw::spoofiq::core imgui)

# Pass version from VERSION file as compile definition
target_compile_definitions(spoofiq_ui PRIVATE
    APP_VERSION="${PROJECT_VERSION}")

if(SPOOFIQ_WARNINGS_AS_ERRORS)
    aw_configure_compiler(TARGET spoofiq_ui STRICT_WARNINGS)
endif()

# ── Target 3: spoofiq_app (executable, GLFW + OpenGL host) ──────────────────────
if(SPOOFIQ_BUILD_APP)
    # Windows version resource and icon (Windows only)
    if(WIN32)
        include(Windows/WinVersion)
        aw_win_version(
            OUTVAR SPOOFIQ_WIN_RESOURCES
            NAME "Spoofiq"
            VERSION_MAJOR ${PROJECT_VERSION_MAJOR}
            VERSION_MINOR ${PROJECT_VERSION_MINOR}
            VERSION_PATCH ${PROJECT_VERSION_PATCH}
            COMPANY_NAME "AI-Warevo"
            COMPANY_COPYRIGHT "Copyright 2024 AI-Warevo"
            FILE_DESCRIPTION "Media metadata spoofing tool"
            COMMENTS "Spoofiq v${PROJECT_VERSION}"
            ORIGINAL_FILENAME "spoofiq.exe"
            INTERNAL_NAME "spoofiq"
            ICON "${CMAKE_SOURCE_DIR}/product.ico"
        )
    endif()

    aw_add_executable(TARGET spoofiq_app
        SOURCES src/main.cpp ${SPOOFIQ_WIN_RESOURCES}
        LINK_LIBRARIES aw::spoofiq::ui glfw OpenGL::GL)

    # Pass version from VERSION file as compile definition
    target_compile_definitions(spoofiq_app PRIVATE
        APP_VERSION="${PROJECT_VERSION}")

    if(WIN32)
        if(MSVC)
            target_link_options(spoofiq_app PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
            # Link dwmapi for DWM title bar theming
            target_link_libraries(spoofiq_app PRIVATE dwmapi)
        elseif(MINGW)
            target_link_options(spoofiq_app PRIVATE -mwindows)
            target_link_libraries(spoofiq_app PRIVATE dwmapi)
        endif()
    endif()

    # Windows code signing (Windows only)
    # Certificate setup: see template_cpp/tools/create_cert.ps1
    if(WIN32)
        include(Windows/CodeSign)
        aw_code_sign_target(
            TARGET spoofiq_app
            CERT_NAME "AI-Warevo Code Signing"
        )
    endif()

    if(SPOOFIQ_WARNINGS_AS_ERRORS)
        aw_configure_compiler(TARGET spoofiq_app STRICT_WARNINGS)
    endif()
endif()

# ── Target 4: spoofiq_overlay (shared lib, embeddable) ──────────────────────────
if(SPOOFIQ_BUILD_OVERLAY)
    set(_overlay_sources src/overlay/overlay_common.cpp)
    if(WIN32)
        list(APPEND _overlay_sources src/overlay/overlay_win32.cpp)
    else()
        list(APPEND _overlay_sources src/overlay/overlay_posix.cpp)
    endif()

    aw_add_library(TARGET spoofiq_overlay TYPE SHARED ALIAS aw::spoofiq::overlay
        SOURCES ${_overlay_sources}
        PUBLIC_INCLUDE_DIRS include
        PUBLIC_LINKS aw::spoofiq::ui)

    set_target_properties(spoofiq_overlay PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR})

    if(SPOOFIQ_WARNINGS_AS_ERRORS)
        aw_configure_compiler(TARGET spoofiq_overlay STRICT_WARNINGS)
    endif()
endif()

# ── Tests (Catch2 v3) ──────────────────────────────────────────────────────────
aw_setup_tests(OPTION SPOOFIQ_BUILD_TESTS DIRECTORY tests)
