#pragma once

#include "core/pch.hpp"

namespace PXTEngine {

	class ProfilingTimer {
	public:
		explicit ProfilingTimer(std::string name) : m_name(std::move(name)) {
			m_startTime = std::chrono::high_resolution_clock::now();
		}

		~ProfilingTimer() {
			const auto endTime = std::chrono::high_resolution_clock::now();
			const auto elapsedTimeMs = std::chrono::duration_cast<std::chrono::nanoseconds>(
				endTime - m_startTime).count() * 0.001f * 0.001f;

			std::cout << "[Timer@" << m_name << "] - " << elapsedTimeMs << "ms\n";
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
		std::string m_name;
	};
}