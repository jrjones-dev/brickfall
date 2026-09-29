cmake_minimum_required(VERSION 3.16)
project(Brickfall LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# raylib: downloaded automatically the first time CMake configures the project,
# so there's nothing to install by hand.
# ---------------------------------------------------------------------------
include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)   # don't build raylib's examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)         # keeps newer CMake (4.x) happy with raylib's scripts
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(raylib)

# ---------------------------------------------------------------------------
# The game itself
# ---------------------------------------------------------------------------
add_executable(brickfall
    src/main.cpp
    src/Game.cpp
    src/Paddle.cpp
    src/Ball.cpp
)
target_include_directories(brickfall PRIVATE src)
target_link_libraries(brickfall PRIVATE raylib)

if (APPLE)
    target_link_libraries(brickfall PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
endif()
