add_executable(unit_test
        ../lsan_suppressions.cpp
        test_ffmpeg_video_decoder.cpp
        )
target_link_libraries(unit_test PRIVATE ${CONAN_LIBS} gmock_main ffmpeg_utils)
