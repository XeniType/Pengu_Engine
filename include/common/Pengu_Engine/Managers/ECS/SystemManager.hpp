/**
 * @file SystemManager.hpp
 * @brief Manages registration and signature matching for ECS systems.
 */

#ifndef SYSTEMMANAGER_HPP
#define SYSTEMMANAGER_HPP 1

#include "Pengu_Engine/Managers/ECS/EntityManager.hpp"
#include "Pengu_Engine/Systems/BaseSystems.hpp"

#include <stdexcept>
#include <typeindex>
#include <set>
#include <memory>
#include <unordered_map>

 /**
	* @class SystemManager
	* @brief Responsible for maintaining a list of registered systems and their required signatures.
	*/
class SystemManager {

public:
	/**
	 * @brief Registers a new system.
	 * @tparam T The system type.
	 * @tparam Args Argument types for the system constructor.
	 * @param args Arguments for the system constructor.
	 * @return Shared pointer to the registered system.
	 * @throws std::logic_error if the system is already registered.
	 */
	template<typename T, typename... Args>
	std::shared_ptr<T> RegisterSystem(Args&&... args) {
		std::type_index typeName = typeid(T);

		if (systems_.find(typeName) != systems_.end()) {
			throw std::logic_error(
				"SystemManager::RegisterSystem - Registering system more than once."
			);
		}

		auto system = std::make_shared<T>(std::forward<Args>(args)...);
		systems_.insert({ typeName, system });
		return system;
	};

	/**
	 * @brief Sets the signature required by a system.
	 * @tparam T The system type.
	 * @param signature The component signature required for an entity to be processed by this system.
	 * @throws std::logic_error if the system has not been registered.
	 */
	template<typename T>
	void SetSignature(Signature signature) {
		std::type_index typeName = typeid(T);

		if (systems_.find(typeName) == systems_.end()) {
			throw std::logic_error(
				"SystemManager::SetSignature - System used before registered."
			);
		}

		signatures_.insert({ typeName, signature });
	};

	/**
	 * @brief Removes an entity from all systems when it is destroyed.
	 * @param entity The entity handle.
	 */
	void EntityDestroyed(Entity entity) {
		for (auto const& pair : systems_) {
			auto const& system = pair.second;

			system->entities_.erase(entity);
		}
	}

	/**
	 * @brief Notifies systems that an entity's component signature has changed.
	 * @param entity The entity handle.
	 * @param entitySignature The new signature of the entity.
	 */
	void EntitySignatureChanged(Entity entity, Signature entitySignature);

private:
	std::unordered_map<std::type_index, Signature> signatures_{}; ///< Required signatures for each system type.
	std::unordered_map<std::type_index, std::shared_ptr<System>> systems_{}; ///< Map of registered systems.
};

#endif // !SYSTEMMANAGER_HPP
