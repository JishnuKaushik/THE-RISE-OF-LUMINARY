cmake_minimum_required(VERSION 3.10)
project(LuminaryGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System Audio)

add_executable(LuminaryGame
    src/main.cpp
    src/Game.cpp
    src/Button.cpp
    src/MenuState.cpp
    src/RegistrationState.cpp
    src/TrainingState.cpp
    src/TrainingStoryState.cpp
    src/SaveManager.cpp
    src/ProfileSelectionState.cpp
    src/Character.cpp
    src/CharacterSelectionState.cpp
    src/BattleSystem.cpp
    src/ScreenShake.cpp
    src/ParticleSystem.cpp
    src/Card.cpp
    src/Enemy.cpp
    src/ElementSystem.cpp
    src/SplashState.cpp
    src/TrainingHubState.cpp
    src/LessonViewerState.cpp
)

# Link stdc++fs explicitly for older MinGW toolchains (no-op on MSVC/GCC 9+)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(LuminaryGame stdc++fs)
endif()

target_link_libraries(LuminaryGame SFML::Graphics SFML::Window SFML::System SFML::Audio)

# Copy assets and data to build directory after build
add_custom_command(TARGET LuminaryGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:LuminaryGame>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:LuminaryGame>/data
    COMMENT "Copying assets and data to build directory..."
)
