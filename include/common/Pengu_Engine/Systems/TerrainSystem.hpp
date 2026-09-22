/**
 * @file TerrainSystem.hpp
 * @brief System for generating and managing terrain in the engine.
 */

#ifndef TERRAINSYSTEM_HPP
#define TERRAINSYSTEM_HPP 1

#include "Pengu_Engine/Systems/BaseSystems.hpp"
#include "Pengu_Engine/Camera/Camera.hpp"
#include "Pengu_Engine/Managers/ECS/ECSManager.hpp"
#include "Pengu_Engine/Components/TerrainComponent.hpp"

namespace Pengu::Core { class PenguEngine; }
namespace Pengu::Resources { class ResourceManager; }

/**
 * @class TerrainSystem
 * @brief Handles terrain generation and updates.
 */
class TerrainSystem : public System {
public:
	/**
	 * @brief Updates the terrain system.
	 * @param dt Delta time since the last frame.
	 * @param world Reference to the ECS manager.
	 * @param engine Reference to the engine instance.
	 * @param camera Reference to the active camera.
	 */
	void update(
		float dt,
		ECSManager& world,
		Pengu::Core::PenguEngine& engine,
		Camera& camera
	);

	/**
	 * @brief Generates a warp noise texture for terrain perturbation.
	 * @param width Texture width.
	 * @param height Texture height.
	 * @param seed Random seed for noise generation.
	 * @return Shared pointer to the generated texture.
	 */
	std::shared_ptr<Pengu::Graphics::Texture> GenerateWarpNoiseTexture(int width = 16, int height = 16, int seed = 1337);

	/**
	 * @brief Calculates the terrain height at a specific world position.
	 * @param worldX World X coordinate.
	 * @param worldZ World Z coordinate.
	 * @param settings Terrain settings used for height calculation.
	 * @return The calculated height (Y coordinate).
	 */
	static float GetHeight(float worldX, float worldZ, const TerrainSettingsComponent& settings);

private:
	/**
	 * @brief Generates terrain mesh data for a specific tile.
	 * @param x Tile X coordinate.
	 * @param z Tile Z coordinate.
	 * @param vertexSize Number of vertices along one side of the tile.
	 * @param settings Noise settings for generation.
	 * @return Package containing the generated vertex and index data.
	 */
	TerrainDataPackage GenerateData(int x, int z, int vertexSize, NoiseSettings settings);
};

#endif // !TERRAINSYSTEM_HPP
