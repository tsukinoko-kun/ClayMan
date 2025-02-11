cmake_minimum_required(VERSION 3.15)
project(ClayManager)

set(CMAKE_CXX_STANDARD 20)

set(EXAMPLES_DIR ${CMAKE_SOURCE_DIR}/examples)
set(EXAMPLES_OUTPUT_DIR ${CMAKE_BINARY_DIR}/examples)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g")

find_package(raylib REQUIRED)

#readme example
add_executable(readme-example ${EXAMPLES_DIR}/readme-example/readme-example.cpp clayman.cpp)
set_target_properties(readme-example PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXAMPLES_OUTPUT_DIR}/readme-example)
target_link_libraries(readme-example raylib)

add_custom_command(
    TARGET readme-example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${EXAMPLES_DIR}/readme-example/resources
    $<TARGET_FILE_DIR:readme-example>/resources
)

#full example
add_executable(full-example ${EXAMPLES_DIR}/full-example/full-example.cpp clayman.cpp)
set_target_properties(full-example PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXAMPLES_OUTPUT_DIR}/full-example)
target_link_libraries(full-example raylib)

add_custom_command(
    TARGET full-example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${EXAMPLES_DIR}/full-example/resources
    $<TARGET_FILE_DIR:full-example>/resources
)

#quick example
add_executable(quick-example ${EXAMPLES_DIR}/quick-example/quick-example.cpp clayman.cpp)
set_target_properties(quick-example PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXAMPLES_OUTPUT_DIR}/quick-example)
target_link_libraries(quick-example raylib)

add_custom_command(
    TARGET quick-example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${EXAMPLES_DIR}/quick-example/resources
    $<TARGET_FILE_DIR:quick-example>/resources
)

#advanced use example
add_executable(advanced-use-example ${EXAMPLES_DIR}/advanced-use-example/advanced-use-example.cpp clayman.cpp)
set_target_properties(advanced-use-example PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXAMPLES_OUTPUT_DIR}/advanced-use-example)
target_link_libraries(advanced-use-example raylib)

add_custom_command(
    TARGET advanced-use-example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${EXAMPLES_DIR}/advanced-use-example/resources
    $<TARGET_FILE_DIR:advanced-use-example>/resources
)
