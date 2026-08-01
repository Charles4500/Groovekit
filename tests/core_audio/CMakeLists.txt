add_executable(deck_test deck_test.cpp)
target_link_libraries(deck_test PRIVATE dj_core_audio)
add_test(NAME deck_test COMMAND deck_test)

add_executable(mp3file_test mp3file_test.cpp)
target_link_libraries(mp3file_test PRIVATE dj_core_audio)
add_test(NAME mp3file_test
    COMMAND mp3file_test "${CMAKE_CURRENT_SOURCE_DIR}/../fixtures/tone_440hz.mp3")
