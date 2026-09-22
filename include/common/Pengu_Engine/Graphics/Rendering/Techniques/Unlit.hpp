/**
 * @file Unlit.hpp
 * @brief Rendering pipeline for unlit materials.
 */

#ifndef UNLIT_HPP
#define UNLIT_HPP 1

#include "Pengu_Engine/Graphics/Rendering/RenderPipeline.hpp"
#include "Pengu_Engine/Components/DrawableComponent.hpp"
#include "Pengu_Engine/Components/TransformComponent.hpp"

#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Pengu::Resources { class ResourceManager; }
namespace Pengu::Scene { class SceneBase; }
namespace Pengu::Graphics { class Shader; }

namespace Pengu::Graphics::Rendering {

	/**
	 * @class UnlitPipeline
	 * @brief Render pipeline that ignores lighting and only renders base colors/textures.
	 */
	class UnlitPipeline : public RenderPipeline {

	public:

		/**
		 * @brief Initializes the unlit pipeline.
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
		 * @brief Renders the scene without lighting.
		 * @param scene The scene to render.
		 * @param camera The camera.
		 */
		void render(Pengu::Scene::SceneBase& scene, Camera& camera) override;

		/**
		 * @brief Handles resize events.
		 */
		void onResize(int width, int height) override;

		/**
		 * @brief Gets the pipeline name.
		 * @return "Unlit"
		 */
		const std::string& getName() const override
		{
			static const std::string name = "Unlit";
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
		 * @brief Resets OpenGL blend state.
		 */
		void resetBlendState();

		std::shared_ptr<Pengu::Graphics::Shader> m_shader; ///< The unlit shader.
		int m_width = 0;  ///< Current viewport width.
		int m_height = 0; ///< Current viewport height.
	};
}
#endif // !UNLIT_HPP
