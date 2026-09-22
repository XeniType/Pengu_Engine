/**
 * @file EntityManager.hpp
 * @brief Manages entity creation, destruction, and signatures.
 */

#ifndef ENTITYMANAGER_HPP
#define ENTITYMANAGER_HPP 1

#include "Pengu_Engine/Managers/ECS/Entity.hpp"
#include <queue>
#include <array>
#include <vector>

/**
 * @class EntityManager
 * @brief Responsible for issuing entity IDs and maintaining their component signatures.
 */
class EntityManager {

public:
	/**
	 * @brief Constructs the entity manager and populates the pool of available IDs.
	 */
	EntityManager();

	/**
	 * @brief Requests a new entity ID from the pool.
	 * @return The created entity handle.
	 */
	Entity CreateEntity();

	/**
	 * @brief Returns an entity ID to the pool and invalidates its signature.
	 * @param entity The entity handle to destroy.
	 */
	void DestroyEntity(Entity entity);

	/**
	 * @brief Sets the component signature for an entity.
	 * @param entity The entity handle.
	 * @param signature The bitset representing attached components.
	 */
	void SetSignature(Entity entity, Signature signature);

	/**
	 * @brief Retrieves the component signature for an entity.
	 * @param entity The entity handle.
	 * @return The entity's component signature.
	 */
	Signature GetSignature(Entity entity);

	/**
	 * @brief Gets a list of all currently active entities.
	 * @return Constant reference to the vector of living entities.
	 */
	const std::vector<Entity>& GetLivingEntities() const { return livingEntities_; }

private:
	std::queue<EntityIndex> availableIndices_{};         ///< Pool of available entity IDs.
	std::array<EntityVersion, MAX_ENTITIES> versions_{};  ///< Version tracking to prevent use of stale handles.
	std::array<Signature, MAX_ENTITIES> signatures_{};   ///< Component signatures for all entities.
	std::vector<Entity> livingEntities_{};               ///< List of active entities for fast iteration.
};

#endif // !ENTITYMANAGER_HPP
