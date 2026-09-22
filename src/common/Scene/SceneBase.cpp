#include "Pengu_Engine/Scene/SceneBase.hpp"
#include "Pengu_Engine/Managers/Resource/ResourceManager.hpp"

namespace Pengu::Scene {

	void SceneBase::registerRenderable(Entity id) {
		auto& ecs = m_world;
		if (ecs->HasComponent<DrawableComponent>(id))
			m_renderables.push_back(id);
	}

	void SceneBase::unregisterRenderable(Entity id) {
		m_renderables.erase(
			std::remove(m_renderables.begin(), m_renderables.end(), id),
			m_renderables.end()
		);
	}
}