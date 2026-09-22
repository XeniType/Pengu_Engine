#ifndef DEFERREDSCENE_HPP 
#define DEFERREDSCENE_HPP 1

#include "Pengu_Engine/Scene/SceneBase.hpp"
#include "Pengu_Engine/PenguEngine.hpp"
#include "Pengu_Engine/Systems/TerrainSystem.hpp"

namespace Pengu::Resources { class ResourceManager; }

namespace Pengu::Scene {

	class DeferredScene : public SceneBase {
	public:
		DeferredScene(Pengu::Core::PenguEngine* engine) : SceneBase(engine) { m_name = "Deferred Scene"; }

		void update(float dt) override;
		void onLoad(Pengu::Resources::ResourceManager& rm) override;
		void onUnload() override;

	protected:
		void onInitialize() override;
	private:
		std::shared_ptr<TerrainSystem> m_terrainSystem;
	};
}
#endif // ! SCENE_HPP 
