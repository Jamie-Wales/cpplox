cmake_minimum_required(VERSION 3.10)
project(vm)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/frontend
    ${PROJECT_SOURCE_DIR}/include/backend
    ${PROJECT_SOURCE_DIR}/include/helper
    ${PROJECT_SOURCE_DIR}/include/vm
)

# Find all source files
file(GLOB_RECURSE SOURCES 
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# Add executable
add_executable(vm ${SOURCES})

# If you have any external libraries, add them here
# For example:
# find_package(SomeLibrary REQUIRED)
# target_link_libraries(vm PRIVATE SomeLibrary)

# Add compile options if needed
# For example, to enable all warnings:
# target_compile_options(vm PRIVATE -Wall -Wextra)

# Print include directories (for debugging)
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "Include dir='${dir}'")
endforeach()
