#include "Pengu_Engine/PenguEngine.hpp"
#include "../assets/scenes/JobSysScene.hpp"
#include "GL/glew.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using Pengu::Core::PenguEngine;
using Pengu::Core::EngineConfig;

static EngineConfig config{
	.screen_width = 1280,
	.screen_height = 960,
	.title = "Pengu_Engine",
	.pipeline = Pengu::Core::RenderPipeline::Unlit };

int main() {
	auto engine = PenguEngine::startEngine(config);

	engine.loadScene(std::make_unique<Pengu::Scene::JobSysScene>(&engine));

	Input& inp = engine.GetInput();
	Camera& cam = engine.GetCamera();

	while (!engine.IsClosing() &&
		!inp.isPressed(Action::Escape))
	{

		engine.update();
		engine.GetInput().update();
		engine.EndFrame();
	}

	return 0;

}