#include "Pengu_Engine/Scene/SceneManager.hpp"
#include "Pengu_Engine/Scene/SceneBase.hpp"
#include "Pengu_Engine/Managers/Resource/ResourceManager.hpp"
#include "Pengu_Engine/PenguEngine.hpp"
#include "Pengu_Engine/Misc/Logmacros.hpp"

namespace Pengu::Scene {

	void SceneManager::loadScene(std::unique_ptr<SceneBase> newScene, Pengu::Resources::ResourceManager& rm)
	{

		if (!newScene) {
			ENGINE_WARNING("SceneManager: cannot load a null scene");
			return;
		}

		if (m_activeScene) {
			m_activeScene->onUnload();
			m_activeScene.reset();
		}

		m_activeScene = std::move(newScene);
		m_activeScene->onLoad(rm);

	}

	void SceneManager::unloadCurrent()
	{

		if (m_activeScene) {
			m_activeScene->onUnload();
			m_activeScene.reset();
		}

	}

	void SceneManager::update(float dt)
	{

		if (m_activeScene) m_activeScene->update(dt);

	}
}
