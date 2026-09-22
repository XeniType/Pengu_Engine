/**
 * @file Renderer.hpp
 * @brief Main entry point for the rendering system.
 */

#ifndef RENDERER_HPP
#define RENDERER_HPP 1

#include "Pengu_Engine/Camera/Camera.hpp"
#include "Pengu_Engine/Graphics/Rendering/RenderPipeline.hpp"
#include <memory>

namespace Pengu::Scene { class SceneBase; }
namespace Pengu::Resources { class ResourceManager; }

namespace Pengu::Graphics::Rendering {

	/**
	 * @class Renderer
	 * @brief Orchestrates the rendering of a scene using a specific pipeline.
	 * 
	 * The Renderer class acts as a high-level manager that takes a scene and a camera
	 * and uses the currently active RenderPipeline to produce the final image.
	 */
	class Renderer {
	public:

		/**
		 * @brief Default constructor.
		 */
		Renderer() = default;

		/**
		 * @brief Destructor that ensures proper shutdown of the rendering system.
		 */
		~Renderer() { shutdown(); }

		/// @name Deleted Operations
		/// @{
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		/// @}

		/**
		 * @brief Initializes the renderer with a pipeline.
		 * @param pipeline Unique pointer to the render pipeline to use.
		 * @param rm Reference to the resource manager.
		 * @param screenSize Initial screen dimensions.
		 */
		void init(std::unique_ptr<RenderPipeline> pipeline, Pengu::Resources::ResourceManager& rm, glm::vec2 screenSize);

		/**
		 * @brief Replaces the current render pipeline.
		 * @param newPipeline Unique pointer to the new render pipeline.
		 */
		void setPipeline(std::unique_ptr<RenderPipeline> newPipeline);

		/**
		 * @brief Renders the given scene from the camera's perspective.
		 * @param scene The scene to render.
		 * @param camera The camera viewing the scene.
		 */
		void render(Pengu::Scene::SceneBase& scene, Camera& camera);

		/**
		 * @brief Notifies the renderer and its pipeline of a window resize.
		 * @param width New window width.
		 * @param height New window height.
		 */
		void onResize(int width, int height);

		/** @brief Gets the current render pipeline. */
		RenderPipeline* getPipeline() { return m_pipeline.get(); }
		/** @brief Gets the current render pipeline (const). */
		const RenderPipeline* getPipeline() const { return m_pipeline.get(); }

		/** @brief Checks if a pipeline is currently set. */
		bool hasPipeline() const { return m_pipeline != nullptr; }

		/**
		 * @brief Reloads the current pipeline's resources.
		 */
		void reload();

		/**
		 * @brief Shuts down the renderer and cleans up resources.
		 */
		void shutdown();

	private:
		std::unique_ptr<RenderPipeline> m_pipeline;    ///< The currently active rendering pipeline.
		Pengu::Resources::ResourceManager* m_rm = nullptr; ///< Pointer to the resource manager.
		int m_width = 0;                               ///< Current viewport width.
		int m_height = 0;                              ///< Current viewport height.
	};
}

#endif // !RENDERER_HPP
