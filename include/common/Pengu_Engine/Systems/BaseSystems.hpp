/**
 * @file BaseSystems.hpp
 * @brief Defines the base class for all ECS systems.
 */

#ifndef BASESYSTEMS_HPP
#define BASESYSTEMS_HPP 1

#include "Pengu_Engine/Managers/ECS/EntityManager.hpp"

#include <set>

/**
 * @class System
 * @brief Base class for systems that operate on entities with specific component signatures.
 * 
 * A system maintains a list of entities that match its required component signature.
 */
class System {
public:
	/**
	 * @brief Set of entities currently managed by this system.
	 */
	std::set<Entity> entities_;
};

#endif // !BASESYSTEMS_HPP
