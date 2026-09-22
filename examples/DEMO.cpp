#include "Pengu_Engine/Scene/Scene.hpp"
#include "Pengu_Engine/PenguEngine.hpp"
#include "scenes/Demo_Scene.hpp"
#include "Pengu_Engine/Misc/FramRate.hpp"
#include "Pengu_Engine/Editor/EngineUI.hpp"
#include "Pengu_Engine/Editor/EditorState.hpp"

#include <iostream>

using Pengu::Core::PenguEngine;
using Pengu::Core::EngineConfig;

static EngineConfig config{
	.screen_width = 1280,
	.screen_height = 960,
	.title = "Pengu_Engine",
	.pipeline = Pengu::Core::RenderPipeline::Deferred };

int main() {
	auto engine = PenguEngine::startEngine(config);

	engine.loadScene(std::make_unique<Pengu::Scene::DemoScene>(&engine));

	EditorState state;
	EngineUI    ui;
	ui.init(engine.GetWindow().GetWindow(), &engine);

	while (!engine.IsClosing())
	{
		auto t0 = std::chrono::high_resolution_clock::now();

		engine.update();
		ui.render(state);

		auto t1 = std::chrono::high_resolution_clock::now();
		state.lastFrameMs = std::chrono::duration<float, std::milli>(t1 - t0).count();
		state.fps = 1000.f / state.lastFrameMs;

		engine.EndFrame();
	}

	ui.shutdown();

	return 0;
}