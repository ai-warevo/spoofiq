cmake_minimum_required(VERSION 3.23)

include(FetchContent)
FetchContent_Declare(
    aw_cmake_modules
    GIT_REPOSITORY https://github.com/ai-warevo/template_cpp
    GIT_TAG main
    GIT_SHALLOW ON)
FetchContent_MakeAvailable(aw_cmake_modules)
list(APPEND CMAKE_MODULE_PATH "${aw_cmake_modules_SOURCE_DIR}/cmake")

include(AWProject)
aw_project_prepare(NAME spoofiq NAMESPACE aw::spoofiq)
project(${AW_PROJECT_NAME} VERSION ${AW_PROJECT_VERSION} LANGUAGES CXX)
aw_project_setup(PREFIX SPOOFIQ BUILD_TESTS BUILD_APP BUILD_OVERLAY)

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
aw_check_cxx23_runtime()

# ── Dependencies ──────────────────────────────────────────────────────────────
# aw::tpl (TPL: Task Parallel Library)
if(NOT TARGET aw::tpl)
    FetchContent_Declare(
        aw_tpl
        GIT_REPOSITORY https://github.com/ai-warevo/tpl.git
        GIT_TAG v3.1.0
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
    FetchContent_MakeAvailable(glfw)
endif()

# Dear ImGui (docking branch) - as static library with backends
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

# ── Target 1: spoofiq_core (static lib, pure C++23, zero imgui.h) ──
aw_add_library(TARGET spoofiq_core TYPE STATIC ALIAS aw::spoofiq::core
    SOURCES
        src/config.cpp
        src/panel_registry.cpp
        src/overlay_canvas.cpp
    HEADERS
        include/aw/spoofiq/config.hpp
        include/aw/spoofiq/types.hpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS aw::tpl aw::tobus Threads::Threads)

if(SPOOFIQ_WARNINGS_AS_ERRORS)
    aw_configure_compiler(TARGET spoofiq_core STRICT_WARNINGS)
endif()

# ── Target 2: spoofiq_ui (static lib, ImGui + spoofiq_core) ──
aw_add_library(TARGET spoofiq_ui TYPE STATIC ALIAS aw::spoofiq::ui
    SOURCES
        src/ui.cpp
        src/panels/general_panel.cpp
        src/panels/overlay_demo_panel.cpp
    HEADERS
        include/aw/spoofiq/ui.hpp
        include/aw/spoofiq/spoofiq.hpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS aw::spoofiq::core imgui)

if(SPOOFIQ_WARNINGS_AS_ERRORS)
    aw_configure_compiler(TARGET spoofiq_ui STRICT_WARNINGS)
endif()

# ── Target 3: spoofiq_app (executable, GLFW + OpenGL host) ──
if(SPOOFIQ_BUILD_APP)
    aw_add_executable(TARGET spoofiq_app
        SOURCES src/main.cpp
        LINK_LIBRARIES aw::spoofiq::ui glfw OpenGL::GL)

    # Hide console window on Windows
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

# ── Target 4: spoofiq_overlay (shared lib, embeddable) ──
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

# ── Tests (Catch2 v3) ──
aw_setup_tests(OPTION SPOOFIQ_BUILD_TESTS DIRECTORY tests)
