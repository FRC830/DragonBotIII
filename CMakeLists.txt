cmake_minimum_required(VERSION 2.8)
include(lib830/cmake/frc.cmake)
set(CMAKE_BUILD_TYPE Debug)
set(FRC_PREFER_TARGET static)

project(DragonBotIII)
add_subdirectory(lib830)
include_directories(src lib830)

add_frc_robot(${PROJECT_NAME} 830
    src/robot.cpp
)
target_link_libraries(${PROJECT_NAME} 830)
