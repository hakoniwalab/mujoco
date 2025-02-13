cmake_minimum_required(VERSION 3.20)

add_executable(
    drone 
    main.cpp
    ${CMAKE_SOURCE_DIR}/examples/common/mujoco_debug.cpp
    ${CMAKE_SOURCE_DIR}/examples/common/mujoco_viewer.cpp
)

#MESSAGE(STATUS "CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR})

target_include_directories(drone 
    PRIVATE ${CMAKE_SOURCE_DIR}/examples/common)

# MuJoCoライブラリをリンク
target_link_libraries(drone 
    ${LIBMUJOCO}
   glfw
    ${OPENGL_gl_LIBRARY}
    "-framework OpenGL"
)