/**
 * @file Scene.hpp
 * @brief Standard scene implementation.
 */

#ifndef SCENE_HPP 
#define SCENE_HPP 1

#include "Pengu_Engine/Scene/SceneBase.hpp"
#include "Pengu_Engine/PenguEngine.hpp"

namespace Pengu::Resources { class ResourceManager; }

namespace Pengu::Scene {

	/**
	 * @class Scene
	 * @brief Concrete implementation of a scene.
	 * 
	 * This class provides standard behavior for scene updates and loading/unloading
	 * logic.
	 */
	class Scene : public SceneBase {
	public:
		/**
		 * @brief Constructs the scene.
		 * @param engine Pointer to the engine instance.
		 */
		Scene(Pengu::Core::PenguEngine* engine) : SceneBase(engine) { m_name = "Scene"; }

		/**
		 * @brief Updates the scene logic.
		 * @param dt Delta time since the last frame.
		 */
		void update(float dt) override;

		/**
		 * @brief Called when the scene is loaded.
		 * @param rm Reference to the resource manager.
		 */
		void onLoad(Pengu::Resources::ResourceManager& rm) override;

		/**
		 * @brief Called when the scene is unloaded to clean up resources.
		 */
		void onUnload() override;

	};
}
#endif // ! SCENE_HPP 
