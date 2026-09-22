/**
 * @file Logger.hpp
 * @brief Logging system for the engine using spdlog.
 */

#ifndef LOGGER_HPP
#define LOGGER_HPP 1

#include <memory>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

/**
 * @class Logger
 * @brief Static class that manages different logging channels.
 * 
 * Provides centralized access to engine, general, and scripting loggers.
 */
class Logger {
public:
	/**
	 * @brief Default constructor.
	 */
	Logger();

	/**
	 * @brief Destructor.
	 */
	~Logger();

	/**
	 * @brief Initializes the logging system, setting up sinks and formatting.
	 */
	static void Init();

	/** @brief Gets the core engine logger. */
	static std::shared_ptr<spdlog::logger>& PE();
	/** @brief Gets the general application logger. */
	static std::shared_ptr<spdlog::logger>& LOG();
	/** @brief Gets the scripting system logger. */
	static std::shared_ptr<spdlog::logger>& SCRIPT();

private:

	static std::shared_ptr<spdlog::logger> s_PenguEngine; ///< Engine-internal logger instance.
	static std::shared_ptr<spdlog::logger> s_Logger;      ///< General-purpose logger instance.
	static std::shared_ptr<spdlog::logger> s_Scripting;   ///< Scripting-specific logger instance.

};

#endif // ! LOGGER_HPP
