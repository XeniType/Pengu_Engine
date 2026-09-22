#include "Pengu_Engine/Managers/ECS/ECSManager.hpp"
#include "Pengu_Engine/Misc/Logmacros.hpp"

ECSManager::ECSManager()
	: entityManager_(nullptr), componentManager_(nullptr), systemManager_(nullptr) {
}

void ECSManager::Init() {
	componentManager_ = std::make_unique<ComponentManager>();
	entityManager_ = std::make_unique<EntityManager>();
	systemManager_ = std::make_unique<SystemManager>();
}

Entity ECSManager::CreateEntity() {
	return entityManager_->CreateEntity();
}

void ECSManager::DestroyEntity(Entity entity) {
	try {
		if (HasComponent<TagComponent>(entity)) {
			tagMap_.erase(GetComponent<TagComponent>(entity).tag);
		}

		if (HasComponent<DrawableComponent>(entity)) {
			if (onRenderableRemoved) {
				onRenderableRemoved(entity);
			}
		}

		entityManager_->DestroyEntity(entity);
		componentManager_->EntityDestroyed(entity);
		systemManager_->EntityDestroyed(entity);
	}
	catch (const std::exception& e) {
		ENGINE_WARNING(e.what());
	}
}

const std::vector<Entity>& ECSManager::GetAllEntities() const {
	return entityManager_->GetLivingEntities();
}

Entity ECSManager::GetEntityByTag(const std::string& tag) const {
	auto it = tagMap_.find(tag);
	if (it != tagMap_.end()) {
		return it->second;
	}
	return Entity{ static_cast<uint32_t>(-1), 0 };
}