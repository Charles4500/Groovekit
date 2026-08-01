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

add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(app)

if(DJ_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
