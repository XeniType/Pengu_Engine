/**
 * @file SceneManager.hpp
 * @brief Manages the lifecycle and transitions between game scenes.
 */

#ifndef SCENEMANAGER_HPP
#define SCENEMANAGER_HPP 1

#include <memory>

namespace Pengu::Scene { class SceneBase; }
namespace Pengu::Resources { class ResourceManager; }
namespace Pengu::Core { class PenguEngine; }

namespace Pengu::Scene {

	/**
	 * @class SceneManager
	 * @brief Responsible for loading, unloading, and updating the active scene.
	 */
	class SceneManager {

	public:

		/**
		 * @brief Default constructor.
		 */
		SceneManager() = default;

		/**
		 * @brief Default destructor.
		 */
		~SceneManager() = default;

		/// @name Deleted/Default Operations
		/// @{
		SceneManager(const SceneManager&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;

		SceneManager(SceneManager&&) = default;
		SceneManager& operator=(SceneManager&&) = default;
		/// @}

		/**
		 * @brief Loads a new scene and replaces the current one.
		 * @param newScene Unique pointer to the scene to load.
		 * @param rm Reference to the resource manager to pass to the scene.
		 */
		void loadScene(std::unique_ptr<SceneBase> newScene, Pengu::Resources::ResourceManager& rm);

		/**
		 * @brief Unloads the current active scene.
		 */
		void unloadCurrent();

		/**
		 * @brief Updates the active scene.
		 * @param dt Delta time since the last frame.
		 */
		void update(float dt);

		/** @brief Gets the currently active scene. */
		Pengu::Scene::SceneBase* getActiveScene() { return m_activeScene.get(); };
		/** @brief Gets the currently active scene (const). */
		const Pengu::Scene::SceneBase* getActiveScene() const { return m_activeScene.get(); };


	private:
		std::unique_ptr<SceneBase> m_activeScene; ///< The currently active scene.

	};
}// Pengu::Scene

#endif // !SCENEMANAGER_HPP
