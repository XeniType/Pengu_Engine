/**
 * @file ScriptSystem.hpp
 * @brief System for managing and executing Lua scripts attached to entities.
 */

#ifndef SCRIPTSYSTEM_HPP
#define SCRIPTSYSTEM_HPP 1

#include "Pengu_Engine/Systems/BaseSystems.hpp"
#include "Pengu_Engine/Components/ScriptComponent.hpp"
#include "Pengu_Engine/Managers/ECS/ECSManager.hpp"
#include "Pengu_Engine/Inputs/Input.hpp"

#include <memory>
#include <unordered_map>

#include "sol/sol.hpp"

/**
 * @class Scripting
 * @brief ECS System that handles Lua script lifecycle and API binding.
 * 
 * This system is responsible for initializing the Lua environment, binding
 * C++ functions to Lua, and calling script hooks (Start, Update, Finish)
 * for entities with a ScriptingComponent.
 */
class Scripting : public System {

public:
	/**
	 * @brief Constructs the scripting system.
	 * @param ecs Reference to the ECS manager.
	 */
	Scripting(ECSManager& ecs);

	/**
	 * @brief Destructor that shuts down the Lua state.
	 */
	~Scripting();

	/**
	 * @brief Initializes the Lua state and binds engine APIs.
	 * @param inp Pointer to the input system for binding.
	 */
	void Init(Input* inp);

	/**
	 * @brief Updates all active entity scripts.
	 * @param dt Delta time since the last frame.
	 */
	void Update(double dt);

	/**
	 * @brief Calls the 'Start' function for all scripts that haven't run yet.
	 */
	void Start();

	/**
	 * @brief Calls the 'Finish' function for all scripts (usually during shutdown).
	 */
	void Finish();

protected:

	/** @brief Binds all engine APIs to the Lua state. */
	void BindAPI(Input* inp);

	/** @brief Binds GLM types (vec2, vec3, etc.) to Lua. */
	void bindGLM();
	/** @brief Binds engine-specific classes (Entity, Component) to Lua. */
	void bindEngine();
	/** @brief Binds the engine logger to Lua. */
	void bindLogger();

	/**
	 * @brief Loads and compiles a script for a specific entity.
	 */
	void LoadScript(Entity entity, ScriptingComponent& sc);

	/**
	 * @brief Reloads a script file from disk.
	 */
	void ReloadScript(Entity entity, ScriptingComponent& sc);

	/**
	 * @brief Helper to call a Lua function safely with error reporting.
	 */
	template<typename... Args>
	void SafeCall(sol::protected_function& func, Args&&... args)
	{
		auto result = func(std::forward<Args>(args)...);

		if (!result.valid()) {
			sol::error err = result;
			SCRP_ERROR("[Lua Call Error] {0}", err.what());
		}
	}

private:
	/** @brief Internal shutdown logic. */
	void Shutdown();

	std::unordered_map<std::string, sol::bytecode> scriptCache_; ///< Cache of compiled Lua bytecode.
	std::set<Entity> entitiesToReload_;                          ///< Entities marked for script reloading.
	std::unique_ptr<sol::state> globalLuaState_;                 ///< The primary Lua state.
	ECSManager& m_world;                                         ///< Reference to the ECS world.
};


#endif // !SCRIPTSYSTEM_HPP
