#include "Pengu_Engine/Managers/ECS/EntityManager.hpp"
#include "Pengu_Engine/Misc/Logmacros.hpp"
#include <exception>

EntityManager::EntityManager()
{
	for (EntityIndex i = 0; i < MAX_ENTITIES; ++i) {
		availableIndices_.push(i);
		versions_[i] = 0;
	}
}

Entity EntityManager::CreateEntity()
{
	EntityIndex id = availableIndices_.front();
	availableIndices_.pop();
	Entity entity = { id, versions_[id] };
	livingEntities_.push_back(entity);
	return entity;
}

void EntityManager::DestroyEntity(Entity entity)
{
	versions_[entity.id]++; // Increment version to invalidate old handles
	signatures_[entity.id].reset();
	availableIndices_.push(entity.id);

	for (auto it = livingEntities_.begin(); it != livingEntities_.end(); ++it) {
		if (it->id == entity.id) {
			livingEntities_.erase(it);
			break;
		}
	}
}

void EntityManager::SetSignature(Entity entity, Signature signature)
{

	if (entity.id >= MAX_ENTITIES) {
		throw std::out_of_range(
			"EntityManager::SetSignature - Entity out of range."
		);
	}

	signatures_[entity.id] = signature;
}

Signature EntityManager::GetSignature(Entity entity)
{
	if (entity.id >= MAX_ENTITIES) {
		ENGINE_WARNING("Entity out of range");
		throw std::out_of_range(
			"EntityManager::GetSignature - Entity out of range"
		);
	}

	return signatures_[entity.id];
}