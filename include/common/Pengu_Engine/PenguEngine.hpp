/**
 * @file PenguEngine.hpp
 * @brief Main entry point and orchestrator for the Pengu Engine core systems.
 */

#ifndef PENGUENGINE_HPP 
#define PENGUENGINE_HPP 1

#include "Pengu_Engine/Window/Window.hpp"
#include "Pengu_Engine/Inputs/Input.hpp"
#include "Pengu_Engine/Camera/Camera.hpp"
#include "Pengu_Engine/Misc/Logger.hpp"

#include "Pengu_Engine/Managers/Resource/ResourceManager.hpp"
#include "Pengu_Engine/Scene/SceneManager.hpp"
#include "Pengu_Engine/Graphics/Rendering/Renderer.hpp"
#include "Pengu_Engine/Scene/SceneBase.hpp"

#include "Pengu_Engine/Misc/JobSystem.hpp"

#include <string>
#include <optional>

namespace Pengu::Resources { class ResourceManager; }
namespace Pengu::Scene { class SceneManager; class SceneBase; }
namespace Pengu::Graphics::Rendering { class Renderer; }

namespace Pengu::Core {

	/**
	 * @enum RenderPipeline
	 * @brief Enumeration of supported rendering pipelines.
	 */
	enum class RenderPipeline {
		Unlit,       ///< Basic rendering without lighting.
		Forward,     ///< Standard forward shading.
		ForwardShad, ///< Forward shading with shadow mapping.
		Deferred     ///< Deferred rendering pipeline.
	};

	/**
	 * @struct EngineConfig
	 * @brief Configuration settings for initializing the engine.
	 */
	struct EngineConfig {
		unsigned int screen_width;  ///< Initial window width.
		unsigned int screen_height; ///< Initial window height.
		std::string title;           ///< Window title bar text.
		RenderPipeline pipeline;    ///< Initial rendering pipeline to use.

		float cam_speed = 0.005f;   ///< Default camera movement speed.
		float fov = 90.0f;          ///< Default field of view in degrees.
		float cam_sens = 0.1f;      ///< Default camera look sensitivity.
	};

	/**
	 * @class PenguEngine
	 * @brief The central class that manages the lifecycle of all engine subsystems.
	 * 
	 * PenguEngine follows a move-only pattern to ensure unique ownership of
	 * hardware resources like the window context and input listeners.
	 */
	class PenguEngine {
	public:
		/// @name Deleted Operations
		/// @{
		PenguEngine(const PenguEngine&) = delete;
		PenguEngine& operator=(const PenguEngine&) = delete;
		/// @}

		/**
		 * @brief Move constructor.
		 */
		PenguEngine(PenguEngine&&);

		/**
		 * @brief Move assignment operator.
		 */
		PenguEngine& operator=(PenguEngine&&);

		/**
		 * @brief Destructor that shuts down all systems.
		 */
		~PenguEngine();

		/// @name Factory Methods
		/// @{
		/**
		 * @brief Creates an engine instance with custom window parameters.
		 * @param config The engine configuration settings.
		 * @return An optional containing the engine instance if successful.
		 */
		static std::optional<PenguEngine> create(const EngineConfig& config);

		/**
		 * @brief Starts the engine with the given configuration.
		 * @param config The engine configuration.
		 * @return A new instance of PenguEngine.
		 */
		static PenguEngine startEngine(const EngineConfig& config);
		/// @}

		/**
		 * @brief Checks if the engine's window has been requested to close.
		 * @return True if the user or OS requested a close, false otherwise.
		 */
		bool IsClosing() const;

		/**
		 * @brief Finalizes the current frame.
		 * 
		 * Swaps buffers, polls events, and clears the per-frame allocators.
		 */
		void EndFrame();

		/// @name System Accessors
		/// @{
		/** @brief Returns a reference to the window management system. */
		Window& GetWindow() { return *m_window; }
		/** @brief Returns a reference to the input system. */
		Input& GetInput() { return *m_input; }
		/** @brief Returns a reference to the primary camera. */
		Camera& GetCamera() { return m_camera; }
		/** @brief Returns a reference to the logger. */
		Logger& Log() { return log_; }
		/** @brief Returns a reference to the job system. */
		JobSystem& GetJobSystem() { return *m_jobsystem; };
		/** @brief Returns a reference to the resource manager. */
		Pengu::Resources::ResourceManager& GetResourceManager();
		/** @brief Returns a reference to the scene manager. */
		Pengu::Scene::SceneManager& getSceneManager();
		/** @brief Returns a reference to the renderer. */
		Pengu::Graphics::Rendering::Renderer& getRenderer();
		/// @}

		/**
		 * @brief Loads a new scene.
		 * @param scene Unique pointer to the scene instance.
		 */
		void loadScene(std::unique_ptr<Pengu::Scene::SceneBase> scene);

		/**
		 * @brief Gets the currently active scene.
		 * @return Pointer to the active scene, or nullptr if none.
		 */
		Pengu::Scene::SceneBase* getActiveScene();

		/**
		 * @brief Updates all engine systems for the current frame.
		 */
		void update();

		/**
		 * @brief Handles window resize events.
		 * @param w New width.
		 * @param h New height.
		 */
		void onResize(int w, int h) {
			m_renderer->onResize(w, h);
			GetCamera().CreatePerspective(w, h);
		}

		/**
		 * @brief Sets the output framebuffer object.
		 * @param fbo OpenGL FBO ID.
		 */
		void setOutputFBO(unsigned int fbo) { m_outputFBO = fbo; }

	private:
		/**
		 * @brief Private constructor used by the static factory methods.
		 */
		PenguEngine(std::unique_ptr<Window> window,
			std::unique_ptr<Input> input,
			Camera                      camera,
			Logger                      log,
			std::unique_ptr<Pengu::Resources::ResourceManager> rm,
			std::unique_ptr<Pengu::Scene::SceneManager> scenemanager,
			std::unique_ptr<Pengu::Graphics::Rendering::Renderer> renderer
		);

		std::unique_ptr<Window> m_window;    ///< The GLFW window context.
		std::unique_ptr<Input> m_input;      ///< Keyboard and Mouse input state.
		Camera m_camera;                     ///< The primary viewing camera.
		Logger log_;                         ///< The engine logger.
		std::unique_ptr<Pengu::Resources::ResourceManager> m_resourceManager; ///< Resource cache.
		std::unique_ptr<Pengu::Scene::SceneManager> m_scenemanager;            ///< Scene lifecycle manager.
		std::unique_ptr<Pengu::Graphics::Rendering::Renderer> m_renderer;     ///< Rendering orchestrator.
		std::unique_ptr<JobSystem> m_jobsystem;                               ///< Async task scheduler.

		unsigned int m_outputFBO = 0; ///< Output framebuffer for offscreen rendering.
	};
}
#endif // !PENGUENGINE_HPP