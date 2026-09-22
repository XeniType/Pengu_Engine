#include "Pengu_Engine/PenguEngine.hpp"
#include "../assets/scenes/ECSScene.hpp"

#include <random>

using Pengu::Core::PenguEngine;
using Pengu::Core::EngineConfig;

static EngineConfig config{
	.screen_width = 1280,
	.screen_height = 960,
	.title = "Pengu_Engine",
	.pipeline = Pengu::Core::RenderPipeline::Unlit };

int main() {
	auto engine = PenguEngine::startEngine(config);

	engine.loadScene(std::make_unique<Pengu::Scene::ECSScene>(&engine));

	while (!engine.IsClosing() &&
		!engine.GetInput().isDown(Action::Escape))
	{
		engine.update();

		engine.EndFrame();
	}

	return 0;
}