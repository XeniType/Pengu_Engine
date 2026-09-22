#include "Pengu_Engine/Graphics/Rendering/Techniques/Unlit.hpp"
#include "Pengu_Engine/Managers/Resource/ResourceManager.hpp"
#include "Pengu_Engine/Scene/SceneBase.hpp"
#include "Pengu_Engine/Graphics/Shader.hpp"

namespace Pengu::Graphics::Rendering {


	void UnlitPipeline::init(Pengu::Resources::ResourceManager& rm)
	{
		aux_rm = &rm;

		m_shader = rm.getShader("../assets/shaders/unlit.vert", "../assets/shaders/unlit.frag");

		m_initialized = true;
	}

	void UnlitPipeline::cleanup()
	{
		m_shader.reset();
		m_initialized = false;
	}

	void UnlitPipeline::reload(Pengu::Resources::ResourceManager& rm)
	{
		cleanup();
		init(rm);
	}

	void UnlitPipeline::render(Pengu::Scene::SceneBase& scene, Camera& camera)
	{
		if (!m_initialized || !m_shader) return;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_shader->bind();
		m_shader->setMat4("view", camera.getViewMatrix());
		m_shader->setMat4("projection", camera.projection_);
		m_shader->setVec3("viewPos", camera.getPosition());

		auto& world = scene.getWorld();
		for (auto id : scene.getRenderables()) {
			auto& draw = world.GetComponent<DrawableComponent>(id);
			auto& transform = world.GetComponent<TransformComponent>(id);

			m_shader->setMat4("model", buildModelMatrix(transform));
			drawSubMeshes(draw);
		}

		resetBlendState();
	}

	void UnlitPipeline::onResize(int width, int height)
	{
		m_width = width;
		m_height = height;

		reload(*aux_rm);
	}

	void UnlitPipeline::drawSubMeshes(const DrawableComponent& draw)
	{
		for (auto& sub : draw.gameObj->subMeshes) {
			if (!sub.visible || !sub.mesh_->isValid()) continue;
			sub.material_->Bind(*m_shader);
			sub.mesh_->Draw(bWireFrame);
		}
	}

	glm::mat4 UnlitPipeline::buildModelMatrix(const TransformComponent& transform)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, transform.position_);
		model = glm::rotate(model, transform.rotation_.x, { 1, 0, 0 });
		model = glm::rotate(model, transform.rotation_.y, { 0, 1, 0 });
		model = glm::rotate(model, transform.rotation_.z, { 0, 0, 1 });
		model = glm::scale(model, transform.scale_);
		return model;
	}

	void UnlitPipeline::resetBlendState()
	{
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}
}