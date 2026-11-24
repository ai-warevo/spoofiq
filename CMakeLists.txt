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

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
aw_check_cxx23_runtime()

# ── Core Dependencies ──────────────────────────────────────────────────────────
# aw::tpl (TPL: Task Parallel Library)
if(NOT TARGET aw::tpl)
    FetchContent_Declare(
        aw_tpl
        GIT_REPOSITORY https://github.com/ai-warevo/tpl.git
        GIT_TAG main
        GIT_SHALLOW ON)
    set(AW_TPL_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(AW_TPL_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(AW_TPL_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(aw_tpl)
endif()

# aw::tobus (Event Bus)
if(NOT TARGET aw::tobus)
    FetchContent_Declare(
        aw_tobus
        GIT_REPOSITORY https://github.com/ai-warevo/tobus.git
        GIT_TAG main
        GIT_SHALLOW ON)
    set(TOBUS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(TOBUS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(TOBUS_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(aw_tobus)
endif()

# GLFW (windowing + input)
if(NOT TARGET glfw)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW ON)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
endif()

# Dear ImGui (docking branch)
if(NOT TARGET imgui)
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.91.8-docking
        GIT_SHALLOW ON)
    FetchContent_MakeAvailable(imgui)

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
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW ON)
    set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
    set(JSON_Install OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# spdlog (v1.14.1) - Used in Beta 1
if(NOT TARGET spdlog)
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.14.1
        GIT_SHALLOW ON)
    set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(spdlog)
endif()

# Exiv2 (v0.28.3) - Used in Alpha 3
# DISABLED FOR NOW - has many dependencies (inih, brotli, etc.)
# Will be properly integrated in Alpha 3
#if(NOT TARGET exiv2lib)
#    FetchContent_Declare(
#        exiv2
#        GIT_REPOSITORY https://github.com/Exiv2/exiv2.git
#        GIT_TAG v0.28.3
#        GIT_SHALLOW ON)
#    set(EXIV2_BUILD_EXIV2_COMMAND OFF CACHE BOOL "" FORCE)
#    set(EXIV2_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
#    set(EXIV2_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
#    set(EXIV2_ENABLE_NLS OFF CACHE BOOL "" FORCE)
#    FetchContent_MakeAvailable(exiv2)
#endif()

# TagLib (v2.0.2) - Used in Alpha 3
# DISABLED FOR NOW - has linking issues in this environment
# Will be properly integrated in Alpha 3
#if(NOT TARGET tag)
#    FetchContent_Declare(
#        taglib
#        GIT_REPOSITORY https://github.com/taglib/taglib.git
#        GIT_TAG v2.0.2
#        GIT_SHALLOW ON)
#    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
#    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
#    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
#    FetchContent_MakeAvailable(taglib)
#endif()

# nativefiledialog-extended (v1.2.1) - Beta 3 (requires GTK3)
# DISABLED - GTK3 not available in environment
#if(NOT TARGET nfd)
#    FetchContent_Declare(
#        nativefiledialog
#        GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
#        GIT_TAG v1.2.1
#        GIT_SHALLOW ON)
#    FetchContent_MakeAvailable(nativefiledialog)
#endif()

# ── Catch2 (v3) - Testing framework ────────────────────────────────────────────
if(SPOOFIQ_BUILD_TESTS AND NOT TARGET Catch2::Catch2WithMain)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.6.0
        GIT_SHALLOW ON)
    set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
    set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(Catch2)
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
    PUBLIC_INCLUDE_DIRS include ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    PUBLIC_LINKS spoofiq_core_types)

# ── Modular spoofiq_core: I/O Image Layer ───────────────────────────────────────
aw_add_library(TARGET spoofiq_core_io_image TYPE OBJECT
    SOURCES
        src/io/image_io.cpp
    PUBLIC_INCLUDE_DIRS include ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    PUBLIC_LINKS spoofiq_core_types spoofiq_core_io_metadata)

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

# ── Modular spoofiq_core: Config Manager (Alpha 2) ───────────────────────────
aw_add_library(TARGET spoofiq_core_config TYPE OBJECT
    SOURCES
        src/config/config_manager.cpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS spoofiq_core_types nlohmann_json::nlohmann_json)

# ── Modular spoofiq_core: Logging (Beta 1) ───────────────────────────────────
aw_add_library(TARGET spoofiq_core_logging TYPE OBJECT
    SOURCES
        src/logging/logger.cpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS spoofiq_core_types spdlog::spdlog imgui)

# ── Modular spoofiq_core: Event Bus Manager (Beta 3) ──────────────────────────
aw_add_library(TARGET spoofiq_core_event_bus TYPE OBJECT
    SOURCES
        src/event_bus_manager.cpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS spoofiq_core_types aw::tobus)

# ── Legacy spoofiq_core modules (to migrate) ──────────────────────────────────
aw_add_library(TARGET spoofiq_core_legacy TYPE OBJECT
    SOURCES
        src/config.cpp
        src/core/overlay_canvas.cpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS spoofiq_core_types)

# ── Aggregate: spoofiq_core (static lib) ───────────────────────────────────────
aw_add_library(TARGET spoofiq_core TYPE STATIC ALIAS aw::spoofiq::core
    PUBLIC_INCLUDE_DIRS include ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    PUBLIC_LINKS
        spoofiq_core_types
        spoofiq_core_io_metadata
        spoofiq_core_io_image
        spoofiq_core_proc_image
        spoofiq_core_proc_video
        spoofiq_core_config
        spoofiq_core_logging
        spoofiq_core_event_bus
        spoofiq_core_legacy
        aw::tpl aw::tobus Threads::Threads)

if(SPOOFIQ_WARNINGS_AS_ERRORS)
    aw_configure_compiler(TARGET spoofiq_core STRICT_WARNINGS)
endif()

# ── Target 2: spoofiq_ui (static lib, ImGui + spoofiq_core) ──────────────────────
aw_add_library(TARGET spoofiq_ui TYPE STATIC ALIAS aw::spoofiq::ui
    SOURCES
        src/ui.cpp
        src/ui/error_dialog.cpp
        src/ui/panels/batch_spoofer_panel.cpp
        src/ui/panels/presets_panel.cpp
        src/ui/panels/console_panel.cpp
        src/ui/panels/appearance_panel.cpp
    HEADERS
        include/aw/spoofiq/ui.hpp
        include/aw/spoofiq/spoofiq.hpp
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
    aw_add_executable(TARGET spoofiq_app
        SOURCES src/main.cpp
        LINK_LIBRARIES aw::spoofiq::ui glfw OpenGL::GL)

    # Pass version from VERSION file as compile definition
    target_compile_definitions(spoofiq_app PRIVATE
        APP_VERSION="${PROJECT_VERSION}")

    if(WIN32)
        if(MSVC)
            target_link_options(spoofiq_app PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
        elseif(MINGW)
            target_link_options(spoofiq_app PRIVATE -mwindows)
        endif()
    endif()

    if(SPOOFIQ_WARNINGS_AS_ERRORS)
        aw_configure_compiler(TARGET spoofiq_app STRICT_WARNINGS)
    endif()
endif()

# ── Target 4: spoofiq_overlay (shared lib, embeddable) ──────────────────────────
if(SPOOFIQ_BUILD_OVERLAY)
    set(_overlay_sources src/overlay_common.cpp)
    if(WIN32)
        list(APPEND _overlay_sources src/overlay_win32.cpp)
    else()
        list(APPEND _overlay_sources src/overlay_posix.cpp)
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
