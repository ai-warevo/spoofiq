cmake_minimum_required(VERSION 3.24)
project(spoofiq_mvp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# MVP: minimal dependencies, stb only
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor)

# Image processor library
add_library(mvp_image_processor STATIC
    src/image_processor.cpp
)
target_include_directories(mvp_image_processor PUBLIC include)

# CLI executable
add_executable(spoofiq-mvp
    src/main.cpp
)
target_link_libraries(spoofiq-mvp PRIVATE mvp_image_processor)

# Optional: Test executable with simple image
enable_testing()

# Check if test image exists, if not create a minimal one
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_image.ppm")
    message(STATUS "Creating minimal test image (256x256 red)...")
    file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/test_image.ppm"
        "P6 256 256 255\n"
    )
    # Append 256*256*3 bytes of red pixel data
    file(APPEND "${CMAKE_CURRENT_SOURCE_DIR}/test_image.ppm"
        # This is a workaround: we'll create the binary in a different way
    )
endif()

# Add a simple test
add_test(
    NAME mvp_basic_test
    COMMAND spoofiq-mvp "${CMAKE_CURRENT_SOURCE_DIR}/test_image.jpg" "${CMAKE_CURRENT_BINARY_DIR}/test_output"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
