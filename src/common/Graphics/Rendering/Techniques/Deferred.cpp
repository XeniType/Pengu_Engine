#include "Pengu_Engine/Graphics/Rendering/Techniques/Deferred.hpp"
#include "Pengu_Engine/Managers/Resource/ResourceManager.hpp"
#include "Pengu_Engine/Graphics/GameObject.hpp"
#include "Pengu_Engine/Systems/LightSystem.hpp"
#include "Pengu_Engine/Scene/SceneBase.hpp"
#include "Pengu_Engine/Graphics/Shader.hpp"
#include "Pengu_Engine/Components/TerrainComponent.hpp"
#include "Pengu_Engine/Components/WaterComponent.hpp"
#include "Pengu_Engine/Camera/Fustrum.hpp"
#include "Pengu_Engine/Misc/Logmacros.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>


void Pengu::Graphics::Rendering::DeferredPipeline::init(Pengu::Resources::ResourceManager& rm)
{
	aux_rm = &rm;

	m_bufferShad = aux_rm->getShader("../assets/shaders/deferred/buffer.vert",
		"../assets/shaders/deferred/buffer.frag");

	m_deferredShad = aux_rm->getShader("../assets/shaders/deferred/deferred.vert",
		"../assets/shaders/deferred/deferred.frag");

	m_terrainShad = aux_rm->getShader("../assets/shaders/deferred/terrain.vert",
		"../assets/shaders/deferred/terrain.frag");

	m_skyboxShad = aux_rm->getShader("../assets/shaders/skyboxShad.vert",
		"../assets/shaders/skyboxShad.frag");

	m_waterShad = aux_rm->getShader("../assets/shaders/deferred/water.vert",
		"../assets/shaders/deferred/water.frag");

	m_shadowShad = aux_rm->getShader("../assets/shaders/shadows.vert",
		"../assets/shaders/shadows.frag");

	m_pointShadowShader = rm.getShader("../assets/shaders/point_shadows.vert",
		"../assets/shaders/point_shadows.frag");

	setupBuffer();
	setupQuad();
	setupShadowFBO();
	setupPointShadowMap();

	m_initialized = true;
}

void Pengu::Graphics::Rendering::DeferredPipeline::cleanup()
{
	deleteBuffers();
	m_bufferShad.reset();
	m_deferredShad.reset();
	m_terrainShad.reset();
	m_skyboxShad.reset();
	m_waterShad.reset();
	m_shadowShad.reset();
	m_initialized = false;
}

void Pengu::Graphics::Rendering::DeferredPipeline::reload(Pengu::Resources::ResourceManager& rm)
{
	cleanup();
	init(rm);
}

void Pengu::Graphics::Rendering::DeferredPipeline::onPreRender(Pengu::Scene::SceneBase& scene, Camera& camera)
{
	shadowPass(scene, camera);
	geometryPass(scene, camera);
}

void Pengu::Graphics::Rendering::DeferredPipeline::render(Pengu::Scene::SceneBase& scene, Camera& camera)
{
	GLint targetFBO = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &targetFBO);

	lightPass(scene, camera);
	if (scene.getWorld().IsComponentRegistered<Pengu::Components::WaterComponent>())
	{
		waterPass(scene, camera, (unsigned int)targetFBO);

	}
	renderQuad();
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_gBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);
	glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);

	if (scene.HasSkyBox())
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		m_skyboxShad->bind();
		m_skyboxShad->setVec3("fogColor", scene.GetSkyBox().fogColor);
		scene.GetSkyBox().Render(*m_skyboxShad, camera.getViewMatrix(), camera.projection_);
	}
	if (bWireFrame) {
		glm::mat4 viewProj = camera.projection_ * camera.getViewMatrix();
		Frustum cameraFrustum = ExtractFrustum(viewProj);

		if (scene.getWorld().IsComponentRegistered<TerrainSettingsComponent>())
			terrainPass(scene, camera, cameraFrustum);

		m_bufferShad->bind();
		m_bufferShad->setMat4("projection", camera.projection_);
		m_bufferShad->setMat4("view", camera.getViewMatrix());

		for (auto obj : scene.getRenderables()) {
			auto& object = scene.getWorld().GetComponent<DrawableComponent>(obj);
			auto& transform = scene.getWorld().GetComponent<TransformComponent>(obj);

			if (object.gameObj->bVisible) {
				m_bufferShad->setMat4("model", buildModelMatrix(transform));
				m_bufferShad->setBool("DrawNormals", bDrawNorms);
				m_bufferShad->setBool("DrawUV", bDrawUV);
				drawSubMeshes(object.gameObj, *m_bufferShad);
			}
		}
	}
}

void Pengu::Graphics::Rendering::DeferredPipeline::onResize(int width, int height)
{
	m_width = width;
	m_height = height;

	reload(*aux_rm);
}

void Pengu::Graphics::Rendering::DeferredPipeline::setupBuffer()
{
	glCreateFramebuffers(1, &m_gBuffer);

	/* -- Position Color Buffer */
	glCreateTextures(GL_TEXTURE_2D, 1, &m_gPosition);
	glTextureStorage2D(m_gPosition, 1, GL_RGBA16F, m_width, m_height);
	glTextureParameteri(m_gPosition, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_gPosition, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glNamedFramebufferTexture(m_gBuffer, GL_COLOR_ATTACHMENT0, m_gPosition, 0);

	/* -- Normal Color Buffer */
	glCreateTextures(GL_TEXTURE_2D, 1, &m_gNormal);
	glTextureStorage2D(m_gNormal, 1, GL_RGBA16F, m_width, m_height);
	glTextureParameteri(m_gNormal, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_gNormal, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glNamedFramebufferTexture(m_gBuffer, GL_COLOR_ATTACHMENT1, m_gNormal, 0);

	/* -- Color Color Buffer */
	glCreateTextures(GL_TEXTURE_2D, 1, &m_gColorSpec);
	glTextureStorage2D(m_gColorSpec, 1, GL_RGBA8, m_width, m_height);
	glTextureParameteri(m_gColorSpec, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_gColorSpec, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glNamedFramebufferTexture(m_gBuffer, GL_COLOR_ATTACHMENT2, m_gColorSpec, 0);

	/* -- PBR Color Buffer (Roughness, Metallic, AO) */
	glCreateTextures(GL_TEXTURE_2D, 1, &m_gPBR);
	glTextureStorage2D(m_gPBR, 1, GL_RGBA8, m_width, m_height);
	glTextureParameteri(m_gPBR, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_gPBR, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glNamedFramebufferTexture(m_gBuffer, GL_COLOR_ATTACHMENT3, m_gPBR, 0);

	unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glNamedFramebufferDrawBuffers(m_gBuffer, 4, attachments);

	glCreateRenderbuffers(1, &m_rboDepth);
	glNamedRenderbufferStorage(m_rboDepth, GL_DEPTH_COMPONENT24, m_width, m_height);
	glNamedFramebufferRenderbuffer(m_gBuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rboDepth);

	if (glCheckNamedFramebufferStatus(m_gBuffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		ENGINE_ERROR("DeferredPipeline: G-Buffer Framebuffer not complete!");

	/* -- Lit Scene Buffer */
	glCreateFramebuffers(1, &m_litFBO);
	glCreateTextures(GL_TEXTURE_2D, 1, &m_sceneColor);
	glTextureStorage2D(m_sceneColor, 1, GL_RGBA16F, m_width, m_height);
	glTextureParameteri(m_sceneColor, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_sceneColor, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glNamedFramebufferTexture(m_litFBO, GL_COLOR_ATTACHMENT0, m_sceneColor, 0);

	if (glCheckNamedFramebufferStatus(m_litFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		ENGINE_ERROR("DeferredPipeline: Lit Framebuffer not complete!");
}

void Pengu::Graphics::Rendering::DeferredPipeline::deleteBuffers()
{
	if (m_gBuffer) glDeleteFramebuffers(1, &m_gBuffer);
	if (m_gPosition) glDeleteTextures(1, &m_gPosition);
	if (m_gNormal) glDeleteTextures(1, &m_gNormal);
	if (m_gColorSpec) glDeleteTextures(1, &m_gColorSpec);
	if (m_gPBR) glDeleteTextures(1, &m_gPBR);
	if (m_rboDepth) glDeleteRenderbuffers(1, &m_rboDepth);

	if (m_litFBO) glDeleteFramebuffers(1, &m_litFBO);
	if (m_sceneColor) glDeleteTextures(1, &m_sceneColor);

	if (m_quadVAO) glDeleteVertexArrays(1, &m_quadVAO);
	if (m_quadVBO) glDeleteBuffers(1, &m_quadVBO);

	for (int i = 0; i < 3; ++i)
	{
		if (cascadeFBOs[i]) glDeleteFramebuffers(1, &cascadeFBOs[i]);
		if (cascadeMaps[i]) glDeleteTextures(1, &cascadeMaps[i]);
		cascadeFBOs[i] = 0;
		cascadeMaps[i] = 0;
	}

	if (m_SpotShadowFBO) glDeleteFramebuffers(1, &m_SpotShadowFBO);
	if (m_SpotShadowMap) glDeleteTextures(1, &m_SpotShadowMap);

	if (m_PointShadowFBO) glDeleteFramebuffers(1, &m_PointShadowFBO);
	if (m_PointShadowCubeMap) glDeleteTextures(1, &m_PointShadowCubeMap);

	m_gBuffer = 0;
	m_gPosition = 0;
	m_gNormal = 0;
	m_gColorSpec = 0;
	m_gPBR = 0;
	m_rboDepth = 0;
	m_litFBO = 0;
	m_sceneColor = 0;
	m_quadVAO = 0;
	m_quadVBO = 0;
	m_SpotShadowFBO = 0;
	m_SpotShadowMap = 0;
}

void Pengu::Graphics::Rendering::DeferredPipeline::setupQuad()
{
	float quadVertices[] = {
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

	glCreateVertexArrays(1, &m_quadVAO);
	glCreateBuffers(1, &m_quadVBO);
	glNamedBufferData(m_quadVBO, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glVertexArrayVertexBuffer(m_quadVAO, 0, m_quadVBO, 0, 5 * sizeof(float));
	glEnableVertexArrayAttrib(m_quadVAO, 0);
	glVertexArrayAttribFormat(m_quadVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(m_quadVAO, 0, 0);
	glEnableVertexArrayAttrib(m_quadVAO, 1);
	glVertexArrayAttribFormat(m_quadVAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
	glVertexArrayAttribBinding(m_quadVAO, 1, 0);
}

void Pengu::Graphics::Rendering::DeferredPipeline::renderQuad()
{
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void Pengu::Graphics::Rendering::DeferredPipeline::setupShadowFBO()
{
	/* Generating Directional Ligth Shadow FBO */
	/* Cascade 1 */

	glCreateFramebuffers(1, &cascadeFBOs[0]);

	glCreateTextures(GL_TEXTURE_2D, 1, &cascadeMaps[0]);
	glTextureStorage2D(cascadeMaps[0], 1, GL_DEPTH_COMPONENT24,
		m_shadowDirResolution, m_shadowDirResolution);

	glTextureParameteri(cascadeMaps[0], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(cascadeMaps[0], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(cascadeMaps[0], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(cascadeMaps[0], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float border[] = { 1, 1, 1, 1 };
	glTextureParameterfv(cascadeMaps[0], GL_TEXTURE_BORDER_COLOR, border);

	glNamedFramebufferTexture(cascadeFBOs[0],
		GL_DEPTH_ATTACHMENT, cascadeMaps[0], 0);
	glNamedFramebufferDrawBuffer(cascadeFBOs[0], GL_NONE);
	glNamedFramebufferReadBuffer(cascadeFBOs[0], GL_NONE);

	if (glCheckNamedFramebufferStatus(cascadeFBOs[0], GL_FRAMEBUFFER)
		!= GL_FRAMEBUFFER_COMPLETE)
		LOG_ERROR("RenderSystem Shadow framebuffer not complete!");

	/* Cascade 2 */

	glCreateFramebuffers(1, &cascadeFBOs[1]);

	glCreateTextures(GL_TEXTURE_2D, 1, &cascadeMaps[1]);
	glTextureStorage2D(cascadeMaps[1], 1, GL_DEPTH_COMPONENT24,
		m_shadowDirResolution, m_shadowDirResolution);

	glTextureParameteri(cascadeMaps[1], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(cascadeMaps[1], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(cascadeMaps[1], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(cascadeMaps[1], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glTextureParameterfv(cascadeMaps[1], GL_TEXTURE_BORDER_COLOR, border);

	glNamedFramebufferTexture(cascadeFBOs[1],
		GL_DEPTH_ATTACHMENT, cascadeMaps[1], 0);
	glNamedFramebufferDrawBuffer(cascadeFBOs[1], GL_NONE);
	glNamedFramebufferReadBuffer(cascadeFBOs[1], GL_NONE);

	if (glCheckNamedFramebufferStatus(cascadeFBOs[1], GL_FRAMEBUFFER)
		!= GL_FRAMEBUFFER_COMPLETE)
		LOG_ERROR("RenderSystem Shadow framebuffer not complete!");

	/* Cascade 3 */

	glCreateFramebuffers(1, &cascadeFBOs[2]);

	glCreateTextures(GL_TEXTURE_2D, 1, &cascadeMaps[2]);
	glTextureStorage2D(cascadeMaps[2], 1, GL_DEPTH_COMPONENT24,
		m_shadowDirResolution, m_shadowDirResolution);

	glTextureParameteri(cascadeMaps[2], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(cascadeMaps[2], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(cascadeMaps[2], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(cascadeMaps[2], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glTextureParameterfv(cascadeMaps[2], GL_TEXTURE_BORDER_COLOR, border);

	glNamedFramebufferTexture(cascadeFBOs[2],
		GL_DEPTH_ATTACHMENT, cascadeMaps[2], 0);
	glNamedFramebufferDrawBuffer(cascadeFBOs[2], GL_NONE);
	glNamedFramebufferReadBuffer(cascadeFBOs[2], GL_NONE);

	if (glCheckNamedFramebufferStatus(cascadeFBOs[2], GL_FRAMEBUFFER)
		!= GL_FRAMEBUFFER_COMPLETE)
		LOG_ERROR("RenderSystem Shadow framebuffer not complete!");

	/* Generating Spot Ligth Shadow FBO */

	glCreateFramebuffers(1, &m_SpotShadowFBO);

	glCreateTextures(GL_TEXTURE_2D, 1, &m_SpotShadowMap);
	glTextureStorage2D(m_SpotShadowMap, 1, GL_DEPTH_COMPONENT24,
		m_shadowSpotResolution, m_shadowSpotResolution);

	glTextureParameteri(m_SpotShadowMap, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_SpotShadowMap, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_SpotShadowMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(m_SpotShadowMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glTextureParameterfv(m_SpotShadowMap, GL_TEXTURE_BORDER_COLOR, border);

	glNamedFramebufferTexture(m_SpotShadowFBO,
		GL_DEPTH_ATTACHMENT, m_SpotShadowMap, 0);
	glNamedFramebufferDrawBuffer(m_SpotShadowFBO, GL_NONE);
	glNamedFramebufferReadBuffer(m_SpotShadowFBO, GL_NONE);

	if (glCheckNamedFramebufferStatus(m_SpotShadowFBO, GL_FRAMEBUFFER)
		!= GL_FRAMEBUFFER_COMPLETE)
		LOG_ERROR("RenderSystem Shadow framebuffer not complete!");

}

void Pengu::Graphics::Rendering::DeferredPipeline::setupPointShadowMap()
{
	glCreateFramebuffers(1, &m_PointShadowFBO);

	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_PointShadowCubeMap);
	glTextureStorage2D(m_PointShadowCubeMap, 1, GL_DEPTH_COMPONENT32F, m_shadowSpotResolution, m_shadowSpotResolution);

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

void Pengu::Graphics::Rendering::DeferredPipeline::bindLight(const Lights& light)
{
	m_deferredShad->setInt("light.light_type", light.type_);
	m_deferredShad->setVec3("light.color", light.color_);
	m_deferredShad->setVec3("light.position", light.position_);
	m_deferredShad->setVec3("light.direction", light.direction_);
	m_deferredShad->setFloat("light.intensity", light.intensity_);
	m_deferredShad->setFloat("light.ambientStrength", light.ambiStr_);
	m_deferredShad->setFloat("light.specularStrength", light.specStr_);
	m_deferredShad->setFloat("light.shininess", light.shin_);
	m_deferredShad->setFloat("light.radius", light.radius_);
	m_deferredShad->setFloat("light.cutoff", glm::cos(glm::radians(light.innerCutoff_)));
	m_deferredShad->setFloat("light.outerCutoff", glm::cos(glm::radians(light.outerCutoff_)));
	m_deferredShad->setFloat("light.constant", light.constant_);
	m_deferredShad->setFloat("light.linear", light.linear_);
	m_deferredShad->setFloat("light.quadratic", light.quadratic_);
	m_deferredShad->setFloat("light.shadowSoftness", light.shadowSoftness_);
	m_deferredShad->setFloat("far_plane", light.radius_);
}

void Pengu::Graphics::Rendering::DeferredPipeline::shadowPass(Pengu::Scene::SceneBase& scene, Camera& camera)
{

	if (!m_shadowShad) return;

	auto& world = scene.getWorld();
	std::vector<LightData> lights;
	if (auto ls = scene.getLightSystem()) lights = ls->collectLights(world);

	for (auto& ld : lights) {
		constructLightMatrix(ld, camera);

		if (ld.light->type_ == 0)
		{
			glViewport(0, 0, m_shadowDirResolution, m_shadowDirResolution);
			for (int i = 0; i < 3; ++i)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, cascadeFBOs[i]);
				glClear(GL_DEPTH_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);

				m_shadowShad->bind();
				m_shadowShad->setMat4("lightSpaceMatrix", ld.light->cascadeSpaceMatrices[i]);

				Frustum cascadeFrustum = ExtractFrustum(ld.light->cascadeSpaceMatrices[i]);

				for (auto obj : scene.getRenderables()) {
					auto& object = scene.getWorld().GetComponent<DrawableComponent>(obj);
					auto& transform = scene.getWorld().GetComponent<TransformComponent>(obj);

					if (object.gameObj->bVisible)
					{
						auto localBounds = object.gameObj->subMeshes[0].mesh_->localBounds;
						auto worldBounds = object.gameObj->subMeshes[0].mesh_->GetWorldBounds(localBounds, transform);

						if (!cascadeFrustum.ContainsSphere(worldBounds.center, worldBounds.radius))
						{
							continue;
						}
					};
					m_shadowShad->setMat4("model", buildModelMatrix(transform));
					for (auto& sub : object.gameObj->subMeshes) {
						if (!sub.visible || !sub.mesh_->isValid()) continue;
						sub.mesh_->Draw(false);
					}
				}
				if (scene.getWorld().IsComponentRegistered<TerrainSettingsComponent>())
					for (auto obj : scene.getTerrainRenderables()) {
						auto& terrainDraw = scene.getWorld().GetComponent<TerrainDrawableComponent>(obj);
						auto& transform = scene.getWorld().GetComponent<TransformComponent>(obj);

						if (terrainDraw.gameObj->bVisible)
						{
							auto localBounds = terrainDraw.gameObj->subMeshes[0].mesh_->localBounds;
							auto worldBounds = terrainDraw.gameObj->subMeshes[0].mesh_->GetWorldBounds(localBounds, transform);

							if (!cascadeFrustum.ContainsSphere(worldBounds.center, worldBounds.radius))
							{
								continue;
							}
						};

						m_shadowShad->setMat4("model", buildModelMatrix(transform));
						for (auto& sub : terrainDraw.gameObj->subMeshes) {
							if (!sub.visible || !sub.mesh_->isValid()) continue;
							sub.mesh_->Draw(false, terrainDraw.gameObj->currentLOD);
						}
					}
			}
		}
		if (ld.light->type_ == 1) {
			glViewport(0, 0, m_shadowSpotResolution, m_shadowSpotResolution);
			glBindFramebuffer(GL_FRAMEBUFFER, m_PointShadowFBO);
			glDepthFunc(GL_LESS);
			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);

			// Build the 6 matrices (You can reuse your constructPointLightMatrix function here)
			float aspect = 1.0f;
			float nearP = 0.1f;
			float farP = ld.light->radius_;
			glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, nearP, farP);

			std::vector<glm::mat4> shadowMatrices;
			shadowMatrices.push_back(shadowProj * glm::lookAt(ld.light->position_, ld.light->position_ + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowMatrices.push_back(shadowProj * glm::lookAt(ld.light->position_, ld.light->position_ + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowMatrices.push_back(shadowProj * glm::lookAt(ld.light->position_, ld.light->position_ + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
			shadowMatrices.push_back(shadowProj * glm::lookAt(ld.light->position_, ld.light->position_ + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
			shadowMatrices.push_back(shadowProj * glm::lookAt(ld.light->position_, ld.light->position_ + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowMatrices.push_back(shadowProj * glm::lookAt(ld.light->position_, ld.light->position_ + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

			m_pointShadowShader->bind();
			m_pointShadowShader->setVec3("lightPos", ld.light->position_);
			m_pointShadowShader->setFloat("far_plane", ld.light->radius_);

			for (int i = 0; i < 6; ++i)
			{
				glNamedFramebufferTextureLayer(m_PointShadowFBO, GL_DEPTH_ATTACHMENT, m_PointShadowCubeMap, 0, i);
				glClear(GL_DEPTH_BUFFER_BIT);
				m_pointShadowShader->setMat4("lightSpaceMatrix", shadowMatrices[i]);

				for (auto obj : scene.getRenderables()) {
					auto& object = scene.getWorld().GetComponent<DrawableComponent>(obj);
					auto& transform = scene.getWorld().GetComponent<TransformComponent>(obj);

					if (object.gameObj->bVisible) {
						m_pointShadowShader->setMat4("model", buildModelMatrix(transform));
						for (auto& sub : object.gameObj->subMeshes) {
							if (!sub.visible || !sub.mesh_->isValid()) continue;
							sub.mesh_->Draw(false);
						}
					}
				}

				// Note: Don't forget to loop through scene.getTerrainRenderables() here if you want point lights to cast shadows from terrain!
			}
		}
		if (ld.light->type_ == 2)
		{
			glViewport(0, 0, m_shadowSpotResolution, m_shadowSpotResolution);
			glBindFramebuffer(GL_FRAMEBUFFER, m_SpotShadowFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);

			m_shadowShad->bind();
			m_shadowShad->setMat4("lightSpaceMatrix", ld.light->lightSpaceMatrix);

			Frustum spotFrustum = ExtractFrustum(ld.light->lightSpaceMatrix);

			for (auto obj : scene.getRenderables()) {
				auto& object = scene.getWorld().GetComponent<DrawableComponent>(obj);
				auto& transform = scene.getWorld().GetComponent<TransformComponent>(obj);

				if (object.gameObj->bVisible) {
					auto localBounds = object.gameObj->subMeshes[0].mesh_->localBounds;
					auto worldBounds = object.gameObj->subMeshes[0].mesh_->GetWorldBounds(localBounds, transform);

					if (!spotFrustum.ContainsSphere(worldBounds.center, worldBounds.radius))
					{
						continue;
					}

					m_shadowShad->setMat4("model", buildModelMatrix(transform));
					for (auto& sub : object.gameObj->subMeshes) {
						if (!sub.visible || !sub.mesh_->isValid()) continue;
						sub.mesh_->Draw(false);
					}
				}
			}
			if (scene.getWorld().IsComponentRegistered<TerrainSettingsComponent>())
				for (auto obj : scene.getTerrainRenderables()) {
					auto& terrainDraw = scene.getWorld().GetComponent<TerrainDrawableComponent>(obj);
					if (!terrainDraw.gameObj->bVisible) continue;
					auto& transform = scene.getWorld().GetComponent<TransformComponent>(obj);

					auto localBounds = terrainDraw.gameObj->subMeshes[0].mesh_->localBounds;
					auto worldBounds = terrainDraw.gameObj->subMeshes[0].mesh_->GetWorldBounds(localBounds, transform);

					if (!spotFrustum.ContainsSphere(worldBounds.center, worldBounds.radius))
					{
						continue;
					}

					m_shadowShad->setMat4("model", buildModelMatrix(transform));
					for (auto& sub : terrainDraw.gameObj->subMeshes) {
						if (!sub.visible || !sub.mesh_->isValid()) continue;
						sub.mesh_->Draw(false, terrainDraw.gameObj->currentLOD);
					}
				}
		}

		glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void Pengu::Graphics::Rendering::DeferredPipeline::geometryPass(Pengu::Scene::SceneBase& scene, Camera& camera)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
	glViewport(0, 0, m_width, m_height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 viewProj = camera.projection_ * camera.getViewMatrix();
	Frustum cameraFrustum = ExtractFrustum(viewProj);

	if (scene.getWorld().IsComponentRegistered<TerrainSettingsComponent>())
	{
		terrainPass(scene, camera, cameraFrustum);
	}

	m_bufferShad->bind();
	m_bufferShad->setMat4("projection", camera.projection_);
	m_bufferShad->setMat4("view", camera.getViewMatrix());

	for (auto obj : scene.getRenderables()) {
		auto& object = scene.getWorld().GetComponent<DrawableComponent>(obj);
		auto& transform = scene.getWorld().GetComponent<TransformComponent>(obj);

		if (object.gameObj->bVisible)
		{
			auto localBounds = object.gameObj->subMeshes[0].mesh_->localBounds;

			auto worldBounds = object.gameObj->subMeshes[0].mesh_->GetWorldBounds(localBounds, transform);

			if (!cameraFrustum.ContainsSphere(worldBounds.center, worldBounds.radius))
			{
				continue;
			}
		};
		m_bufferShad->setMat4("model", buildModelMatrix(transform));
		m_bufferShad->setBool("DrawNormals", bDrawNorms);
		m_bufferShad->setBool("DrawUV", bDrawUV);
		drawSubMeshes(object.gameObj, *m_bufferShad);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Pengu::Graphics::Rendering::DeferredPipeline::lightPass(Pengu::Scene::SceneBase& scene, Camera& camera)
{
	if (!m_initialized || !m_deferredShad) return;

	glBindFramebuffer(GL_FRAMEBUFFER, m_litFBO);
	glViewport(0, 0, m_width, m_height);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	auto& world = scene.getWorld();
	std::vector<LightData> lights;
	if (auto ls = scene.getLightSystem()) lights = ls->collectLights(world);

	m_deferredShad->bind();
	glBindTextureUnit(0, m_gPosition);
	glBindTextureUnit(1, m_gNormal);
	glBindTextureUnit(2, m_gColorSpec);
	glBindTextureUnit(7, m_gPBR);

	glBindTextureUnit(3, cascadeMaps[0]);
	glBindTextureUnit(4, cascadeMaps[1]);
	glBindTextureUnit(5, cascadeMaps[2]);
	glBindTextureUnit(6, m_SpotShadowMap);
	glBindTextureUnit(8, m_PointShadowCubeMap);


	m_deferredShad->setInt("gPosition", 0);
	m_deferredShad->setInt("gNormal", 1);
	m_deferredShad->setInt("gAlbedoAO", 2);
	m_deferredShad->setInt("gPBR", 7);

	m_deferredShad->setInt("DirShadowMapCasc1", 3);
	m_deferredShad->setInt("DirShadowMapCasc2", 4);
	m_deferredShad->setInt("DirShadowMapCasc3", 5);
	m_deferredShad->setInt("SpotShadowMap", 6);
	m_deferredShad->setInt("PointShadowMap", 8);

	m_deferredShad->setVec3("viewPos", camera.getPosition());
	m_deferredShad->setMat4("view", camera.getViewMatrix());
	m_deferredShad->setVec3("fogColor", scene.GetSkyBox().fogColor);
	m_deferredShad->setFloat("fogDensity", scene.GetSkyBox().fogDensity);

	m_firstpass = true;
	for (auto& ld : lights) {
		if (m_firstpass) {
			glDisable(GL_BLEND);
			m_firstpass = false;
		}
		else {
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
		}
		if (ld.light->type_ == 0)
		{
			m_deferredShad->setMat4("lightSpaceMatrices[0]", ld.light->cascadeSpaceMatrices[0]);
			m_deferredShad->setMat4("lightSpaceMatrices[1]", ld.light->cascadeSpaceMatrices[1]);
			m_deferredShad->setMat4("lightSpaceMatrices[2]", ld.light->cascadeSpaceMatrices[2]);

			// Pass the depths so the shader knows when to switch cascades
			m_deferredShad->setFloat("cascadeDepths[0]", ld.light->cascadeSplits[0]);
			m_deferredShad->setFloat("cascadeDepths[1]", ld.light->cascadeSplits[1]);
			m_deferredShad->setFloat("cascadeDepths[2]", ld.light->cascadeSplits[2]);
		}
		else if (ld.light->type_ == 2)
		{
			m_deferredShad->setMat4("lightSpaceMatrix", ld.light->lightSpaceMatrix);
		}

		bindLight(*ld.light);
		renderQuad();
	}
	glDisable(GL_BLEND);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Pengu::Graphics::Rendering::DeferredPipeline::waterPass(Pengu::Scene::SceneBase& scene, Camera& camera, unsigned int targetFBO)
{
	auto waterEntities = scene.getWorld().GetEntitiesWith<Pengu::Components::WaterComponent>();
	if (waterEntities.empty()) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_litFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);
		glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		return;
	}

	// 1. Blit lit scene to the target FBO first
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_litFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);
	glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// 2. Draw water quad over it with blending
	glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
	glViewport(0, 0, m_width, m_height);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_waterShad->bind();
	glBindTextureUnit(0, m_gPosition);
	glBindTextureUnit(1, m_gNormal);
	glBindTextureUnit(2, m_gColorSpec);
	glBindTextureUnit(3, m_sceneColor);
	m_waterShad->setInt("gPosition", 0);
	m_waterShad->setInt("gNormal", 1);
	m_waterShad->setInt("gColorSpec", 2);
	m_waterShad->setInt("sceneColor", 3);

	m_waterShad->setVec3("viewPos", camera.getPosition());
	m_waterShad->setMat4("view", camera.getViewMatrix());
	m_waterShad->setMat4("projection", camera.projection_);
	m_waterShad->setFloat("time", static_cast<float>(glfwGetTime()));

	for (auto entity : waterEntities) {
		auto& water = scene.getWorld().GetComponent<Pengu::Components::WaterComponent>(entity);

		if (water.noiseTexture) glBindTextureUnit(4, water.noiseTexture->getID());
		m_waterShad->setInt("noiseTexture", 4);

		m_waterShad->setFloat("waterHeight", water.height);
		m_waterShad->setVec4("waterColor", water.color);
		m_waterShad->setFloat("waveSpeed", water.waveSpeed);
		m_waterShad->setFloat("waveStrength", water.waveStrength);
		m_waterShad->setFloat("tiling", water.tiling);
		m_waterShad->setFloat("reflectivity", water.reflectivity);
		m_waterShad->setBool("ssrEnabled", water.ssrEnabled);
		m_waterShad->setInt("maxSteps", water.maxStep);
		m_waterShad->setFloat("stepSize", water.stepSize);
		m_waterShad->setFloat("thickness", water.thickness);

		renderQuad();
	}
	glDisable(GL_BLEND);
}

void Pengu::Graphics::Rendering::DeferredPipeline::terrainPass(Pengu::Scene::SceneBase& scene, Camera& camera, Frustum& CameraFrustrum)
{
	auto settingsEntities = scene.getWorld().GetEntitiesWith<TerrainSettingsComponent>();
	if (!settingsEntities.empty()) {
		auto& terrainSettings = scene.getWorld().GetComponent<TerrainSettingsComponent>(settingsEntities[0]);

		m_terrainShad->bind();
		m_terrainShad->setMat4("projection", camera.projection_);
		m_terrainShad->setMat4("view", camera.getViewMatrix());
		m_terrainShad->setInt("weightMap", 0);
		m_terrainShad->setInt("noiseTex", 1);
		m_terrainShad->setFloat("u_seaLevel", terrainSettings.settings.seaLevel);

		// Bind Base Layer (Unit 2-6)
		if (terrainSettings.baseLayer.albedoMap) {
			glBindTextureUnit(2, terrainSettings.baseLayer.albedoMap->getID());
			m_terrainShad->setInt("textureBase.albedoMap", 2);
		}
		if (terrainSettings.baseLayer.normalMap) {
			glBindTextureUnit(3, terrainSettings.baseLayer.normalMap->getID());
			m_terrainShad->setInt("textureBase.normalMap", 3);
			m_terrainShad->setBool("textureBase.hasNormalMap", true);
		}
		else m_terrainShad->setBool("textureBase.hasNormalMap", false);

		if (terrainSettings.baseLayer.roughnessMap) {
			glBindTextureUnit(4, terrainSettings.baseLayer.roughnessMap->getID());
			m_terrainShad->setInt("textureBase.roughnessMap", 4);
			m_terrainShad->setBool("textureBase.hasRoughnessMap", true);
		}
		else m_terrainShad->setBool("textureBase.hasRoughnessMap", false);

		if (terrainSettings.baseLayer.metallicMap) {
			glBindTextureUnit(5, terrainSettings.baseLayer.metallicMap->getID());
			m_terrainShad->setInt("textureBase.metallicMap", 5);
			m_terrainShad->setBool("textureBase.hasMetallicMap", true);
		}
		else m_terrainShad->setBool("textureBase.hasMetallicMap", false);

		if (terrainSettings.baseLayer.aoMap) {
			glBindTextureUnit(6, terrainSettings.baseLayer.aoMap->getID());
			m_terrainShad->setInt("textureBase.aoMap", 6);
			m_terrainShad->setBool("textureBase.hasAOMap", true);
		}
		else m_terrainShad->setBool("textureBase.hasAOMap", false);

		m_terrainShad->setFloat("textureBase.tiling", terrainSettings.baseLayer.tiling);

		// Bind Layers 0-3 (Units 7-26)
		for (size_t i = 0; i < terrainSettings.layers.size() && i < 4; ++i) {
			int unitOffset = 7 + (int)i * 5;
			std::string structName = "layers[" + std::to_string(i) + "]";

			if (terrainSettings.layers[i].albedoMap) {
				glBindTextureUnit(unitOffset, terrainSettings.layers[i].albedoMap->getID());
				m_terrainShad->setInt((structName + ".albedoMap").c_str(), unitOffset);
			}

			if (terrainSettings.layers[i].normalMap) {
				glBindTextureUnit(unitOffset + 1, terrainSettings.layers[i].normalMap->getID());
				m_terrainShad->setInt((structName + ".normalMap").c_str(), unitOffset + 1);
				m_terrainShad->setBool((structName + ".hasNormalMap").c_str(), true);
			}
			else m_terrainShad->setBool((structName + ".hasNormalMap").c_str(), false);

			if (terrainSettings.layers[i].roughnessMap) {
				glBindTextureUnit(unitOffset + 2, terrainSettings.layers[i].roughnessMap->getID());
				m_terrainShad->setInt((structName + ".roughnessMap").c_str(), unitOffset + 2);
				m_terrainShad->setBool((structName + ".hasRoughnessMap").c_str(), true);
			}
			else m_terrainShad->setBool((structName + ".hasRoughnessMap").c_str(), false);

			if (terrainSettings.layers[i].metallicMap) {
				glBindTextureUnit(unitOffset + 3, terrainSettings.layers[i].metallicMap->getID());
				m_terrainShad->setInt((structName + ".metallicMap").c_str(), unitOffset + 3);
				m_terrainShad->setBool((structName + ".hasMetallicMap").c_str(), true);
			}
			else m_terrainShad->setBool((structName + ".hasMetallicMap").c_str(), false);

			if (terrainSettings.layers[i].aoMap) {
				glBindTextureUnit(unitOffset + 4, terrainSettings.layers[i].aoMap->getID());
				m_terrainShad->setInt((structName + ".aoMap").c_str(), unitOffset + 4);
				m_terrainShad->setBool((structName + ".hasAOMap").c_str(), true);
			}
			else m_terrainShad->setBool((structName + ".hasAOMap").c_str(), false);

			m_terrainShad->setFloat((structName + ".tiling").c_str(), terrainSettings.layers[i].tiling);
		}

		if (terrainSettings.settings.noiseTextureID) {
			glBindTextureUnit(1, terrainSettings.settings.noiseTextureID->getID());
		}

		for (auto obj : scene.getTerrainRenderables()) {
			auto& terrainDraw = scene.getWorld().GetComponent<TerrainDrawableComponent>(obj);
			auto& transform = scene.getWorld().GetComponent<TransformComponent>(obj);
			if (terrainDraw.gameObj->bVisible)
			{
				auto localBounds = terrainDraw.gameObj->subMeshes[0].mesh_->localBounds;

				auto worldBounds = terrainDraw.gameObj->subMeshes[0].mesh_->GetWorldBounds(localBounds, transform);

				if (!CameraFrustrum.ContainsSphere(worldBounds.center, worldBounds.radius))
				{
					continue;
				}
			};

			auto& chunkData = scene.getWorld().GetComponent<TerrainChunkComponent>(obj);
			if (chunkData.weightMap) {
				glBindTextureUnit(0, chunkData.weightMap->getID());
				m_terrainShad->setInt("weightMap", 0);
			}

			m_terrainShad->setMat4("model", buildModelMatrix(transform));
			m_terrainShad->setFloat("maxTerrainHeight", chunkData.maxHeight);
			m_terrainShad->setBool("DrawNormals", bDrawNorms);
			m_terrainShad->setBool("DrawUV", bDrawUV);
			drawSubMeshesWithLOD(terrainDraw.gameObj, *m_terrainShad);
		}
	}
}

void Pengu::Graphics::Rendering::DeferredPipeline::resetBlendState()
{
	m_deferredShad->unbind();
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

void Pengu::Graphics::Rendering::DeferredPipeline::constructLightMatrix(LightData& light, Camera& camera)
{

	glm::mat4 LightMatrix = 0.0f;

	glm::mat4 proj = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);

	if (light.light->type_ == 0)
	{

		light.light->cascadeSpaceMatrices.clear();
		light.light->cascadeSplits.clear();

		float nearP = camera.getNear();
		float farP = camera.getFar();

		std::vector<float> shadowCascadeLeveles{ farP * 0.1f, farP * 0.3f, farP };

		for (size_t i = 0; i < shadowCascadeLeveles.size(); ++i)
		{
			float splitNear = (i == 0) ? nearP : shadowCascadeLeveles[i - 1];
			float splitFar = shadowCascadeLeveles[i];

			glm::mat4 splitProj = glm::perspective(glm::radians(camera.zoom_), camera.m_aspect, splitNear, splitFar);
			auto corners = getFrustumCornersWorldSpace(splitProj, camera.getViewMatrix());

			glm::vec3 center = glm::vec3(0.0f);
			for (const auto& v : corners) center += glm::vec3(v);
			center /= corners.size();

			glm::mat4 lightView = glm::lookAt(center - light.light->direction_, center, glm::vec3(0.0f, 1.0f, 0.0f));

			float minX = std::numeric_limits<float>::max();
			float maxX = std::numeric_limits<float>::lowest();
			float minY = std::numeric_limits<float>::max();
			float maxY = std::numeric_limits<float>::lowest();
			float minZ = std::numeric_limits<float>::max();
			float maxZ = std::numeric_limits<float>::lowest();


			for (const auto& v : corners) {
				const auto trf = lightView * v;
				minX = std::min(minX, trf.x);
				maxX = std::max(maxX, trf.x);
				minY = std::min(minY, trf.y);
				maxY = std::max(maxY, trf.y);
				minZ = std::min(minZ, trf.z);
				maxZ = std::max(maxZ, trf.z);
			}

			float zNear = -maxZ;
			float zFar = -minZ;

			zNear -= 1000.0f;
			zFar += 1000.0f;

			glm::mat4 lightOrtho = glm::ortho(minX, maxX, minY, maxY, zNear, zFar);
			light.light->cascadeSpaceMatrices.push_back(lightOrtho * lightView);
			light.light->cascadeSplits.push_back(splitFar);
		}
	}
	else if (light.light->type_ == 2)
	{
		float fov = glm::radians(light.light->outerCutoff_) * 2.0f;
		proj = glm::perspective(fov, 1.0f, 0.1f, camera.getFar());

		glm::vec3 pos = light.light->position_;
		glm::vec3 target = pos + light.light->direction_;

		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		if (glm::abs(glm::dot(glm::normalize(light.light->direction_), up)) > 0.99f) {
			up = glm::vec3(0.0f, 0.0f, 1.0f);
		}

		view = glm::lookAt(pos, target, up);
		light.light->lightSpaceMatrix = proj * view;
	}
}

glm::mat4 Pengu::Graphics::Rendering::DeferredPipeline::buildModelMatrix(const TransformComponent& transform)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, transform.position_);
	glm::mat4 rotation = glm::yawPitchRoll(transform.rotation_.y, transform.rotation_.x, transform.rotation_.z);
	model = model * rotation;
	model = glm::scale(model, transform.scale_);
	return model;
}

void Pengu::Graphics::Rendering::DeferredPipeline::drawSubMeshes(std::shared_ptr<Pengu::Graphics::GameObject> draw, Pengu::Graphics::Shader& shader)
{
	for (auto& sub : draw->subMeshes) {
		if (!sub.visible || !sub.mesh_->isValid()) continue;
		sub.material_->Bind(shader);
		sub.mesh_->Draw(bWireFrame);
	}
}

void Pengu::Graphics::Rendering::DeferredPipeline::drawSubMeshesWithLOD(std::shared_ptr<Pengu::Graphics::GameObject> draw, Pengu::Graphics::Shader& shader)
{
	for (auto& sub : draw->subMeshes) {
		if (!sub.visible || !sub.mesh_->isValid()) continue;
		sub.material_->Bind(shader);
		sub.mesh_->Draw(bWireFrame, draw->currentLOD);
	}
}

std::vector<glm::vec4> Pengu::Graphics::Rendering::DeferredPipeline::getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
	const auto inv = glm::inverse(proj * view);
	std::vector<glm::vec4> frustumCorners;
	for (unsigned int x = 0; x < 2; ++x) {
		for (unsigned int y = 0; y < 2; ++y) {
			for (unsigned int z = 0; z < 2; ++z) {
				const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
				frustumCorners.push_back(pt / pt.w);
			}
		}
	}
	return frustumCorners;
}
