#include "Pengu_Engine/PenguEngine.hpp"
#include "Pengu_Engine/Misc/FramRate.hpp"
#include "Pengu_Engine/Misc/Logmacros.hpp"
#include <optional>

using Pengu::Core::PenguEngine;
using Pengu::Core::EngineConfig;

static EngineConfig config{
	.screen_width = 1280,
	.screen_height = 960,
	.title = "Pengu_Engine",
	.pipeline = Pengu::Core::RenderPipeline::Unlit };

int main() {

	auto engine = PenguEngine::startEngine(config);

	while (!engine.IsClosing())
	{
		auto t0 = std::chrono::high_resolution_clock::now();

		engine.update();

		engine.EndFrame();
	}

	return 0;
}