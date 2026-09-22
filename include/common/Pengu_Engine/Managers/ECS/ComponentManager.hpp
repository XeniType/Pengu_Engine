/**
 * @file ComponentManager.hpp
 * @brief Manages component arrays and registration.
 */

#ifndef COMPONENTMANAGER_HPP
#define COMPONENTMANAGER_HPP 1

#include "Pengu_Engine/Misc/Logmacros.hpp"

#include "Pengu_Engine/Managers/ECS/Entity.hpp"
#include <vector>
#include <memory>
#include <typeindex>
#include <unordered_map>

/**
 * @class IComponentArray
 * @brief Interface for generic component storage.
 */
class IComponentArray {
public:
	virtual ~IComponentArray() = default;
	/**
	 * @brief Called when an entity is destroyed to remove its component data.
	 * @param entity The entity handle.
	 */
	virtual void EntityDestroyed(Entity entity) = 0;
};

/**
 * @class ComponentArray
 * @brief SoA (Structure of Arrays) storage for components of type T.
 * 
 * Uses a sparse-dense array structure to maintain packed component data for
 * high cache efficiency during iterations.
 */
template<typename T>
class ComponentArray : public IComponentArray {
public:

	/**
	 * @brief Constructs the component array and pre-allocates the sparse array.
	 */
	ComponentArray() {
		sparseArray_.assign(MAX_ENTITIES, 0xFFFFFFFF);
	}

	/**
	 * @brief Inserts component data for an entity.
	 * @param entity The entity handle.
	 * @param component The component data.
	 */
	void InsertData(Entity entity, T component)
	{
		size_t newIndex = denseArray_.size();
		sparseArray_[entity.id] = static_cast<EntityIndex>(newIndex);
		denseArray_.push_back(component);
		indexToEntity_.push_back(entity);
	}

	/**
	 * @brief Removes component data for an entity, keeping the dense array packed.
	 * @param entity The entity handle.
	 */
	void RemoveData(Entity entity)
	{
		EntityIndex indexOfRemoved = sparseArray_[entity.id];
		EntityIndex indexOfLast = static_cast<EntityIndex>(denseArray_.size() - 1);

		denseArray_[indexOfRemoved] = denseArray_[indexOfLast];
		Entity entityOfLast = indexToEntity_[indexOfLast];
		sparseArray_[entityOfLast.id] = indexOfRemoved;
		indexToEntity_[indexOfRemoved] = entityOfLast;

		sparseArray_[entity.id] = 0xFFFFFFFF;
		denseArray_.pop_back();
		indexToEntity_.pop_back();
	}

	/**
	 * @brief Retrieves component data for an entity.
	 * @param entity The entity handle.
	 * @return Reference to the component data.
	 */
	T& GetData(Entity entity)
	{
		return denseArray_[sparseArray_[entity.id]];
	}

	/**
	 * @brief Retrieves component data for an entity (const).
	 * @param entity The entity handle.
	 * @return Constant reference to the component data.
	 */
	const T& GetData(Entity entity) const
	{
		return denseArray_[sparseArray_[entity.id]];
	}

	/**
	 * @brief Checks if an entity has data in this array.
	 * @param entity The entity handle.
	 * @return True if the entity has data, false otherwise.
	 */
	bool HasData(Entity entity) const {
		return entity.id < sparseArray_.size() && sparseArray_[entity.id] != 0xFFFFFFFF;
	}

	/**
	 * @brief Implementation of IComponentArray::EntityDestroyed.
	 * @param entity The entity handle.
	 */
	void EntityDestroyed(Entity entity) override
	{
		if (HasData(entity)) RemoveData(entity);
	}

private:
	std::vector<T> denseArray_;           ///< Packed array of component data.
	std::vector<EntityIndex> sparseArray_; ///< Map from entity ID to index in denseArray.
	std::vector<Entity> indexToEntity_;   ///< Map from denseArray index back to entity.

	size_t size_ = 0; ///< Current number of components in the array.
};

/**
 * @class ComponentManager
 * @brief Orchestrates multiple component arrays.
 */
class ComponentManager {
public:
	/**
	 * @brief Registers a new component type.
	 * @tparam T The component type to register.
	 */
	template<typename T>
	void RegisterComponent()
	{
		std::type_index typeName = typeid(T);
		componentTypes_[typeName] = nextComponentType_++;
		componentArrays_[typeName] = std::make_shared<ComponentArray<T>>();
	}

	/**
	 * @brief Checks if a component type is registered.
	 * @tparam T The component type to check.
	 * @return True if registered, false otherwise.
	 */
	template<typename T>
	bool IsComponentRegistered() const
	{
		return componentTypes_.find(typeid(T)) != componentTypes_.end();
	}

	/**
	 * @brief Gets the unique ID for a component type.
	 * @tparam T The component type.
	 * @return The component type ID.
	 */
	template<typename T>
	ComponentType GetComponentType() const
	{
		return componentTypes_.at(typeid(T));
	}

	/**
	 * @brief Adds a component to an entity.
	 * @tparam T The component type.
	 * @param entity The entity handle.
	 * @param component The component data.
	 */
	template<typename T>
	void AddComponent(Entity entity, T component)
	{
		GetComponentArray<T>()->InsertData(entity, component);
	}

	/**
	 * @brief Removes a component from an entity.
	 * @tparam T The component type.
	 * @param entity The entity handle.
	 */
	template<typename T>
	void RemoveComponent(Entity entity)
	{
		GetComponentArray<T>()->RemoveData(entity);
	}

	/**
	 * @brief Retrieves a component for an entity.
	 * @tparam T The component type.
	 * @param entity The entity handle.
	 * @return Reference to the component data.
	 */
	template<typename T>
	T& GetComponent(Entity entity)
	{
		return GetComponentArray<T>()->GetData(entity);
	}

	/**
	 * @brief Retrieves a component for an entity (const).
	 * @tparam T The component type.
	 * @param entity The entity handle.
	 * @return Constant reference to the component data.
	 */
	template<typename T>
	const T& GetComponent(Entity entity) const
	{
		return GetComponentArray<T>()->GetData(entity);
	}

	/**
	 * @brief Checks if an entity has a specific component.
	 * @tparam T The component type.
	 * @param entity The entity handle.
	 * @return True if the entity has the component, false otherwise.
	 */
	template<typename T>
	bool HasComponent(Entity entity) const {
		return GetComponentArray<T>()->HasData(entity);
	}

	/**
	 * @brief Notifies all component arrays that an entity has been destroyed.
	 * @param entity The entity handle.
	 */
	void EntityDestroyed(Entity entity)
	{
		for (auto const& pair : componentArrays_) {
			pair.second->EntityDestroyed(entity);
		}
	}

private:
	std::unordered_map<std::type_index, ComponentType> componentTypes_{}; ///< Map from type to component ID.

	std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> componentArrays_{}; ///< Map from type to component array.

	ComponentType nextComponentType_{}; ///< Counter for assigning unique component IDs.

	/**
	 * @brief Helper to get the typed component array for T.
	 * @tparam T The component type.
	 * @return Shared pointer to the component array.
	 */
	template<typename T>
	std::shared_ptr<ComponentArray<T>> GetComponentArray()
	{
		return std::static_pointer_cast<ComponentArray<T>>(componentArrays_[typeid(T)]);
	}

	/**
	 * @brief Helper to get the typed component array for T (const).
	 * @tparam T The component type.
	 * @return Shared pointer to the constant component array.
	 */
	template<typename T>
	std::shared_ptr<const ComponentArray<T>> GetComponentArray() const
	{
		return std::static_pointer_cast<const ComponentArray<T>>(componentArrays_.at(typeid(T)));
	}
};

#endif // !COMPONENTMANAGER_HPP
