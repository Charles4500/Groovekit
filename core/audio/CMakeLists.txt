add_library(dj_core_audio
    AudioDeviceManager.cpp
    WavFile.cpp
    Mp3File.cpp
    Deck.cpp
    AudioEngine.cpp
)

target_include_directories(dj_core_audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dj_core_audio PUBLIC rtaudio PRIVATE dr_mp3)
