/**
 * @file Deferred.hpp
 * @brief Deferred rendering pipeline implementation.
 */

#ifndef DEFERRED_HPP
#define DEFERRED_HPP 1

#include "Pengu_Engine/Graphics/Rendering/RenderPipeline.hpp"
#include "Pengu_Engine/Components/DrawableComponent.hpp"
#include "Pengu_Engine/Components/TransformComponent.hpp"
#include "Pengu_Engine/Components/LightComponent.hpp"
#include "Pengu_Engine/Systems/LightSystem.hpp"
#include "Pengu_Engine/Camera/Fustrum.hpp"

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Pengu::Resources { class ResourceManager; }
namespace Pengu::Scene { class SceneBase; }
namespace Pengu::Graphics { class Shader; class GameObject; }


namespace Pengu::Graphics::Rendering {

	/**
	 * @class DeferredPipeline
	 * @brief Implementation of a deferred rendering pipeline.
	 */
	class DeferredPipeline : public RenderPipeline
	{
	public:
		/**
		 * @brief Initializes the deferred pipeline.
		 * @param rm Reference to the resource manager.
		 */
		void init(Pengu::Resources::ResourceManager& rm) override;
		/**
		 * @brief Cleans up pipeline resources.
		 */
		void cleanup() override;
		/**
		 * @brief Reloads pipeline resources.
		 * @param rm Reference to the resource manager.
		 */
		void reload(Pengu::Resources::ResourceManager& rm) override;

		/**
		 * @brief Called before the main rendering pass.
		 * @param scene The scene to render.
		 * @param camera The camera to render from.
		 */
		void onPreRender(Pengu::Scene::SceneBase& scene, Camera& camera) override;
		/**
		 * @brief Renders the scene using deferred shading.
		 * @param scene The scene to render.
		 * @param camera The camera to render from.
		 */
		void render(Pengu::Scene::SceneBase& scene, Camera& camera) override;

		/**
		 * @brief Handles window resize events.
		 * @param width New width.
		 * @param height New height.
		 */
		void onResize(int width, int height) override;

		/**
		 * @brief Gets the name of the pipeline.
		 * @return const std::string& Pipeline name.
		 */
		const std::string& getName() const override
		{
			static const std::string name = "Deferred";
			return name;
		}


	private:

		/** @brief Sets up the G-Buffer. */
		void setupBuffer();
		/** @brief Deletes G-Buffer resources. */
		void deleteBuffers();

		/** @brief Sets up the full-screen quad. */
		void setupQuad();
		/** @brief Renders the full-screen quad. */
		void renderQuad();

		/** @brief Sets up shadow Framebuffer Objects. */
		void setupShadowFBO();
		/** @brief Sets up Point shadow Framebuffer Objects. */
		void setupPointShadowMap();

		/**
		 * @brief Binds a light source to the shader.
		 * @param light The light source to bind.
		 */
		void bindLight(const Lights& light);
		/** @brief Resets the OpenGL blend state. */
		void resetBlendState();

		/* Render Passes */
		/** @brief Shadow map generation pass. */
		void shadowPass(Pengu::Scene::SceneBase& scene, Camera& camera);
		/** @brief Point Shadow map generation pass. */
		void pointShadowPass(Lights& light, Pengu::Scene::SceneBase& scene);
		/** @brief G-Buffer generation pass. */
		void geometryPass(Pengu::Scene::SceneBase& scene, Camera& camera);
		/** @brief Lighting pass using G-Buffer data. */
		void lightPass(Pengu::Scene::SceneBase& scene, Camera& camera);
		/** @brief Water rendering pass. */
		void waterPass(Pengu::Scene::SceneBase& scene, Camera& camera, unsigned int targetFBO);
		/** @brief Terrain rendering pass. */
		void terrainPass(Pengu::Scene::SceneBase& scene, Camera& camera, Frustum& CameraFrustrum);

		/** @brief Constructs the light matrix for shadows. */
		void constructLightMatrix(LightData& light, Camera& camera);

		/** @brief Builds a model matrix from a transform component. */
		glm::mat4 buildModelMatrix(const TransformComponent& transform);
		/** @brief Draws sub-meshes of a game object. */
		void drawSubMeshes(std::shared_ptr<GameObject> draw, Pengu::Graphics::Shader& shader);
		/** @brief Draws sub-meshes with Level of Detail (LOD). */
		void drawSubMeshesWithLOD(std::shared_ptr<GameObject> draw, Pengu::Graphics::Shader& shader);

		/* Shaders */
		/** @brief Shader for G-Buffer filling. */
		std::shared_ptr<Pengu::Graphics::Shader> m_bufferShad;
		/** @brief Shader for deferred lighting. */
		std::shared_ptr<Pengu::Graphics::Shader> m_deferredShad;
		/** @brief Shader for terrain rendering. */
		std::shared_ptr<Pengu::Graphics::Shader> m_terrainShad;
		/** @brief Shader for skybox rendering. */
		std::shared_ptr<Pengu::Graphics::Shader> m_skyboxShad;
		/** @brief Shader for water rendering. */
		std::shared_ptr<Pengu::Graphics::Shader> m_waterShad;
		/** @brief Shader for shadow mapping. */
		std::shared_ptr<Pengu::Graphics::Shader> m_shadowShad;
		/** @brief Shader for point shadow mapping. */
		std::shared_ptr<Pengu::Graphics::Shader> m_pointShadowShader;

		/* Buffers */
		/** @brief G-Buffer FBO ID. */
		unsigned int m_gBuffer = 0;
		/** @brief Position texture ID. */
		unsigned int m_gPosition = 0;
		/** @brief Normal texture ID. */
		unsigned int m_gNormal = 0;
		/** @brief Color and Specular texture ID. */
		unsigned int m_gColorSpec = 0;
		/** @brief PBR data texture ID. */
		unsigned int m_gPBR = 0;
		/** @brief Renderbuffer ID for depth. */
		unsigned int m_rboDepth = 0;

		/* Intermediate Lit Scene Buffer */
		/** @brief FBO for the lit scene. */
		unsigned int m_litFBO = 0;
		/** @brief Texture ID for the scene color. */
		unsigned int m_sceneColor = 0;

		/* Quad */
		/** @brief VAO for the full-screen quad. */
		unsigned int m_quadVAO = 0;
		/** @brief VBO for the full-screen quad. */
		unsigned int m_quadVBO = 0;

		/** @brief Flag for the first render pass. */
		bool m_firstpass = true;

		/** @brief Gets frustum corners in world space. */
		std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view);

		/** @brief FBOs for cascaded shadows. */
		unsigned int cascadeFBOs[3];
		/** @brief Textures for cascaded shadows. */
		unsigned int cascadeMaps[3];
		/** @brief Resolution for directional shadows. */
		unsigned int m_shadowDirResolution = 2048;

		/** @brief FBO for spot light shadows. */
		unsigned int m_SpotShadowFBO = 0;
		/** @brief Texture for spot light shadows. */
		unsigned int m_SpotShadowMap = 0;
		/** @brief Resolution for spot shadows. */
		unsigned int m_shadowSpotResolution = 512;

		/** @brief FBO for point light shadows. */
		unsigned int m_PointShadowFBO = 0;
		/** @brief Texture for point light shadows. */
		unsigned int m_PointShadowCubeMap = 0;
	};
};


#endif // !DEFERRED_HPP
