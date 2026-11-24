#pragma once

/*
 * @file gl_loader.hpp
 * Minimal OpenGL loader wrapper for Dear ImGui backends.
 * Ensures GL functions are properly declared before ImGui uses them.
 */

#ifdef _WIN32
    /*
     * On Windows, GLFW provides all necessary GL headers via its own includes.
     * We only need to ensure windows.h is included first to avoid macro conflicts.
     */
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#endif

/*
 * On Linux/Unix, include system GL headers
 */
#ifdef __linux__
    #define GL_GLEXT_PROTOTYPES
    #include <GL/gl.h>
    #include <GL/glext.h>
#endif

/*
 * On macOS, use system OpenGL framework
 */
#ifdef __APPLE__
    #include <OpenGL/gl.h>
    #include <OpenGL/glext.h>
#endif
