#include "Pengu_Engine/Graphics/Rendering/Techniques/ForwardShad.hpp"
#include "Pengu_Engine/Managers/Resource/ResourceManager.hpp"
#include "Pengu_Engine/Systems/LightSystem.hpp"
#include "Pengu_Engine/Scene/SceneBase.hpp"
#include "Pengu_Engine/Graphics/Shader.hpp"
#include "Pengu_Engine/Camera/Fustrum.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

namespace Pengu::Graphics::Rendering {

	void ForwardShadPipeline::init(Pengu::Resources::ResourceManager& rm)
	{
		aux_rm = &rm;

		m_forwardShader = rm.getShader(
			"../assets/shaders/forwardShad.vert",
			"../assets/shaders/forwardShad.frag");
		m_shadowShader = rm.getShader(
			"../assets/shaders/shadows.vert",
			"../assets/shaders/shadows.frag");
		m_pointShadowShader = rm.getShader(
			"../assets/shaders/point_shadows.vert",
			"../assets/shaders/point_shadows.frag");

		setupDirShadowMap();
		setupSpotShadowMap();
		setupPointShadowMap();
		m_initialized = true;
	}

	void ForwardShadPipeline::cleanup()
	{
		m_forwardShader.reset();
		m_shadowShader.reset();
		m_pointShadowShader.reset();
		destroyLightShadowMap();
		m_initialized = false;
	}

	void ForwardShadPipeline::reload(Pengu::Resources::ResourceManager& rm)
	{
		cleanup();
		init(rm);
	}

	void ForwardShadPipeline::render(Pengu::Scene::SceneBase& scene, Camera& camera)
	{
		if (!m_initialized || !m_forwardShader || !m_shadowShader) return;

		glEnable(GL_DEPTH_TEST);
		m_firstpass = true;
		glEnable(GL_CULL_FACE);

		auto& world = scene.getWorld();

		std::vector<LightData> lights;
		if (auto ls = scene.getLightSystem())
		{
			lights = ls->collectLights(world);
		}

		if (lights.empty())
		{
			glViewport(0, 0, m_width, m_height);
			glCullFace(GL_BACK);
			glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			m_forwardShader->bind();
			m_forwardShader->setMat4("view", camera.getViewMatrix());
			m_forwardShader->setMat4("projection", camera.projection_);
			m_forwardShader->setVec3("viewPos", camera.getPosition());

			for (auto id : scene.getRenderables())
			{
				auto& draw = world.GetComponent<DrawableComponent>(id);
				auto& transform = world.GetComponent<TransformComponent>(id);

				uploadTransform(transform, camera);
				drawSubMeshes(draw);
			}
		}
		else
		{
			for (auto& ld : lights)
			{
				if (ld.light->type_ == 1) // Point light
				{
					pointShadowPass(*ld.light, scene);
				}
				else if (ld.light->type_ == 2)
				{
					ld.light->lightSpaceMatrix = buildLightSpaceMatrix(*ld.light, camera);
					spotShadowPass(*ld.light, scene);
				}
				else {
					ld.light->lightSpaceMatrix = buildLightSpaceMatrix(*ld.light, camera);
					dirShadowPass(*ld.light, scene);
				}

				glViewport(0, 0, m_width, m_height);

				if (m_firstpass)
				{
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
					glDepthFunc(GL_LEQUAL);
				}

				m_forwardShader->bind();
				m_forwardShader->setMat4("view", camera.getViewMatrix());
				m_forwardShader->setMat4("projection", camera.projection_);
				m_forwardShader->setVec3("viewPos", camera.getPosition());
				m_forwardShader->setMat4("lightSpaceMatrix", ld.light->lightSpaceMatrix);
				m_forwardShader->setBool("u_isFirstPass", m_firstpass);

				m_forwardShader->setInt("DirShadowMap", 10);
				glBindTextureUnit(10, m_DirShadowMap);

				m_forwardShader->setInt("SpotShadowMap", 11);
				glBindTextureUnit(11, m_SpotShadowMap);

				m_forwardShader->setInt("PointShadowMap", 12);
				glBindTextureUnit(12, m_PointShadowCubeMap);

				bindLight(*ld.light);

				for (auto id : scene.getRenderables())
				{
					auto& draw = world.GetComponent<DrawableComponent>(id);
					auto& transform = world.GetComponent<TransformComponent>(id);
					uploadTransform(transform, camera);
					drawSubMeshes(draw);
				}
			}
		}
		resetBlendState();
	}

	void ForwardShadPipeline::destroyLightShadowMap()
	{
		if (m_DirShadowFBO) { glDeleteFramebuffers(1, &m_DirShadowFBO); m_DirShadowFBO = 0; }
		if (m_DirShadowMap) { glDeleteTextures(1, &m_DirShadowMap); m_DirShadowMap = 0; }
		if (m_SpotShadowFBO) { glDeleteFramebuffers(1, &m_SpotShadowFBO); m_SpotShadowFBO = 0; }
		if (m_SpotShadowMap) { glDeleteTextures(1, &m_SpotShadowMap); m_SpotShadowMap = 0; }
		if (m_PointShadowFBO) { glDeleteFramebuffers(1, &m_PointShadowFBO); m_PointShadowFBO = 0; }
		if (m_PointShadowCubeMap) { glDeleteTextures(1, &m_PointShadowCubeMap); m_PointShadowCubeMap = 0; }
	}

	void ForwardShadPipeline::dirShadowPass(Lights& light, Pengu::Scene::SceneBase& scene)
	{
		glCullFace(GL_FRONT);
		glBindFramebuffer(GL_FRAMEBUFFER, m_DirShadowFBO);
		glViewport(0, 0, m_dirShadowResolution, m_dirShadowResolution);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glClear(GL_DEPTH_BUFFER_BIT);

		m_shadowShader->bind();
		m_shadowShader->setMat4("lightSpaceMatrix", light.lightSpaceMatrix);

		auto& world = scene.getWorld();
		for (auto id : scene.getRenderables()) {

			if (!world.HasComponent<DrawableComponent>(id) || !world.HasComponent<TransformComponent>(id)) {
				continue;
			}

			if (world.HasComponent<TagComponent>(id)) {
				if (world.GetComponent<TagComponent>(id).tag == "Floor") {
					continue;
				}
			}

			auto& draw = world.GetComponent<DrawableComponent>(id);
			auto& transform = world.GetComponent<TransformComponent>(id);
			m_shadowShader->setMat4("model", buildModelMatrix(transform));
			drawSubMeshesShadow(draw);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glCullFace(GL_BACK);
	}

	void ForwardShadPipeline::spotShadowPass(Lights& light, Pengu::Scene::SceneBase& scene)
	{
		glCullFace(GL_FRONT);
		glBindFramebuffer(GL_FRAMEBUFFER, m_SpotShadowFBO);
		glViewport(0, 0, m_shadowResolution, m_shadowResolution);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glClear(GL_DEPTH_BUFFER_BIT);

		m_shadowShader->bind();
		m_shadowShader->setMat4("lightSpaceMatrix", light.lightSpaceMatrix);

		auto& world = scene.getWorld();
		for (auto id : scene.getRenderables()) {

			if (!world.HasComponent<DrawableComponent>(id) || !world.HasComponent<TransformComponent>(id)) {
				continue;
			}

			if (world.HasComponent<TagComponent>(id)) {
				if (world.GetComponent<TagComponent>(id).tag == "Floor") {
					continue;
				}
			}

			auto& draw = world.GetComponent<DrawableComponent>(id);
			auto& transform = world.GetComponent<TransformComponent>(id);
			m_shadowShader->setMat4("model", buildModelMatrix(transform));
			drawSubMeshesShadow(draw);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glCullFace(GL_BACK);
	}

	void ForwardShadPipeline::setupDirShadowMap()
	{
		glCreateFramebuffers(1, &m_DirShadowFBO);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_DirShadowMap);
		glTextureStorage2D(m_DirShadowMap, 1, GL_DEPTH_COMPONENT24,
			m_dirShadowResolution, m_dirShadowResolution);

		glTextureParameteri(m_DirShadowMap, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_DirShadowMap, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_DirShadowMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTextureParameteri(m_DirShadowMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		float border[] = { 1, 1, 1, 1 };
		glTextureParameterfv(m_DirShadowMap, GL_TEXTURE_BORDER_COLOR, border);

		glNamedFramebufferTexture(m_DirShadowFBO,
			GL_DEPTH_ATTACHMENT, m_DirShadowMap, 0);
		glNamedFramebufferDrawBuffer(m_DirShadowFBO, GL_NONE);
		glNamedFramebufferReadBuffer(m_DirShadowFBO, GL_NONE);

		if (glCheckNamedFramebufferStatus(m_DirShadowFBO, GL_FRAMEBUFFER)
			!= GL_FRAMEBUFFER_COMPLETE)
			LOG_ERROR("RenderSystem Shadow framebuffer not complete!");
	}

	glm::mat4 ForwardShadPipeline::buildLightSpaceMatrix(const Lights& ld, const Camera& cam)
	{
		glm::mat4 proj = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		if (ld.type_ == 0) {
			float size = 40.0f;
			proj = glm::ortho(-size, size, -size, size, cam.getNear(), cam.getFar());

			glm::vec3 pos = -ld.direction_ * 50.0f;
			glm::vec3 target = pos + ld.direction_;

			glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
			if (glm::abs(glm::dot(glm::normalize(ld.direction_), up)) > 0.99f) {
				up = glm::vec3(0.0f, 0.0f, 1.0f);
			}

			view = glm::lookAt(pos, target, up);
		}
		else if (ld.type_ == 2)
		{
			float fov = glm::radians(ld.outerCutoff_) * 2.0f;
			proj = glm::perspective(fov, 1.0f, 0.1f, cam.getFar());

			glm::vec3 pos = ld.position_;
			glm::vec3 target = pos + ld.direction_;

			glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
			if (glm::abs(glm::dot(glm::normalize(ld.direction_), up)) > 0.99f) {
				up = glm::vec3(0.0f, 0.0f, 1.0f);
			}

			view = glm::lookAt(pos, target, up);
		}
		return proj * view;
	}

	void ForwardShadPipeline::uploadTransform(const TransformComponent& transform, Camera& cam)
	{
		glm::mat4 model = buildModelMatrix(transform);
		glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(model)));
		m_forwardShader->setMat4("model", model);
		m_forwardShader->setMat4("projection", cam.projection_);
		m_forwardShader->setMat3("normalMatrix", normalMat);
	}

	void ForwardShadPipeline::onResize(int width, int height)
	{
		m_width = width;
		m_height = height;

		reload(*aux_rm);
	}

	void ForwardShadPipeline::bindLight(const Lights& light)
	{
		m_forwardShader->setInt("light.light_type", light.type_);
		m_forwardShader->setVec3("light.color", light.color_);
		m_forwardShader->setVec3("light.position", light.position_);
		m_forwardShader->setVec3("light.direction", light.direction_);
		m_forwardShader->setFloat("light.intensity", light.intensity_);
		m_forwardShader->setFloat("light.ambientStrength", light.ambiStr_);
		m_forwardShader->setFloat("light.specularStrength", light.specStr_);
		m_forwardShader->setFloat("light.shininess", light.shin_);
		m_forwardShader->setFloat("light.radius", light.radius_);
		m_forwardShader->setFloat("light.cutoff", glm::cos(glm::radians(light.innerCutoff_)));
		m_forwardShader->setFloat("light.outerCutoff", glm::cos(glm::radians(light.outerCutoff_)));
		m_forwardShader->setFloat("light.constant", light.constant_);
		m_forwardShader->setFloat("light.linear", light.linear_);
		m_forwardShader->setFloat("light.quadratic", light.quadratic_);
		m_forwardShader->setFloat("light.shadowSoftness", light.shadowSoftness_);
		m_forwardShader->setFloat("far_plane", light.radius_);
	}

	glm::mat4 ForwardShadPipeline::buildModelMatrix(const TransformComponent& transform)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, transform.position_);
		model = model * glm::eulerAngleYXZ(
			glm::radians(transform.rotation_.y),
			glm::radians(transform.rotation_.x),
			glm::radians(transform.rotation_.z)
		);
		model = glm::scale(model, transform.scale_);
		return model;
	}

	void ForwardShadPipeline::drawSubMeshesShadow(const DrawableComponent& draw)
	{
		for (auto& sub : draw.gameObj->subMeshes) {
			if (!sub.visible || !sub.mesh_->isValid()) continue;
			sub.mesh_->Draw(bWireFrame);
		}
	}

	void ForwardShadPipeline::drawSubMeshes(const DrawableComponent& draw)
	{
		for (auto& sub : draw.gameObj->subMeshes) {
			if (!sub.visible || !sub.mesh_->isValid()) continue;
			sub.material_->Bind(*m_forwardShader);
			sub.mesh_->Draw(bWireFrame);
		}
	}

	void ForwardShadPipeline::setupSpotShadowMap()
	{
		glCreateFramebuffers(1, &m_SpotShadowFBO);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_SpotShadowMap);
		glTextureStorage2D(m_SpotShadowMap, 1, GL_DEPTH_COMPONENT24,
			m_shadowResolution, m_shadowResolution);

		glTextureParameteri(m_SpotShadowMap, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_SpotShadowMap, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_SpotShadowMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTextureParameteri(m_SpotShadowMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		float border[] = { 1, 1, 1, 1 };
		glTextureParameterfv(m_SpotShadowMap, GL_TEXTURE_BORDER_COLOR, border);

		glNamedFramebufferTexture(m_SpotShadowFBO,
			GL_DEPTH_ATTACHMENT, m_SpotShadowMap, 0);
		glNamedFramebufferDrawBuffer(m_SpotShadowFBO, GL_NONE);
		glNamedFramebufferReadBuffer(m_SpotShadowFBO, GL_NONE);

		if (glCheckNamedFramebufferStatus(m_SpotShadowFBO, GL_FRAMEBUFFER)
			!= GL_FRAMEBUFFER_COMPLETE)
			LOG_ERROR("RenderSystem Shadow framebuffer not complete!");
	}

	void ForwardShadPipeline::setupPointShadowMap()
	{
		glCreateFramebuffers(1, &m_PointShadowFBO);

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_PointShadowCubeMap);
		glTextureStorage2D(m_PointShadowCubeMap, 1, GL_DEPTH_COMPONENT32F, m_shadowResolution, m_shadowResolution);

		glTextureParameteri(m_PointShadowCubeMap, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_PointShadowCubeMap, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_PointShadowCubeMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_PointShadowCubeMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_PointShadowCubeMap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glNamedFramebufferTexture(m_PointShadowFBO, GL_DEPTH_ATTACHMENT, m_PointShadowCubeMap, 0);
		glNamedFramebufferDrawBuffer(m_PointShadowFBO, GL_NONE);
		glNamedFramebufferReadBuffer(m_PointShadowFBO, GL_NONE);

		if (glCheckNamedFramebufferStatus(m_PointShadowFBO, GL_FRAMEBUFFER)
			!= GL_FRAMEBUFFER_COMPLETE)
			LOG_ERROR("Point Shadow framebuffer not complete!");
	}

	void ForwardShadPipeline::pointShadowPass(Lights& light, Pengu::Scene::SceneBase& scene)
	{
		glViewport(0, 0, m_shadowResolution, m_shadowResolution);
		glBindFramebuffer(GL_FRAMEBUFFER, m_PointShadowFBO);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glCullFace(GL_FRONT);

		auto shadowMatrices = buildPointLightMatrices(light);
		m_pointShadowShader->bind();
		m_pointShadowShader->setVec3("lightPos", light.position_);
		m_pointShadowShader->setFloat("far_plane", light.radius_);

		auto& world = scene.getWorld();

		for (int i = 0; i < 6; ++i)
		{
			glNamedFramebufferTextureLayer(m_PointShadowFBO, GL_DEPTH_ATTACHMENT, m_PointShadowCubeMap, 0, i);
			glClear(GL_DEPTH_BUFFER_BIT);
			m_pointShadowShader->setMat4("lightSpaceMatrix", shadowMatrices[i]);

			for (auto id : scene.getRenderables())
			{
				if (!world.HasComponent<DrawableComponent>(id) || !world.HasComponent<TransformComponent>(id)) continue;
				auto& draw = world.GetComponent<DrawableComponent>(id);
				auto& transform = world.GetComponent<TransformComponent>(id);
				m_pointShadowShader->setMat4("model", buildModelMatrix(transform));
				drawSubMeshesShadow(draw);
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glCullFace(GL_BACK);
	}

	std::vector<glm::mat4> ForwardShadPipeline::buildPointLightMatrices(const Lights& ld)
	{
		float aspect = 1.0f;
		float near = 0.1f;
		float far = ld.radius_;
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);

		std::vector<glm::mat4> shadowMatrices;
		shadowMatrices.push_back(shadowProj * glm::lookAt(ld.position_, ld.position_ + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowMatrices.push_back(shadowProj * glm::lookAt(ld.position_, ld.position_ + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowMatrices.push_back(shadowProj * glm::lookAt(ld.position_, ld.position_ + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		shadowMatrices.push_back(shadowProj * glm::lookAt(ld.position_, ld.position_ + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		shadowMatrices.push_back(shadowProj * glm::lookAt(ld.position_, ld.position_ + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowMatrices.push_back(shadowProj * glm::lookAt(ld.position_, ld.position_ + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

		return shadowMatrices;
	}

	void ForwardShadPipeline::resetBlendState()
	{
		m_forwardShader->unbind();
		m_shadowShader->unbind();
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}

}