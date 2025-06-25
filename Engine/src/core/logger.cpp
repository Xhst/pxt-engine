#include "core/logger.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <string>

namespace PXTEngine {

	Shared<spdlog::logger> Logger::s_logger;

	void Logger::init() {

		const std::string logPath = "logs/PXT.log";
		const std::string loggerName = "PXT";
		const std::string pattern = "%^[%H:%M:%S] [%!] [%l]%$ %v";

		// %l for the log level (trace, debug, info, warn, error, critical)
		// %n for the logger name
		// %v for the log message
		// %t for thread ID
		// %s for the source file -> file.cpp
		// %@ for source file and line number -> file.cpp:123

		s_logger = createShared<spdlog::logger>(loggerName, spdlog::sinks_init_list{
			createShared<spdlog::sinks::stdout_color_sink_mt>(),
			createShared<spdlog::sinks::basic_file_sink_mt>(logPath, true)
			});
		s_logger->set_pattern(pattern);

		s_logger->set_level(spdlog::level::trace);
	}
}