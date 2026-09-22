/**
 * @file TerrainComponent.hpp
 * @brief Components and structures for terrain generation and management.
 */

#ifndef TERRAINCOMPONENT_HPP
#define TERRAINCOMPONENT_HPP 1

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <memory>
#include <unordered_set>
#include <future>
#include <queue>
#include <string>

#include "Pengu_Engine/Managers/ECS/ECSManager.hpp"

namespace Pengu::Graphics { class Material; class Texture; struct Vertex; }

/**
 * @struct TerrainTextureLayer
 * @brief Represents a texture layer for terrain blending.
 */
struct TerrainTextureLayer {
	/** @brief Name of the layer. */
	std::string name;
	/** @brief Albedo map texture. */
	std::shared_ptr<Pengu::Graphics::Texture> albedoMap;
	/** @brief Normal map texture. */
	std::shared_ptr<Pengu::Graphics::Texture> normalMap;
	/** @brief Roughness map texture. */
	std::shared_ptr<Pengu::Graphics::Texture> roughnessMap;
	/** @brief Metallic map texture. */
	std::shared_ptr<Pengu::Graphics::Texture> metallicMap;
	/** @brief Ambient Occlusion map texture. */
	std::shared_ptr<Pengu::Graphics::Texture> aoMap;

	/** @brief Tiling factor for the textures. */
	float tiling = 20.0f;
	/** @brief Strength of the layer blending. */
	float strength = 1.0f;
};

/**
 * @struct TerrainDataPackage
 * @brief Data package containing generated terrain mesh data.
 */
struct TerrainDataPackage {
	/** @brief Chunk X coordinate. */
	int x;
	/** @brief Chunk Z coordinate. */
	int z;
	/** @brief Vertices of the terrain mesh. */
	std::vector<Pengu::Graphics::Vertex> vertices;
	/** @brief LOD indices for the terrain mesh. */
	std::vector<std::vector<unsigned int>> lodIndices;

	/** @brief Pixels for the weight map. */
	std::vector<unsigned char> weightMapPixels;
	/** @brief Resolution of the weight map. */
	int weightMapResolution;
};

/**
 * @struct NoiseSettings
 * @brief Settings for noise generation used in terrain.
 */
struct NoiseSettings {
	/** @brief Scale of the noise. */
	float scale = 10.0f;
	/** @brief Maximum height of the terrain. */
	float maxHeight = 50.0f;
	/** @brief Number of octaves for fractal noise. */
	int octaves = 2;
	/** @brief Persistence of the noise. */
	float persistence = 0.5f;
	/** @brief Lacunarity of the noise. */
	float lacunarity = 2.0f;

	/** @brief Exponent for height distribution. */
	float exponent = 2.0f;
	/** @brief Sea level offset. */
	float seaLevel = 0.0f;
	/** @brief Scale for the mask. */
	float maskScale = 500.0f;
	/** @brief World offset for noise sampling. */
	glm::vec2 worldOffset = { 0,0 };
	/** @brief Intensity of domain warping. */
	float warpIntensity = 15.0f;
	/** @brief Size of each chunk. */
	float chunkSize = 64.0f;

	/** @brief Number of vertices per side. */
	int vertexSize = 16;
	/** @brief Seed for noise generation. */
	int seed = 00000;

	/** @brief Texture containing noise data. */
	std::shared_ptr<Pengu::Graphics::Texture> noiseTextureID;
};

/**
 * @struct TerrainSettingsComponent
 * @brief Component containing global settings for terrain generation.
 */
struct TerrainSettingsComponent {
	/** @brief Noise settings for height generation. */
	NoiseSettings settings;

	/** @brief Base texture layer for the terrain. */
	TerrainTextureLayer baseLayer;

	/** @brief Additional texture layers. */
	std::vector<TerrainTextureLayer> layers;
	/** @brief Global weight map for layer blending. */
	std::shared_ptr<Pengu::Graphics::Texture> globalWeightMap;

	/** @brief Radius for loading chunks around the player. */
	int loadRadius = 5;
	/** @brief Distance for viewing chunks. */
	int viewDistance = 3;
	/** @brief Material used for terrain rendering. */
	std::shared_ptr<Pengu::Graphics::Material> terrainMaterial;

	// Trackers for the system to use
	/** @brief Keys of chunks currently loading. */
	std::unordered_set<std::string> loadingKeys;
	/** @brief Map of spawned chunk entities. */
	std::unordered_map<std::string, Entity> spawnedChunks;
	/** @brief List of active generation jobs. */
	std::vector<std::shared_future<TerrainDataPackage>> activeJobs;

	/** @brief Pool of entities for chunk reuse. */
	std::queue<Entity> chunkPool;
	/** @brief Flag to trigger chunk reloading. */
	bool bReloadChunks = false;
};

/**
 * @struct TerrainChunkComponent
 * @brief Component identifying an entity as a terrain chunk.
 */
struct TerrainChunkComponent {
	/** @brief Chunk X coordinate. */
	int chunkX;
	/** @brief Chunk Z coordinate. */
	int chunkZ;
	/** @brief Maximum height of this chunk. */
	float maxHeight;
	/** @brief Size of this chunk. */
	float chunkSize;
	/** @brief Weight map texture for this chunk. */
	std::shared_ptr<Pengu::Graphics::Texture> weightMap;
};

#endif // !TERRAINCOMPONENT_HPP
