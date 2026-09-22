/**
 * @file Entity.hpp
 * @brief Defines the basic types for the Entity Component System.
 */

#ifndef ENTITY_HPP
#define ENTITY_HPP 1

#include <cstdint>
#include <bitset>

/**
 * @typedef EntityIndex
 * @brief Type used for entity IDs.
 */
using EntityIndex = std::uint32_t;

/**
 * @typedef EntityVersion
 * @brief Type used for entity versions to prevent stale handle issues.
 */
using EntityVersion = std::uint32_t;

/**
 * @struct Entity
 * @brief A handle to an entity in the ECS.
 */
struct Entity {
	EntityIndex id;      ///< Unique index of the entity.
	EntityVersion version; ///< Version of the entity at the time of creation.

	/**
	 * @brief Equality operator.
	 */
	bool operator==(const Entity& other) const { return id == other.id && version == other.version; }
	
	/**
	 * @brief Inequality operator.
	 */
	bool operator!=(const Entity& other) const { return id != other.id && version != other.version; }
	
	/**
	 * @brief Less-than operator for sorting/mapping.
	 */
	bool operator<(const Entity& other) const { return id < other.id; }
	
	/**
	 * @brief Checks if the entity handle is valid.
	 */
	bool isValid() const { return id != 0xFFFFFFFF; }
};

/**
 * @brief Maximum number of entities allowed in the system.
 */
const EntityIndex MAX_ENTITIES = 50000;

/**
 * @brief Maximum number of distinct component types allowed.
 */
const std::uint8_t MAX_COMPONENTS = 32;

/**
 * @typedef Signature
 * @brief A bitset representing the set of components attached to an entity.
 */
using Signature = std::bitset<MAX_COMPONENTS>;

/**
 * @typedef ComponentType
 * @brief Type used for component type IDs.
 */
using ComponentType = std::uint8_t;

#endif // !ENTITY_HPP
