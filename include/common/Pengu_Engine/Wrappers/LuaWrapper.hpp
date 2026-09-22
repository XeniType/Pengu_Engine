/**
 * @file LuaWrapper.hpp
 * @brief Lua-friendly wrappers for ECS objects.
 */

#ifndef LUAWRAPPER_HPP
#define LUAWRAPPER_HPP 1

#include "Pengu_Engine/Managers/ECS/EntityManager.hpp"
#include "Pengu_Engine/Managers/ECS/ComponentManager.hpp"
#include "Pengu_Engine/Managers/ECS/ECSManager.hpp"
#include "Pengu_Engine/Misc/Logmacros.hpp"


#include <memory>

/**
 * @class LuaEntity
 * @brief A wrapper around an ECS Entity handle for use within Lua scripts.
 */
class LuaEntity {
public:
	/**
	 * @brief Constructs a Lua-compatible entity handle.
	 * @param e The entity handle.
	 * @param ecs Reference to the ECS manager.
	 */
	LuaEntity(Entity e, ECSManager& ecs) : entity_{ e }, ecs_{ ecs } {}

	/**
	 * @brief Template helper to get a component for the entity from Lua.
	 * @tparam T The component type.
	 * @return Reference to the component data.
	 */
	template<typename T>
	T& GetComponent()
	{
		return ecs_.GetComponent<T>(entity_);
	}

	Entity entity_;    ///< The wrapped entity handle.
	ECSManager& ecs_; ///< Reference to the ECS manager.
};

#endif // !LUAWRAPPER_HPP
