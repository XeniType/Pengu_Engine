#ifndef ECSSCENE_HPP 
#define ECSSCENE_HPP 1

#include "Pengu_Engine/Scene/SceneBase.hpp"
#include "Pengu_Engine/PenguEngine.hpp"

namespace Pengu::Resources { class ResourceManager; }

namespace Pengu::Scene {

	class ECSScene : public SceneBase {
	public:
		ECSScene(Pengu::Core::PenguEngine* engine) : SceneBase(engine) { m_name = "ECS_Scene"; }

		void update(float dt) override;
		void onLoad(Pengu::Resources::ResourceManager& rm) override;
		void onUnload() override;

	protected:
		void onInitialize() override;
	private:
	};
}
#endif // ! SCENE_HPP 
