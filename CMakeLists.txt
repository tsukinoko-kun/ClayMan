cmake_minimum_required(VERSION 3.15)
# Use ClayMan as the project name for the library itself
project(ClayMan)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

# Define the ClayMan library target.
# Assuming clayman.cpp is in the root of the vendored/ClayMan directory.
# We create a STATIC library as is common for vendored dependencies.
add_library(ClayMan STATIC clayman.cpp)

# Link necessary dependencies for the ClayMan library.
# The user's project finds SDL3 libs *before* adding ClayMan as a subdirectory,
# so these targets should be available here.
target_link_libraries(ClayMan PRIVATE
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    SDL3_image::SDL3_image
)

# Optionally set include directories if clayman.cpp needs headers.
# Assuming ClayMan's public headers are also in the root or an include subdir.
# If clayman.hpp exists in the same dir as clayman.cpp:
target_include_directories(ClayMan PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# Use PUBLIC if headers need to be visible to targets linking ClayMan. [3][4][6][7][12]

# Define an alias target for easy linking in the parent project.
# This matches the target name already used in your main CMakeLists.txt. [8][9][15]
add_library(ClayMan::ClayMan ALIAS ClayMan)

