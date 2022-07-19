add_executable(create_window create_window.cpp)
target_link_libraries(create_window PRIVATE ${CONAN_LIBS})

add_executable(moving_rectangle moving_rectangle.cpp)
target_link_libraries(moving_rectangle PRIVATE ${CONAN_LIBS})

add_executable(render_preset render_preset.cpp)
target_link_libraries(render_preset PRIVATE ${CONAN_LIBS})