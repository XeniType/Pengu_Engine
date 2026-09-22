/**
 * @file SceneBase.hpp
 * @brief Abstract base class for all game scenes.
 */

#ifndef SCENEBASE_HPP 
#define SCENEBASE_HPP 1

#include "Pengu_Engine/Managers/ECS/ECSManager.hpp"
#include "Pengu_Engine/Components/DrawableComponent.hpp"
#include "Pengu_Engine/Components/TransformComponent.hpp"
#include "Pengu_Engine/Components/LightComponent.hpp"
#include "Pengu_Engine/Components/ScriptComponent.hpp"
#include "Pengu_Engine/Systems/LightSystem.hpp"
#include "Pengu_Engine/Systems/ScriptSystem.hpp"
#include "Pengu_Engine/Objects/skybox.hpp"

#include <vector>
#include <string>
#include <algorithm>

namespace Pengu::Resources { class ResourceManager; }
namespace Pengu::Core { class PenguEngine; }

namespace Pengu::Scene
{
	/**
	 * @class SceneBase
	 * @brief Defines the structure and common functionality for a game scene.
	 * 
	 * SceneBase manages its own ECS world and tracks renderable entities.
	 * It provides hooks for initialization, updates, and loading/unloading.
	 */
	class SceneBase {
	public:
		/**
		 * @brief Constructs a scene base and initializes the ECS world.
		 * @param engine Pointer to the engine instance.
		 */
		SceneBase(Pengu::Core::PenguEngine* engine) : m_world(std::make_unique<ECSManager>()), m_engine(engine) {
			m_world->Init();

			m_world->onRenderableAdded = [this](Entity e) {
				this->registerRenderable(e);
				};
			m_world->onRenderableRemoved = [this](Entity e) {
				this->unregisterRenderable(e);
				};

			m_world->onTerrainAdded = [this](Entity e) {
				this->registerTerrainRenderable(e);
				};
			m_world->onTerrainRemoved = [this](Entity e) {
				this->unregisterTerrainRenderable(e);
				};
		};

		/**
		 * @brief Virtual destructor.
		 */
		virtual ~SceneBase() = default;

		/// @name Deleted/Default Operations
		/// @{
		SceneBase(const SceneBase&) = delete;
		SceneBase& operator=(const SceneBase&) = delete;
		SceneBase(SceneBase&&) = default;
		SceneBase& operator=(SceneBase&&) = default;
		/// @}

		/**
		 * @brief Updates the scene.
		 * @param dt Delta time since the last frame.
		 */
		virtual void update(float dt) = 0;

		/**
		 * @brief Called when the scene is loaded into the manager.
		 * @param rm Reference to the resource manager.
		 */
		virtual void onLoad(Pengu::Resources::ResourceManager& rm) = 0;

		/**
		 * @brief Called when the scene is unloaded.
		 */
		virtual void onUnload() = 0;

		/** @brief Gets the ECS world for this scene. */
		ECSManager& getWorld() { return *m_world; };
		/** @brief Gets the ECS world for this scene (const). */
		const ECSManager& getWorld() const { return *m_world; };

		/** @brief Gets the list of renderable entities. */
		const std::vector<Entity>& getRenderables() const { return m_renderables; }
		/** @brief Gets the list of terrain renderable entities. */
		const std::vector<Entity>& getTerrainRenderables() const { return m_terrainRenderables; }

		/** @brief Sets the light system for the scene. */
		void setLightSystem(std::shared_ptr<LightSystem> ls) { m_lightSystem = ls; }
		/** @brief Gets the light system for the scene. */
		std::shared_ptr<LightSystem> getLightSystem() const { return m_lightSystem; }

		/**
		 * @brief Registers an entity as a renderable object.
		 * @param id The entity handle.
		 */
		void registerRenderable(Entity id);

		/**
		 * @brief Unregisters an entity from the renderable list.
		 * @param id The entity handle.
		 */
		void unregisterRenderable(Entity id);

		/**
		 * @brief Registers an entity as a terrain renderable.
		 * @param id The entity handle.
		 */
		void registerTerrainRenderable(Entity id) {
			m_terrainRenderables.push_back(id);
		}

		/**
		 * @brief Unregisters an entity from the terrain renderable list.
		 * @param id The entity handle.
		 */
		void unregisterTerrainRenderable(Entity id) {
			m_terrainRenderables.erase(
				std::remove(m_terrainRenderables.begin(), m_terrainRenderables.end(), id),
				m_terrainRenderables.end()
			);
		}

		/** @brief Gets the human-readable name of the scene. */
		const std::string getName() const { return m_name; };

		/** @brief Checks if the scene has a skybox. */
		bool HasSkyBox() const { return m_skybox != nullptr; }
		/** @brief Gets the skybox instance. */
		SkyBox& GetSkyBox() const { return *m_skybox; }
		/** @brief Sets the skybox for the scene. */
		void SetSkyBox(std::unique_ptr<SkyBox> skybox) { m_skybox = std::move(skybox); }

	protected:
		/**
		 * @brief Internal initialization hook.
		 */
		virtual void onInitialize() = 0;

		std::string m_name = "Unnamed Scene"; ///< Name of the scene.
		Pengu::Core::PenguEngine* m_engine = nullptr; ///< Pointer back to the engine.

		Pengu::Resources::ResourceManager* m_resourceManager = nullptr; ///< Pointer to the resource manager.
	private:
		std::unique_ptr<ECSManager> m_world;          ///< The ECS world for this scene.
		std::vector<Entity> m_renderables;            ///< List of standard renderable entities.
		std::vector<Entity> m_terrainRenderables;     ///< List of terrain renderable entities.
		std::shared_ptr<LightSystem> m_lightSystem;   ///< System for collecting light data.

		std::unique_ptr<SkyBox> m_skybox = nullptr;   ///< The skybox for the scene.
	};

} //PenguEngine::Scene
#endif // ! SCENEBASE_HPP 
