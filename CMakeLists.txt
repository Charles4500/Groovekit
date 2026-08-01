cmake_minimum_required(VERSION 3.21)
project(dj_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(DJ_BUILD_TESTS "Build unit/integration tests" ON)

include(FetchContent)
FetchContent_Declare(
    rtaudio
    GIT_REPOSITORY https://github.com/thestk/rtaudio.git
    GIT_TAG 6.0.1
)
set(RTAUDIO_BUILD_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(rtaudio)

# dr_mp3 is a single header with no build system of its own (see ADR-002) —
# fetched by URL and pinned to a specific commit, not a floating branch.
FetchContent_Declare(
    dr_mp3
    URL https://raw.githubusercontent.com/mackron/dr_libs/6d78776c2c05358e351e3c67878a8b681c76c5d1/dr_mp3.h
    DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(dr_mp3)
add_library(dr_mp3 INTERFACE)
target_include_directories(dr_mp3 INTERFACE ${dr_mp3_SOURCE_DIR})

add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(app)

if(DJ_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
