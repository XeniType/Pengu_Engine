/**
 * @file RenderPipeline.hpp
 * @brief Base class for all rendering pipelines.
 */

#ifndef RENDERPIPELINE_HPP
#define RENDERPIPELINE_HPP 1

#include "Pengu_Engine/Camera/Camera.hpp"
#include <string>

namespace Pengu::Resources { class ResourceManager; }
namespace Pengu::Scene { class SceneBase; }

namespace Pengu::Graphics::Rendering {

	/**
	 * @class RenderPipeline
	 * @brief Abstract base class defining the interface for a rendering technique.
	 * 
	 * Different rendering strategies (Forward, Deferred, etc.) inherit from this 
	 * class and implement the rendering logic.
	 */
	class RenderPipeline {
	public:

		/**
		 * @brief Default constructor.
		 */
		RenderPipeline() = default;

		/**
		 * @brief Virtual destructor.
		 */
		virtual ~RenderPipeline() = default;

		/// @name Deleted Operations
		/// @{
		RenderPipeline(const RenderPipeline&) = delete;
		RenderPipeline& operator=(const RenderPipeline&) = delete;
		/// @}

		/**
		 * @brief Initializes the pipeline.
		 * @param rm Reference to the resource manager.
		 */
		virtual void init(Pengu::Resources::ResourceManager& rm) = 0;

		/**
		 * @brief Cleans up pipeline-specific resources.
		 */
		virtual void cleanup() = 0;

		/**
		 * @brief Reloads pipeline resources (e.g., shaders).
		 * @param rm Reference to the resource manager.
		 */
		virtual void reload(Pengu::Resources::ResourceManager& rm) = 0;

		/**
		 * @brief Optional hook called before the main render pass.
		 * @param scene The scene to be rendered.
		 * @param camera The camera.
		 */
		virtual void onPreRender(Pengu::Scene::SceneBase& scene, Camera& camera) {}

		/**
		 * @brief Executes the main rendering pass for the scene.
		 * @param scene The scene to render.
		 * @param camera The camera.
		 */
		virtual void render(Pengu::Scene::SceneBase& scene, Camera& camera) = 0;

		/**
		 * @brief Handles viewport resize events.
		 * @param width New width.
		 * @param height New height.
		 */
		virtual void onResize(int width, int height) = 0;

		/**
		 * @brief Gets the human-readable name of the pipeline.
		 * @return The pipeline name.
		 */
		virtual const std::string& getName() const = 0;

		/** @brief Checks if the pipeline has been initialized. */
		bool isInitialized() const { return m_initialized; }

		bool bWireFrame = false; ///< Global wireframe toggle for the pipeline.
		bool bDrawNorms = false; ///< Global debug normals toggle.
		bool bDrawUV = false;    ///< Global debug UVs toggle.

	protected:
		bool m_initialized = false; ///< Initialization state flag.
		int m_width = 0;            ///< Current viewport width.
		int m_height = 0;           ///< Current viewport height.
		Pengu::Resources::ResourceManager* aux_rm; ///< Pointer to the resource manager for internal use.
	};
}
#endif // ! RENDERPIPELINE_HPP