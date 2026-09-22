/**
 * @file Forward.hpp
 * @brief Basic forward rendering pipeline.
 */

#ifndef FORWARD_HPP
#define FORWARD_HPP 1

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
	 * @class ForwardPipeline
	 * @brief Implementation of a standard forward shading pipeline.
	 * 
	 * Objects are rendered one by one, with light contributions calculated
	 * for each object during the main pass.
	 */
	class ForwardPipeline : public RenderPipeline
	{
	public:

		/**
		 * @brief Initializes the forward pipeline.
		 * @param rm Reference to the resource manager.
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
		 * @brief Renders the scene using forward shading.
		 * @param scene The scene to render.
		 * @param camera The active camera.
		 */
		void render(Pengu::Scene::SceneBase& scene, Camera& camera) override;

		/**
		 * @brief Handles window resize events.
		 * @param width New width.
		 * @param height New height.
		 */
		void onResize(int width, int height) override;

		/**
		 * @brief Gets the pipeline name.
		 * @return "Forward"
		 */
		const std::string& getName() const override
		{
			static const std::string name = "Forward";
			return name;
		}

	private:
		/**
		 * @brief Builds a model matrix from a transform component.
		 */
		glm::mat4 buildModelMatrix(const TransformComponent& transform);

		/**
		 * @brief Draws sub-meshes of a drawable component.
		 */
		void drawSubMeshes(const DrawableComponent& draw);

		/**
		 * @brief Binds light uniforms to the shader.
		 */
		void bindLight(const Lights& light);

		/**
		 * @brief Resets OpenGL blend state.
		 */
		void resetBlendState();

		std::shared_ptr<Pengu::Graphics::Shader> m_shader; ///< The forward shading shader.
		bool m_firstpass = true; ///< Flag to identify the first light pass.
		int m_width = 0;  ///< Current viewport width.
		int m_height = 0; ///< Current viewport height.
	};
}
#endif // !FORWARD_HPP
