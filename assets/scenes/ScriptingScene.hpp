#ifndef SCRIPTINGSCENE_HPP
#define SCRIPTINGSCENE_HPP 1

#include "Pengu_Engine/Scene/SceneBase.hpp"
#include "Pengu_Engine/PenguEngine.hpp"

namespace Pengu::Resources { class ResourceManager; }

namespace Pengu::Scene {
	class ScriptingScene : public SceneBase {
	public:
		ScriptingScene(Pengu::Core::PenguEngine* engine) : SceneBase(engine) { m_name = "Scripting Scene"; }

		void update(float dt) override;
		void onLoad(Pengu::Resources::ResourceManager& rm) override;
		void onUnload() override;
	protected:
		void onInitialize() override;

	private:
		std::shared_ptr<Scripting> scriptsys;
	};
}

#endif // !SCRIPTINGSCENE_HPP
