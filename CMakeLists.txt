project(v4l2framesink)
cmake_minimum_required(VERSION 3.10)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

configure_file(MANIFEST.in.in MANIFEST.in @ONLY)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/py${PROJECT_NAME}/__init__.py "")

add_library(${PROJECT_NAME} SHARED ${PROJECT_NAME}.cpp)
set_target_properties(${PROJECT_NAME}
    PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/py${PROJECT_NAME}")

# Ensure ctypesgen is installed
add_custom_target(build_deps
    COMMAND ${Python3_EXECUTABLE} -m pip install -r ${CMAKE_CURRENT_SOURCE_DIR}/build-requirements.txt)
add_dependencies(${PROJECT_NAME} build_deps)

configure_file(generate-bindings.sh.in generate-bindings.sh @ONLY)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ./generate-bindings.sh)