#include "Pengu_Engine/Misc/Logger.hpp"
#include <vector>

std::shared_ptr<spdlog::logger> Logger::s_PenguEngine;
std::shared_ptr<spdlog::logger> Logger::s_Logger;
std::shared_ptr<spdlog::logger> Logger::s_Scripting;

Logger::Logger()
{

}

Logger::~Logger()
{

}

std::shared_ptr<spdlog::logger>& Logger::PE()
{
	return s_PenguEngine;
}

std::shared_ptr<spdlog::logger>& Logger::LOG()
{
	return s_Logger;
}

std::shared_ptr<spdlog::logger>& Logger::SCRIPT()
{
	return s_Scripting;
}

void Logger::Init()
{
	auto sinks = std::vector<spdlog::sink_ptr>{
		std::make_shared<spdlog::sinks::stderr_color_sink_mt>(),
		std::make_shared<spdlog::sinks::rotating_file_sink_mt>("../logs/engine.log",10 * 1024 * 1024, 5)
	};

	auto scrp_sink = std::vector<spdlog::sink_ptr>{
		std::make_shared<spdlog::sinks::stderr_color_sink_mt>(),
		std::make_shared<spdlog::sinks::rotating_file_sink_mt>("../logs/scripting.log",10 * 1024 * 1024, 5)
	};

	s_PenguEngine = std::make_shared<spdlog::logger>("ENGINE", sinks.begin(), sinks.end());
	s_Logger = std::make_shared<spdlog::logger>("LOGGER", sinks.begin(), sinks.end());
	s_Scripting = std::make_shared<spdlog::logger>("SCRIPTING", scrp_sink.begin(), scrp_sink.end());

	s_PenguEngine->set_level(spdlog::level::trace);
	s_Logger->set_level(spdlog::level::trace);
	s_Scripting->set_level(spdlog::level::trace);
}


