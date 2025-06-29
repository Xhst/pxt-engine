#pragma once


// Standard Library Headers - Core Functionality
#include <iostream>      // For input/output operations (e.g., std::cout, std::cin)
#include <string>        // For std::string class, representing character sequences
#include <memory>        // For smart pointers (e.g., std::unique_ptr, std::shared_ptr) for memory management
#include <utility>       // For std::pair, std::move, std::forward, and other utility functions
#include <algorithm>     // For various algorithms (e.g., sort, find, min, max)
#include <functional>    // For std::function, std::bind, and other function-related utilities
#include <chrono>        // For time-related utilities (durations, time points, clocks)
#include <limits>        // For numeric_limits, providing properties of fundamental types
#include <random>		 // For random number generation
#include <fstream>		 // For file input/output operations

// Standard Library Headers - Data Structures and Containers
#include <vector>           // For std::vector, a dynamic array
#include <array>            // For std::array, a fixed-size array
#include <unordered_map>    // For std::unordered_map, a hash table-based associative container
#include <unordered_set>    // For std::unordered_set, a hash table-based set
#include <set>              // For std::set, a sorted associative container (balanced binary search tree)
#include <map>              // For std::map, a sorted associative container of key-value pairs (balanced binary search tree)

// Standard Library Headers - Low-level Utilities and C-style Compatibility
// These headers offer lower-level functionalities, often inherited from C.
#include <cstdint>       // For fixed-width integer types (e.g., int32_t, uint64_t)
#include <cstring>       // For C-style string manipulation functions (e.g., strcpy, memset)
#include <cassert>       // For assert macro, used for debugging to check conditions

// Standard Library Headers - Error Handling
// These headers provide mechanisms for handling errors and exceptions.
#include <stdexcept>     // For standard exception classes (e.g., std::runtime_error, std::invalid_argument)

// Standard Library Headers - File System Operations
// This header provides functionalities for interacting with the file system.
#include <filesystem>    // For std::filesystem::path, std::filesystem::directory_iterator, etc.

// Standard Library Headers - String Manipulation and Formatting
#include <sstream>       // For std::stringstream, allowing string-based I/O
#include <format>        // For std::format, a modern type-safe string formatting library (C++20)
#include <ranges>        // For std::ranges, providing views and algorithms for ranges (C++20)
#include <span>			 // For std::span, to represent a contiguous sequence of objects (C++20)


// --- External Libraries Headers --- //

// GLM (OpenGL Mathematics) Headers - C++ mathematics library for graphics software
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

// Vulkan Headers - For using the Vulkan API, a low-level graphics API
#include <vulkan/vulkan.h>

// GLFW (Graphics Library Framework) Headers - For creating windows and managing input
// Also includes Vulkan support.
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Entt (Entity-Component-System) Headers - Lightweight and fast entity-component-system library
#include <entt/entt.hpp>

// ImGui Headers - For creating graphical user interfaces
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imconfig.h"
#include "imgui_internal.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

// --- Engine-Specific Headers --- //

#include "core/memory.hpp"
#include "core/logger.hpp"
#include "core/diagnostics.hpp"
#include "core/constants.hpp"