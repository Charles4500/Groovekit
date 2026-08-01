add_executable(deck_test deck_test.cpp)
target_link_libraries(deck_test PRIVATE dj_core_audio)
add_test(NAME deck_test COMMAND deck_test)
