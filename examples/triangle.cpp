#include "Pengu_Engine/PenguEngine.hpp"
#include "Pengu_Engine/Misc/FramRate.hpp"
#include "Pengu_Engine/Graphics/Mesh.hpp"
#include "Pengu_Engine/Graphics/Shader.hpp"
#include "GL/glew.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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

	std::shared_ptr<Pengu::Graphics::Shader> shader = engine.GetResourceManager().getShader("../assets/shaders/triangle.vert", "../assets/shaders/triangle.frag");

	std::vector<Pengu::Graphics::Vertex> triangle;

	Pengu::Graphics::Vertex v1
	{
		.position = glm::vec3(-0.5f, -0.5f, 0.0f),
			.normal = glm::vec3(0.0f, 0.0f, 1.0f),
			.uv = glm::vec2(0.0f, 0.0f),
	};
	triangle.push_back(v1);
	Pengu::Graphics::Vertex v2
	{
		.position = glm::vec3(0.5f, -0.5f, 0.0f),
			.normal = glm::vec3(0.0f, 0.0f, 1.0f),
			.uv = glm::vec2(1.0f, 0.0f),
	};
	triangle.push_back(v2);
	Pengu::Graphics::Vertex v3
	{
		.position = glm::vec3(0.0f,  0.5f, 0.0f),
			.normal = glm::vec3(0.0f, 0.0f, 1.0f),
			.uv = glm::vec2(0.5f, 1.0f),
	};
	triangle.push_back(v3);

	std::vector<unsigned int> indices = {
		0, 1, 2
	};

	Pengu::Graphics::Mesh tri;
	tri.upload(triangle, indices);



	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.0f));

	while (!engine.IsClosing() &&
		!engine.GetInput().isDown(Action::Escape)) {

		FrameRate::Get().FPSTick();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader->bind();
		shader->setMat4("model", model);
		shader->setMat4("projection", engine.GetCamera().projection_);
		shader->setMat4("view", engine.GetCamera().getViewMatrix());

		tri.Draw(false);

		engine.EndFrame();
	}

	return 0;
}