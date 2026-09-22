/**
 * @file LightSystem.hpp
 * @brief System for managing and collecting light data in the ECS.
 */

#ifndef LIGHTSYSTEM_HPP
#define LIGHTSYSTEM_HPP 1

#include "Pengu_Engine/Systems/BaseSystems.hpp"
#include "Pengu_Engine/Components/LightComponent.hpp"
#include "Pengu_Engine/Components/TransformComponent.hpp"
#include "Pengu_Engine/Managers/ECS/ECSManager.hpp"

#include <vector>

/**
 * @struct LightData
 * @brief Helper structure containing a light component and its world transform.
 */
struct LightData {
	Lights* light;                     ///< Pointer to the light component.
	const TransformComponent* transform; ///< Pointer to the transform component.
};

/**
 * @class LightSystem
 * @brief System that processes entities with light components.
 */
class LightSystem : public System {
public:
	LightSystem() = default;
	~LightSystem() = default;

	/**
	 * @brief Collects all active lights and their transforms from the ECS.
	 * @param ecs Reference to the ECS manager.
	 * @return A vector of LightData structures.
	 */
	std::vector<LightData> collectLights(ECSManager& ecs) const {
		std::vector<LightData> result;
		for (auto& light : entities_) {
			if (ecs.HasComponent<Lights>(light)) {
				result.push_back({
					&ecs.GetComponent<Lights>(light)
					});
			}
		}
		return result;
	}
};

#endif // !LIGHTSYSTEM_HPP
