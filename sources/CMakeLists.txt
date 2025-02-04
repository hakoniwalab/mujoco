cmake_minimum_required(VERSION 3.10)

project(MuJoCoSample)
set(CMAKE_CXX_STANDARD 17)


set(HAKO_INC_DIR "/usr/local/include/hakoniwa")
set(LIBMUJOCO "mujoco.3.2.7")

# MuJoCoのヘッダーファイルとライブラリのパスを指定
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    /opt/homebrew/Cellar/glfw/3.4/include/
    )

link_directories(${CMAKE_SOURCE_DIR}/lib)
link_directories(/usr/local/lib/hakoniwa)

MESSAGE(STATUS "CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR})

find_package(OpenGL REQUIRED) # OpenGL を探す
find_package(glfw3 REQUIRED)   # GLFW を探す

add_subdirectory(examples/mujoco_capi_call)
