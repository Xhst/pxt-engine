#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
// Windows (32-bit and 64-bit, this part is common)
	#define PXT_PLATFORM_WINDOWS
	#ifdef _WIN64
		// Windows (64-bit)
		#define PXT_PLATFORM_WINDOWS_64
	#else
		// Windows (32-bit only)
		#define PXT_PLATFORM_WINDOWS_32
	#endif
#elif __APPLE__
	#include <TargetConditionals.h>
	#define PXT_PLATFORM_APPLE
	#if TARGET_IPHONE_SIMULATOR
	// iOS, tvOS, or watchOS Simulator
		#define PXT_PLATFORM_IOS_SIMULATOR
	#elif TARGET_OS_MACCATALYST
	// Mac's Catalyst (ports iOS API into Mac, like UIKit).
		#define PXT_PLATFORM_MACCATALYST
	#elif TARGET_OS_IPHONE
	// iOS, tvOS, or watchOS device
		#define PXT_PLATFORM_IOS
	#elif TARGET_OS_MAC
	// Other kinds of Apple platforms
		#define PXT_PLATFORM_MAC
	#else
	#   error "Unknown Apple platform"
	#endif
#elif __ANDROID__
	// Below __linux__ check should be enough to handle Android,
	// but something may be unique to Android.
	#define PXT_PLATFORM_ANDROID
#elif __linux__
	// linux
	#define PXT_PLATFORM_LINUX
#elif __unix__
	// Unix
	#define PXT_PLATFORM_UNIX
#elif defined(_POSIX_VERSION)
	// POSIX
	#define PXT_PLATFORM_POSIX
#else
#   error "Unknown compiler"
#endif


#if defined(PXT_PLATFORM_APPLE) || defined(PXT_PLATFORM_ANDROID) || defined(PXT_PLATFORM_LINUX) || \
	defined(PXT_PLATFORM_UNIX)  || defined(PXT_PLATFORM_POSIX)
#define PXT_PLATFORM_POSIX_LIKE
#endif
