#ifndef DEMO_SCENE_HPP 
#define DEMO_SCENE_HPP 1

#include "Pengu_Engine/Scene/SceneBase.hpp"
#include "Pengu_Engine/PenguEngine.hpp"
#include "Pengu_Engine/Systems/TerrainSystem.hpp"

namespace Pengu::Resources { class ResourceManager; }

namespace Pengu::Scene {

	class DemoScene : public SceneBase {
	public:
		DemoScene(Pengu::Core::PenguEngine* engine) : SceneBase(engine) { m_name = "Demo Scene"; }

		void update(float dt) override;
		void onLoad(Pengu::Resources::ResourceManager& rm) override;
		void onUnload() override;

	protected:
		void onInitialize() override;
	private:
		std::shared_ptr<TerrainSystem> m_terrainSystem;

		bool bFollowTerrain = true;
		float terrainOffset = 35.0f;
		float lerpFactor = 0.1f;
		float lookAheadTime = 3.0f;
	};
}
#endif // ! SCENE_HPP 
