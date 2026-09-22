/**
 * @file ECSManager.hpp
 * @brief Core Entity Component System (ECS) management.
 * @details This system manages entity lifecycles and component storage using
 * a Type-to-List mapping. It facilitates high-performance data-oriented programming.
 */

#ifndef ECSMANAGER_HPP
#define ECSMANAGER_HPP 1

#include "Pengu_Engine/Managers/ECS/EntityManager.hpp"
#include "Pengu_Engine/Managers/ECS/ComponentManager.hpp"
#include "Pengu_Engine/Managers/ECS/SystemManager.hpp"
#include "Pengu_Engine/Components/TagComponent.hpp"
#include "Pengu_Engine/Components/DrawableComponent.hpp"
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>

/**
 * @class ECSManager
 * @brief Coordinates the entity, component, and system managers.
 * 
 * The ECSManager provides a unified interface for creating entities, adding/removing 
 * components, and registering systems. It acts as the primary entry point for 
 * ECS-based logic in the engine.
 */
class ECSManager {
public:

	/**
	 * @brief Constructs the ECS manager and initializes internal sub-managers.
	 */
	ECSManager();

	/// @name Deleted Operations
	/// @{
	ECSManager(const ECSManager& rvalue) = delete;
	ECSManager& operator = (const ECSManager& rvalue) = delete;
	/// @}

	/**
	 * @brief Move constructor.
	 */
	ECSManager(ECSManager&& rvalue) noexcept : entityManager_(std::move(rvalue.entityManager_)), componentManager_(std::move(rvalue.componentManager_)), systemManager_(std::move(rvalue.systemManager_))
	{

	}

	/**
	 * @brief Move assignment operator.
	 */
	ECSManager& operator = (ECSManager&& rvalue)noexcept
	{
		if (this != &rvalue)
		{
			componentManager_ = std::move(rvalue.componentManager_);
			entityManager_ = std::move(rvalue.entityManager_);
			systemManager_ = std::move(rvalue.systemManager_);
		}

		return *this;
	};

	/**
	 * @brief Initializes the ECS system.
	 */
	void Init();

	/**
	 * @brief Creates a new entity.
	 * @return The created entity handle.
	 */
	Entity CreateEntity();

	/**
	 * @brief Destroys an entity and cleans up its components.
	 * @param entity The entity to destroy.
	 */
	void DestroyEntity(Entity entity);

	/**
	 * @brief Retrieves all currently living entities.
	 * @return A constant reference to the vector of living entities.
	 */
	const std::vector<Entity>& GetAllEntities() const;

	/**
	 * @brief Finds an entity by its tag component value.
	 * @param tag The tag string to search for.
	 * @return The entity handle if found, otherwise an invalid entity.
	 */
	Entity GetEntityByTag(const std::string& tag) const;

	/**
	 * @brief Retrieves all entities that have a specific component type.
	 * @tparam T The component type to search for.
	 * @return A vector of entity handles matching the requirement.
	 */
	template<typename T>
	std::vector<Entity> GetEntitiesWith() const {
		std::vector<Entity> matchingEntities;

		// 1. Get the unique bit-index for this component type
		ComponentType type = componentManager_->GetComponentType<T>();

		// 2. Iterate through all entities currently in the world
		for (Entity entity : GetAllEntities()) {
			// 3. Get the signature (bitset) for this specific entity
			Signature entitySignature = entityManager_->GetSignature(entity);

			// 4. Check if the bit for component T is set to 'true'
			if (entitySignature.test(type)) {
				matchingEntities.push_back(entity);
			}
		}

		return matchingEntities;
	}

	std::function<void(Entity)> onRenderableAdded;   ///< Callback triggered when a DrawableComponent is added.
	std::function<void(Entity)> onRenderableRemoved; ///< Callback triggered when a DrawableComponent is removed.

	std::function<void(Entity)> onTerrainAdded;      ///< Callback triggered when a TerrainDrawableComponent is added.
	std::function<void(Entity)> onTerrainRemoved;    ///< Callback triggered when a TerrainDrawableComponent is removed.

	/**
	 * @brief Registers a new component type with the system.
	 * @tparam T The component type to register.
	 */
	template<typename T>
	void RegisterComponent() {
		componentManager_->RegisterComponent<T>();
	};

	/**
	 * @brief Checks if a component type is already registered.
	 * @tparam T The component type to check.
	 * @return True if registered, false otherwise.
	 */
	template<typename T>
	bool IsComponentRegistered() {
		return componentManager_->IsComponentRegistered<T>();
	};

	/**
	 * @brief Adds a component to an entity.
	 * @tparam T The component type.
	 * @param entity The entity handle.
	 * @param component The component data.
	 */
	template<typename T>
	void AddComponent(Entity entity, T component) {
		componentManager_->AddComponent<T>(entity, component);

		if constexpr (std::is_same_v<T, TagComponent>) {
			tagMap_[component.tag] = entity;
		}

		if constexpr (std::is_same_v<T, DrawableComponent>) {
			if (onRenderableAdded) {
				onRenderableAdded(entity);
			}
		}

		if constexpr (std::is_same_v<T, TerrainDrawableComponent>) {
			if (onTerrainAdded) {
				onTerrainAdded(entity);
			}
		}

		auto signature = entityManager_->GetSignature(entity);
		signature.set(componentManager_->GetComponentType<T>(), true);
		entityManager_->SetSignature(entity, signature);

		systemManager_->EntitySignatureChanged(entity, signature);
	};

	/**
	 * @brief Removes a component from an entity.
	 * @tparam T The component type to remove.
	 * @param entity The entity handle.
	 */
	template<typename T>
	void RemoveComponent(Entity entity) {
		if constexpr (std::is_same_v<T, TagComponent>) {
			if (componentManager_->HasComponent<TagComponent>(entity)) {
				tagMap_.erase(componentManager_->GetComponent<TagComponent>(entity).tag);
			}
		}

		if constexpr (std::is_same_v<T, DrawableComponent>) {
			if (onRenderableRemoved) {
				onRenderableRemoved(entity);
			}
		}

		if constexpr (std::is_same_v<T, TerrainDrawableComponent>) {
			if (onTerrainRemoved) {
				onTerrainRemoved(entity);
			}
		}

		componentManager_->RemoveComponent<T>(entity);

		auto signature = entityManager_->GetSignature(entity);
		signature.set(componentManager_->GetComponentType<T>(), false);
		entityManager_->SetSignature(entity, signature);

		systemManager_->EntitySignatureChanged(entity, signature);
	};

	/**
	 * @brief Retrieves a reference to an entity's component.
	 * @tparam T The component type.
	 * @param entity The entity handle.
	 * @return Reference to the component data.
	 */
	template<typename T>
	T& GetComponent(Entity entity) {
		return componentManager_->GetComponent<T>(entity);
	};

	/**
	 * @brief Retrieves a constant reference to an entity's component.
	 * @tparam T The component type.
	 * @param entity The entity handle.
	 * @return Constant reference to the component data.
	 */
	template<typename T>
	const T& GetComponent(Entity entity) const {
		return componentManager_->GetComponent<T>(entity);
	};

	/**
	 * @brief Checks if an entity has a specific component type.
	 * @tparam T The component type.
	 * @param entity The entity handle.
	 * @return True if the entity has the component, false otherwise.
	 */
	template<typename T>
	bool HasComponent(Entity entity) const {
		return componentManager_->HasComponent<T>(entity);
	}

	/**
	 * @brief Gets the unique ID for a component type.
	 * @tparam T The component type.
	 * @return The component type ID.
	 */
	template<typename T>
	ComponentType GetComponentType() const {
		return componentManager_->GetComponentType<T>();
	};

	/**
	 * @brief Registers a new system with the manager.
	 * @tparam T The system type.
	 * @tparam Args Argument types for the system constructor.
	 * @param args Arguments for the system constructor.
	 * @return Shared pointer to the registered system.
	 */
	template<typename T, typename... Args>
	std::shared_ptr<T> RegisterSystem(Args&&... args) {
		return systemManager_->RegisterSystem<T>(std::forward<Args>(args)...);
	};

	/**
	 * @brief Sets the signature requirement for a specific system type.
	 * @tparam T The system type.
	 * @param signature The bitset signature required by the system.
	 */
	template<typename T>
	void SetSystemSignature(Signature signature) {
		return systemManager_->SetSignature<T>(signature);
	};

private:
	std::unique_ptr<ComponentManager> componentManager_; ///< Manages component storage and arrays.
	std::unique_ptr<EntityManager> entityManager_;       ///< Manages entity IDs and lifecycles.
	std::unique_ptr<SystemManager> systemManager_;       ///< Manages system registration and updates.

	std::unordered_map<std::string, Entity> tagMap_;     ///< Fast lookup map for tagged entities.
};

#endif // ECSMANAGER_HPP
