#include "Pengu_Engine/Managers/ECS/SystemManager.hpp"

void SystemManager::EntitySignatureChanged(Entity entity, Signature entitySignature) {
	for (auto const& pair : systems_) {
		auto const& system = pair.second;
		auto const& systemSignature = signatures_[pair.first];

		// Strict bitwise check
		if ((entitySignature & systemSignature) == systemSignature) {
			system->entities_.insert(entity);
		}
		else {
			system->entities_.erase(entity);
		}
	}
}