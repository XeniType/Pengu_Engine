#ifndef JOBSYSSCENE_HPP 
#define JOBSYSSCENE_HPP 1

#include "Pengu_Engine/Scene/SceneBase.hpp"
#include "Pengu_Engine/PenguEngine.hpp"

namespace Pengu::Resources { class ResourceManager; }

namespace Pengu::Scene {

	class JobSysScene : public SceneBase {
	public:
		JobSysScene(Pengu::Core::PenguEngine* engine) : SceneBase(engine) { m_name = "Job System Scene"; }

		void update(float dt) override;
		void onLoad(Pengu::Resources::ResourceManager& rm) override;
		void onUnload() override;

	protected:
		void onInitialize() override;
	private:
		std::vector<std::string> paths;
		unsigned int current_path = 0;
	};
}
#endif // ! JOBSYSSCENE_HPP 
