#include "Pengu_Engine/Graphics/Rendering/Techniques/Forward.hpp"
#include "Pengu_Engine/Managers/Resource/ResourceManager.hpp"
#include "Pengu_Engine/Systems/LightSystem.hpp"
#include "Pengu_Engine/Scene/SceneBase.hpp"
#include "Pengu_Engine/Graphics/Shader.hpp"

namespace Pengu::Graphics::Rendering {

	void ForwardPipeline::init(Pengu::Resources::ResourceManager& rm)
	{
		aux_rm = &rm;

		m_shader = rm.getShader("../assets/shaders/forward.vert",
			"../assets/shaders/forward.frag");

		m_initialized = true;
	}

	void ForwardPipeline::cleanup()
	{
		m_shader.reset();
		m_initialized = false;
	}

	void ForwardPipeline::reload(Pengu::Resources::ResourceManager& rm)
	{
		cleanup();
		init(rm);
	}

	void ForwardPipeline::render(Pengu::Scene::SceneBase& scene, Camera& camera)
	{
		if (!m_initialized || !m_shader) return;

		m_firstpass = true;
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		auto& world = scene.getWorld();

		std::vector<LightData> lights;
		if (auto ls = scene.getLightSystem())
		{
			lights = ls->collectLights(world);
		}

		m_shader->bind();
		m_shader->setMat4("view", camera.getViewMatrix());
		m_shader->setMat4("projection", camera.projection_);
		m_shader->setVec3("viewPos", camera.getPosition());

		if (lights.empty()) {

			glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			for (auto id : scene.getRenderables()) {
				auto& draw = world.GetComponent<DrawableComponent>(id);
				auto& transform = world.GetComponent<TransformComponent>(id);

				m_shader->setMat4("model", buildModelMatrix(transform));
				drawSubMeshes(draw);
			}
		}
		else
		{

			for (auto& ld : lights)
			{

				if (m_firstpass) {
					glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					glDepthFunc(GL_LESS);
					glDepthMask(GL_TRUE);
					glDisable(GL_BLEND);
					m_firstpass = false;
				}
				else
				{
					glEnable(GL_BLEND);
					glBlendFunc(GL_ONE, GL_ONE);
					glBlendEquation(GL_FUNC_ADD);
					glDepthMask(GL_FALSE);
					glDepthFunc(GL_EQUAL);
				}

				bindLight(*ld.light);

				for (auto id : scene.getRenderables()) {
					auto& draw = world.GetComponent<DrawableComponent>(id);
					auto& transform = world.GetComponent<TransformComponent>(id);

					m_shader->setMat4("model", buildModelMatrix(transform));
					drawSubMeshes(draw);
				}
			}
		}
		resetBlendState();
	}

	void ForwardPipeline::bindLight(const Lights& light)
	{
		m_shader->setInt("light.light_type", light.type_);
		m_shader->setVec3("light.color", light.color_);
		m_shader->setVec3("light.position", light.position_);
		m_shader->setVec3("light.direction", light.direction_);
		m_shader->setFloat("light.ambientStrength", light.ambiStr_);
		m_shader->setFloat("light.specularStrength", light.specStr_);
		m_shader->setFloat("light.shininess", light.shin_);
		m_shader->setFloat("light.radius", light.radius_);
		m_shader->setFloat("light.cutoff", light.innerCutoff_);
		m_shader->setFloat("light.outerCutoff", light.outerCutoff_);
		m_shader->setFloat("light.constant", light.constant_);
		m_shader->setFloat("light.linear", light.linear_);
		m_shader->setFloat("light.quadratic", light.quadratic_);
	}

	void ForwardPipeline::onResize(int width, int height)
	{
		m_width = width;
		m_height = height;

		reload(*aux_rm);
	}

	glm::mat4 ForwardPipeline::buildModelMatrix(const TransformComponent& transform)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, transform.position_);
		model = glm::rotate(model, transform.rotation_.x, { 1, 0, 0 });
		model = glm::rotate(model, transform.rotation_.y, { 0, 1, 0 });
		model = glm::rotate(model, transform.rotation_.z, { 0, 0, 1 });
		model = glm::scale(model, transform.scale_);
		return model;
	}

	void ForwardPipeline::drawSubMeshes(const DrawableComponent& draw)
	{
		for (auto& sub : draw.gameObj->subMeshes) {
			if (!sub.visible || !sub.mesh_->isValid()) continue;
			sub.material_->Bind(*m_shader);
			sub.mesh_->Draw(bWireFrame);
		}
	}

	void ForwardPipeline::resetBlendState()
	{
		m_shader->unbind();
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}
}