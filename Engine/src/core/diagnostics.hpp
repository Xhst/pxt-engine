#pragma once

#include "core/platform.hpp"
#include "utils/timer.hpp"

#include <filesystem>
#include <iostream>
#include <format>
#include <cassert>

#define PXT_LOG(fmt, ...) std::cout << std::format(fmt, __VA_ARGS__) << "\n"

#define PXT_ERROR(fmt, ...) std::cerr << std::format(fmt, __VA_ARGS__) << "\n"

#if defined(PXT_PLATFORM_WINDOWS)
	// Use the Windows-specific intrinsic
	#define PXT_DEBUG_BREAK() __debugbreak()
#elif defined(PXT_PLATFORM_POSIX_LIKE)
	// Use the POSIX signal method for Apple, Android, Linux, Unix, and generic POSIX
	#include <signal.h>
	#define PXT_DEBUG_BREAK() raise(SIGTRAP)
#elif defined(__GNUC__) || defined(__clang__)
	// Use GCC/Clang built-in function
	#define PXT_DEBUG_BREAK() __builtin_trap()
#else
	#error "Unsupported platform for debug break"
#endif

// Internal assertion implementation
#define PXT_ASSERT_IMPL(condition, msg, ...) \
    do { \
        if (!(condition)) { \
            PXT_ERROR(msg, __VA_ARGS__); \
            PXT_DEBUG_BREAK(); \
        } \
    } while (0)

// Assertion with custom message
#define PXT_ASSERT_WITH_MSG(condition, ...) PXT_ASSERT_IMPL(condition, "Assertion failed: {}", __VA_ARGS__)

// Assertion with default message
#define PXT_ASSERT_NO_MSG(condition) PXT_ASSERT_IMPL(condition, "Assertion `{}` failed at {}:{}", #condition, std::filesystem::path(__FILE__).filename().string(), __LINE__)

// Macro chooser (1 arg = no msg, 2+ = with msg)
#define PXT_GET_ASSERT_MACRO(_1, _2, NAME, ...) NAME
#define PXT_ASSERT_CHOOSER(...) PXT_GET_ASSERT_MACRO(__VA_ARGS__, PXT_ASSERT_WITH_MSG, PXT_ASSERT_NO_MSG)

// Dispatcher
#define PXT_ASSERT(...) PXT_ASSERT_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define PXT_STATIC_ASSERT(condition, msg) static_assert(condition, msg)

#define PXT_ENABLE_PROFILING

// Profiling macros
#if defined(PXT_ENABLE_PROFILING)
#define PXT_PROFILE(name) ProfilingTimer timer##__LINE__(name)
#define PXT_PROFILE_FN() PXT_PROFILE(__FUNCTION__)
#else
#define PXT_PROFILE(name)
#define PXT_PROFILE_FN()
#endif