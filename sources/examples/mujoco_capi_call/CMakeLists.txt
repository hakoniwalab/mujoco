cmake_minimum_required(VERSION 3.20)

add_executable(
    main 
    main.cpp
    ${CMAKE_SOURCE_DIR}/examples/common/mujoco_debug.cpp
)

#MESSAGE(STATUS "CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR})

target_include_directories(main 
    PRIVATE ${CMAKE_SOURCE_DIR}/examples/common)

# MuJoCoライブラリをリンク
target_link_libraries(main 
    ${LIBMUJOCO})

