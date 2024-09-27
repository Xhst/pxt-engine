# Minimum version of CMake required
cmake_minimum_required(VERSION 3.10)

# Project name and the supported language
project(CG_ENGINE CXX)

# Set the standard for the C++ language
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable OpenMP
find_package(OpenMP REQUIRED)

# Directories for source, include, object, and binary files
set(INC_DIR "${CMAKE_SOURCE_DIR}/include")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(OBJ_DIR "${CMAKE_BINARY_DIR}/obj")
set(BIN_DIR "${CMAKE_BINARY_DIR}/bin")

# Create binary and object directories
file(MAKE_DIRECTORY ${BIN_DIR})
file(MAKE_DIRECTORY ${OBJ_DIR})

# Add the include directory
include_directories(${INC_DIR})

# Find Vulkan
find_package(Vulkan REQUIRED)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3 -march=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lm ${OpenMP_CXX_FLAGS}")

# Collect all source files in the src directory
file(GLOB_RECURSE SRCS "${SRC_DIR}/*.cpp")

# Create an executable
add_executable(engine ${SRCS})

# Set the output directory for the final binary
set_target_properties(engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
)

# Link the necessary libraries (libpng, OpenMP, Vulkan, etc.)
target_link_libraries(engine m ${OpenMP_CXX_FLAGS} Vulkan::Vulkan)

# Custom clean command
add_custom_target(clean 
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${OBJ_DIR}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${BIN_DIR}
)
