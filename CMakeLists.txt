cmake_minimum_required(VERSION 3.21)
project(MarioRealistic LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(mario_game
    src/main.cpp
    src/Game.cpp
    src/entities/Player.cpp
    src/entities/Enemy.cpp
    src/entities/ParticleSystem.cpp
    src/render/CameraController.cpp
    src/render/ParallaxBackground.cpp
    src/render/LightingPass.cpp
    src/world/TileMap.cpp
)

target_include_directories(mario_game PRIVATE src)

if (MSVC)
    target_compile_options(mario_game PRIVATE /W4 /permissive-)
else ()
    target_compile_options(mario_game PRIVATE -Wall -Wextra -Wpedantic)
endif ()

target_link_libraries(mario_game PRIVATE sfml-graphics sfml-window sfml-system)

add_custom_command(TARGET mario_game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:mario_game>/assets
)
