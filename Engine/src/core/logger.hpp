#pragma once

#include "core/memory.hpp"

#include <spdlog/spdlog.h>

namespace PXTEngine {

	class Logger {
	public:
		static void init();

		inline static Shared<spdlog::logger>& get() { return s_logger; }

		inline static void shutdown() {
			if (s_logger) {
				s_logger->flush(); // Flush the logger before dropping it
				spdlog::drop_all(); // Drop all loggers
				s_logger.reset(); // Reset the logger pointer
			}
		}

		~Logger() {
			shutdown();
		}

	private:
		static Shared<spdlog::logger> s_logger;
	};
/*
#define PXT_TRACE(...) PXTEngine::Logger::get()->trace(__VA_ARGS__)
#define PXT_DEBUG(...) PXTEngine::Logger::get()->debug(__VA_ARGS__)
#define PXT_INFO(...)  PXTEngine::Logger::get()->info(__VA_ARGS__)
#define PXT_WARN(...)  PXTEngine::Logger::get()->warn(__VA_ARGS__)
#define PXT_ERROR(...) PXTEngine::Logger::get()->error(__VA_ARGS__)
#define PXT_FATAL(...) PXTEngine::Logger::get()->critical(__VA_ARGS__)
*/

#define PXT_TRACE(...) SPDLOG_LOGGER_TRACE(PXTEngine::Logger::get(), __VA_ARGS__)
#define PXT_DEBUG(...) SPDLOG_LOGGER_DEBUG(PXTEngine::Logger::get(), __VA_ARGS__)
#define PXT_INFO(...)  SPDLOG_LOGGER_INFO(PXTEngine::Logger::get(), __VA_ARGS__)
#define PXT_WARN(...)  SPDLOG_LOGGER_WARN(PXTEngine::Logger::get(), __VA_ARGS__)
#define PXT_ERROR(...) SPDLOG_LOGGER_ERROR(PXTEngine::Logger::get(), __VA_ARGS__)
#define PXT_FATAL(...) SPDLOG_LOGGER_CRITICAL(PXTEngine::Logger::get(), __VA_ARGS__)

}