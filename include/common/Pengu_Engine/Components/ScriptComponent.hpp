/**
 * @file ScriptComponent.hpp
 * @brief Component for attaching Lua scripts to entities.
 */

#ifndef SCRIPTCOMPONENT_HPP
#define SCRIPTCOMPONENT_HPP 1

#include <filesystem>
#include <string>

#include "sol/sol.hpp"

/**
 * @struct ScriptingComponent
 * @brief Component that holds Lua script state and functions.
 */
struct ScriptingComponent {
	/**
	 * @brief Default constructor.
	 */
	ScriptingComponent() : scriptPath_{}
	{
		env_ = sol::nil;
		instance = sol::nil;
	};
	/**
	 * @brief Constructor with script path.
	 * @param path Path to the Lua script.
	 */
	ScriptingComponent(std::string path) : scriptPath_{ path } {};

	/** @brief Path to the script file. */
	std::string scriptPath_;

	/** @brief Lua environment for the script. */
	sol::environment env_ = sol::nil;
	/** @brief Lua instance (table) for the script. */
	sol::table instance;

	/** @brief Lua function called on start. */
	sol::protected_function onStart;
	/** @brief Lua function called on update. */
	sol::protected_function onUpdate;
	/** @brief Lua function called on finish. */
	sol::protected_function onFinish;

	/** @brief Last write time of the script file for hot-reloading. */
	std::filesystem::file_time_type lastWriteTime;
};

#endif // !SCRIPTCOMPONENT_HPP
