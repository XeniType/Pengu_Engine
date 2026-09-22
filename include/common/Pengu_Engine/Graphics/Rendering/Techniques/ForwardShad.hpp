/**
 * @file ForwardShad.hpp
 * @brief Forward rendering pipeline with shadow mapping support.
 */

#ifndef FORWARDSHAD_HPP
#define FORWARDSHAD_HPP 1

#include "Pengu_Engine/Graphics/Rendering/RenderPipeline.hpp"
#include "Pengu_Engine/Components/DrawableComponent.hpp"
#include "Pengu_Engine/Components/TransformComponent.hpp"
#include "Pengu_Engine/Components/LightComponent.hpp"

#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Pengu::Resource { class ResourceManager; }
namespace Pengu::Scene { class SceneBase; }
namespace Pengu::Graphics { class Shader; }

namespace Pengu::Graphics::Rendering {

	/**
	 * @class ForwardShadPipeline
	 * @brief A render pipeline that implements forward shading with shadows.
	 * 
	 * This pipeline handles rendering objects using a forward shading approach,
	 * including multi-pass shadow mapping for directional, point, and spot lights.
	 */
	class ForwardShadPipeline : public RenderPipeline
	{
	public:

		/**
		 * @brief Initializes the forward shadow pipeline.
		 * @param rm Reference to the resource manager to load shaders and other resources.
		 */
		void init(Pengu::Resources::ResourceManager& rm) override;

		/**
		 * @brief Cleans up pipeline resources.
		 */
		void cleanup() override;

		/**
		 * @brief Reloads shaders and resources.
		 * @param rm Reference to the resource manager.
		 */
		void reload(Pengu::Resources::ResourceManager& rm) override;

		/**
		 * @brief Renders the scene from the camera's perspective.
		 * @param scene The scene to render.
		 * @param camera The camera viewing the scene.
		 */
		void render(Pengu::Scene::SceneBase& scene, Camera& camera) override;

		/**
		 * @brief Handles window resize events.
		 * @param width New window width.
		 * @param height New window height.
		 */
		void onResize(int width, int height) override;

		/**
		 * @brief Gets the human-readable name of the pipeline.
		 * @return A string containing the pipeline name.
		 */
		const std::string& getName() const override
		{
			static const std::string name = "Forward Shadows";
			return name;
		}

	private:
		/**
		 * @brief Destroys shadow map textures and FBOs.
		 */
		void destroyLightShadowMap();

		/**
		 * @brief Performs a shadow pass for a directional light.
		 * @param light The directional light.
		 * @param scene The scene to render into the shadow map.
		 */
		void dirShadowPass(Lights& light, Pengu::Scene::SceneBase& scene);

		/**
		 * @brief Performs a shadow pass for a spot light.
		 * @param light The spot light.
		 * @param scene The scene to render into the shadow map.
		 */
		void spotShadowPass(Lights& light, Pengu::Scene::SceneBase& scene);

		/**
		 * @brief Performs a shadow pass for a point light.
		 * @param light The point light.
		 * @param scene The scene to render into the shadow map.
		 */
		void pointShadowPass(Lights& light, Pengu::Scene::SceneBase& scene);

		/**
		 * @brief Sets up the FBO and texture for directional shadows.
		 */
		void setupDirShadowMap();

		/**
		 * @brief Sets up the FBO and texture for spot shadows.
		 */
		void setupSpotShadowMap();

		/**
		 * @brief Sets up the FBO and texture for point shadows (cubemap).
		 */
		void setupPointShadowMap();

		/**
		 * @brief Builds the light space matrix for shadow mapping.
		 * @param ld The light component.
		 * @param cam The camera (used for directional light frustum fitting).
		 * @return The 4x4 light space matrix.
		 */
		glm::mat4 buildLightSpaceMatrix(const Lights& ld, const Camera& cam);

		/**
		 * @brief Builds the 6 view-projection matrices for point light shadow cubemaps.
		 * @param ld The light component.
		 * @return A vector containing 6 matrices.
		 */
		std::vector<glm::mat4> buildPointLightMatrices(const Lights& ld);

		/**
		 * @brief Builds a model matrix from a transform component.
		 * @param transform The transform component.
		 * @return The 4x4 model matrix.
		 */
		glm::mat4 buildModelMatrix(const TransformComponent& transform);

		/**
		 * @brief Uploads transform uniforms to the current shader.
		 * @param transform The transform component.
		 * @param cam The camera.
		 */
		void uploadTransform(const TransformComponent& transform, Camera& cam);

		/**
		 * @brief Draws all sub-meshes of a drawable component.
		 * @param draw The drawable component.
		 */
		void drawSubMeshes(const DrawableComponent& draw);

		/**
		 * @brief Draws all sub-meshes for a shadow pass (depth only).
		 * @param draw The drawable component.
		 */
		void drawSubMeshesShadow(const DrawableComponent& draw);

		/**
		 * @brief Binds light uniforms to the forward shader.
		 * @param light The light component to bind.
		 */
		void bindLight(const Lights& light);

		/**
		 * @brief Resets the OpenGL blend state after rendering.
		 */
		void resetBlendState();

		std::shared_ptr<Pengu::Graphics::Shader> m_forwardShader; ///< The main forward shader.
		std::shared_ptr<Pengu::Graphics::Shader> m_shadowShader;  ///< Shader for directional and spot shadows.
		std::shared_ptr<Pengu::Graphics::Shader> m_pointShadowShader; ///< Shader for point shadow cubemaps.
		bool m_firstpass = true; ///< Tracks if this is the first light pass.
		int m_width = 0;  ///< Current viewport width.
		int m_height = 0; ///< Current viewport height.

		int m_dirShadowResolution = 2048; ///< Resolution for directional shadow maps.
		GLuint m_DirShadowFBO = 0;        ///< FBO for directional shadows.
		GLuint m_DirShadowMap = 0;        ///< Depth texture for directional shadows.

		int m_shadowResolution = 512;     ///< Default resolution for other shadow maps.

		GLuint m_SpotShadowFBO = 0;       ///< FBO for spot shadows.
		GLuint m_SpotShadowMap = 0;       ///< Depth texture for spot shadows.

		GLuint m_PointShadowFBO = 0;      ///< FBO for point shadows.
		GLuint m_PointShadowCubeMap = 0;  ///< Depth cubemap for point shadows.
	};
}
#endif // !FORWARDSHAD_HPP
