/**
 * @file Logmacros.hpp
 * @brief Convenience macros for logging at different severity levels.
 */

#pragma once

#include "Logger.hpp"

/** @name Engine Logging Macros */
/** @{ */
#define ENGINE_INFO(...)    ::Logger::PE()->info(__VA_ARGS__);
#define ENGINE_WARNING(...) ::Logger::PE()->warn(__VA_ARGS__);
#define ENGINE_ERROR(...)   ::Logger::PE()->error(__VA_ARGS__);
/** @} */

/** @name General Logging Macros */
/** @{ */
#define LOG_INFO(...)       ::Logger::LOG()->info(__VA_ARGS__); 
#define LOG_WARNING(...)    ::Logger::LOG()->warn(__VA_ARGS__); 
#define LOG_ERROR(...)      ::Logger::LOG()->error(__VA_ARGS__);
/** @} */

/** @name Scripting Logging Macros */
/** @{ */
#define SCRP_INFO(...)       ::Logger::SCRIPT()->info(__VA_ARGS__); 
#define SCRP_WARNING(...)    ::Logger::SCRIPT()->warn(__VA_ARGS__); 
#define SCRP_ERROR(...)      ::Logger::SCRIPT()->error(__VA_ARGS__);
/** @} */
