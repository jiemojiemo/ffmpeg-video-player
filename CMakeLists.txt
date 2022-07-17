cmake_minimum_required(VERSION 3.10)

project(ffmpeg_and_sdl_tutorial)

set(CMAKE_CXX_STANDARD 17)

option(FF_BUILD_TESTING "build tests" ON)

if (ENABLE_ASAN)
    message(STATUS "build with ASAN")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif ()

add_subdirectory(3rdparty)
add_subdirectory(src)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(tutorial01 tutorial01.cpp lsan_suppressions.cpp)
target_link_libraries(tutorial01 PRIVATE ${CONAN_LIBS})

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND FF_BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif ()
