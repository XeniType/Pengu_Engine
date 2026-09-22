#include "Pengu_Engine/Systems/TerrainSystem.hpp"
#include "Pengu_Engine/Components/TerrainComponent.hpp"
#include "Pengu_Engine/Components/TransformComponent.hpp"
#include "Pengu_Engine/PenguEngine.hpp"
#include "Pengu_Engine/Managers/ECS/ECSManager.hpp"
#include "../src/vendor/FastNoiseLite/FastNoiseLite.h"
#include "Pengu_Engine/Graphics/Mesh.hpp"

static int NUM_LODS = 4;

void TerrainSystem::update(float dt,
	ECSManager& world,
	Pengu::Core::PenguEngine& engine,
	Camera& camera)
{
	if (entities_.empty()) return;

	auto& jobSystem = engine.GetJobSystem();
	auto& rm = engine.GetResourceManager();

	const int MAX_UPLOADS_PER_FRAME = 1;  // Prevents GPU upload stutter
	const int MAX_DISPATCH_PER_FRAME = 2; // Prevents CPU string allocation stutter
	int chunksUploadedThisFrame = 0;
	int jobsDispatchedThisFrame = 0;

	for (auto const& settingsEntity : entities_) {
		auto& terrain = world.GetComponent<TerrainSettingsComponent>(settingsEntity);

		glm::vec3 camPos = camera.getPosition();
		int curX = (int)std::floor(camPos.x / (terrain.settings.chunkSize - 1.0f));
		int curZ = (int)std::floor(camPos.z / (terrain.settings.chunkSize - 1.0f));
		int renderRadius = terrain.viewDistance;
		int loadRadius = terrain.loadRadius;

		// --- 1. DESPAWNING (RECYCLING): Identify out-of-bounds chunks ---
		for (auto it = terrain.spawnedChunks.begin(); it != terrain.spawnedChunks.end(); ) {
			Entity chunkEntity = it->second;
			auto& chunkData = world.GetComponent<TerrainChunkComponent>(chunkEntity);
			if (!world.HasComponent<TerrainDrawableComponent>(chunkEntity))
				return;
			auto& drawable = world.GetComponent<TerrainDrawableComponent>(chunkEntity);

			int distX = std::abs(chunkData.chunkX - curX);
			int distZ = std::abs(chunkData.chunkZ - curZ);

			if (distX > loadRadius || distZ > loadRadius) {

				if (world.HasComponent<TerrainDrawableComponent>(chunkEntity)) {
					drawable.gameObj->bVisible = false;
				}
				terrain.chunkPool.push(chunkEntity);
				it = terrain.spawnedChunks.erase(it);
			}
			else if (distX > renderRadius || distZ > renderRadius) {
				if (world.HasComponent<TerrainDrawableComponent>(chunkEntity)) {
					drawable.gameObj->bVisible = false;
				}
				++it;
			}
			else
			{
				if (world.HasComponent<TerrainDrawableComponent>(chunkEntity))
				{
					drawable.gameObj->bVisible = true;
					int maxDist = std::max(distX, distZ);
					if (maxDist <= 1) {
						drawable.gameObj->SetActiveLOD(0);
					}
					else if (maxDist <= 2) {
						drawable.gameObj->SetActiveLOD(1);
					}
					else if (maxDist <= 4) {
						drawable.gameObj->SetActiveLOD(2);
					}
					else {
						drawable.gameObj->SetActiveLOD(3);
					}
					++it;
				}
			}
		}
		// Reload Chunks
		if (terrain.bReloadChunks) {

			for (auto it = terrain.spawnedChunks.begin(); it != terrain.spawnedChunks.end(); ) {
				Entity chunkEntity = it->second;
				auto& chunkData = world.GetComponent<TerrainChunkComponent>(chunkEntity);

				if (world.HasComponent<TerrainDrawableComponent>(chunkEntity)) {
					world.GetComponent<TerrainDrawableComponent>(chunkEntity).gameObj->bVisible = false;
				}
				terrain.chunkPool.push(chunkEntity);
				it = terrain.spawnedChunks.erase(it);
			}
			terrain.bReloadChunks = false;
		}

		// --- 2. POLLING: Check for finished Job results ---
		auto it = terrain.activeJobs.begin();
		while (it != terrain.activeJobs.end()) {

			if (chunksUploadedThisFrame >= MAX_UPLOADS_PER_FRAME) {
				ENGINE_INFO("Skipping Chunks For Next Frame");
				break; // Leave the rest of the jobs for the next frame
			}

			if (it->wait_for(std::chrono::seconds(0)) == std::future_status::ready) {

				TerrainDataPackage result = it->get();
				std::string key = std::to_string(result.x) + "," + std::to_string(result.z);
				Entity chunk;

				auto weightMap = std::make_shared<Pengu::Graphics::Texture>();
				weightMap->LoadFromMemory(result.weightMapPixels.data(), result.weightMapResolution, result.weightMapResolution, 4);

				// --- POOLING LOGIC: Reuse or Create ---
				if (!terrain.chunkPool.empty()) {
					chunk = terrain.chunkPool.front();
					terrain.chunkPool.pop();

					auto& drawable = world.GetComponent<TerrainDrawableComponent>(chunk);

					rm.updateGameObjectData(drawable.gameObj, result.vertices, result.lodIndices);

					drawable.gameObj->bVisible = false;

					auto& transform = world.GetComponent<TransformComponent>(chunk);
					transform.position_ = glm::vec3(result.x * terrain.settings.chunkSize, terrain.settings.seaLevel, result.z * terrain.settings.chunkSize);

					auto& chunkComp = world.GetComponent<TerrainChunkComponent>(chunk);
					chunkComp.chunkX = result.x;
					chunkComp.chunkZ = result.z;
					chunkComp.maxHeight = terrain.settings.maxHeight;
					chunkComp.chunkSize = terrain.settings.chunkSize;
					chunkComp.weightMap = weightMap;
				}
				else {
					auto go = rm.createGameObjectFromData(
						"Chunk_" + key,
						result.vertices,
						result.lodIndices,
						terrain.terrainMaterial
					);
					// Pool is empty, create a new entity
					chunk = world.CreateEntity();
					world.AddComponent(chunk, TerrainDrawableComponent(go));
					world.AddComponent(chunk, TransformComponent(
						glm::vec3(result.x * terrain.settings.chunkSize, terrain.settings.seaLevel, result.z * terrain.settings.chunkSize),
						glm::vec3(1.0f),
						glm::vec3(0.0f))
					);
					world.AddComponent(chunk, TerrainChunkComponent{ result.x, result.z , terrain.settings.maxHeight, terrain.settings.chunkSize, weightMap });
				}

				terrain.spawnedChunks[key] = chunk;
				terrain.loadingKeys.erase(key);
				it = terrain.activeJobs.erase(it);

				chunksUploadedThisFrame++;
			}
			else {
				++it;
			}
		}

		// --- 3. STREAMING: Request new chunks ---
		for (int x = curX - loadRadius; x <= curX + loadRadius && jobsDispatchedThisFrame < MAX_DISPATCH_PER_FRAME; ++x) {
			for (int z = curZ - loadRadius; z <= curZ + loadRadius && jobsDispatchedThisFrame < MAX_DISPATCH_PER_FRAME; ++z) {
				std::string key = std::to_string(x) + "," + std::to_string(z);

				if (terrain.spawnedChunks.count(key) == 0 && terrain.loadingKeys.count(key) == 0) {
					terrain.loadingKeys.insert(key);
					NoiseSettings localSettings = terrain.settings;

					terrain.activeJobs.push_back(jobSystem.Enqueue([=]() {
						return GenerateData(x, z, localSettings.vertexSize + 1, localSettings);
						}));

					jobsDispatchedThisFrame++;
				}
			}
		}
	}
}

std::shared_ptr<Pengu::Graphics::Texture> TerrainSystem::GenerateWarpNoiseTexture(int width, int height, int seed)
{
	FastNoiseLite noise;
	noise.SetSeed(seed);
	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	noise.SetFrequency(0.02f);

	std::vector<unsigned char> pixelData(width * height * 4); // Use 4 channels for consistency

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			float n1 = noise.GetNoise(static_cast<float>(x), static_cast<float>(y));
			unsigned char r = static_cast<unsigned char>((n1 + 1.0f) * 0.5f * 255.0f);

			float n2 = noise.GetNoise(static_cast<float>(x + 5000.0f), static_cast<float>(y + 5000.0f));
			unsigned char g = static_cast<unsigned char>((n2 + 1.0f) * 0.5f * 255.0f);

			int index = (y * width + x) * 4;

			pixelData[index + 0] = r;
			pixelData[index + 1] = g;
			pixelData[index + 2] = 0;
			pixelData[index + 3] = 255;
		}
	}

	auto texture = std::make_shared<Pengu::Graphics::Texture>();
	texture->LoadFromMemory(pixelData.data(), width, height, 4);

	return texture;
}

float TerrainSystem::GetHeight(float worldX, float worldZ, const TerrainSettingsComponent& settings)
{
	FastNoiseLite noise;
	noise.SetSeed(settings.settings.seed);
	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	noise.SetFractalType(FastNoiseLite::FractalType_FBm);
	noise.SetFractalOctaves(settings.settings.octaves);
	noise.SetFractalGain(settings.settings.persistence);
	noise.SetFractalLacunarity(settings.settings.lacunarity);

	auto GetRawHeight = [&](float wX, float wZ) {
		float mask = noise.GetNoise(wX / settings.settings.maskScale, wZ / settings.settings.maskScale);
		mask = (mask + 1.0f) * 0.5f;
		mask = mask * mask * (3.0f - 2.0f * mask);

		float offX = noise.GetNoise(wX * 0.8f, wZ * 0.8f) * settings.settings.warpIntensity;
		float offZ = noise.GetNoise((wX + 52.4f) * 0.8f, (wZ + 52.4f) * 0.8f) * settings.settings.warpIntensity;

		float wpX = (wX + offX) / settings.settings.scale;
		float wpZ = (wZ + offZ) / settings.settings.scale;

		float noiseVal = noise.GetNoise(wpX, wpZ);
		noiseVal = 1.0f - (noiseVal * noiseVal);

		float oceanDepth = -20.0f;
		float terrainHeight = std::pow(noiseVal, settings.settings.exponent) * settings.settings.maxHeight;

		float height = std::lerp(oceanDepth, terrainHeight, mask);

		float terraceHeight = 5.0f;
		float terraced = std::round(height / terraceHeight) * terraceHeight;

		float finalHeight = std::lerp(height, terraced, 0.4f);

		return finalHeight + settings.settings.seaLevel;
		};

	return GetRawHeight(worldX, worldZ);
}

TerrainDataPackage TerrainSystem::GenerateData(int x, int z, int vertexSize, NoiseSettings settings)
{
	TerrainDataPackage data;
	data.x = x;
	data.z = z;
	data.vertices.resize(vertexSize * vertexSize);

	data.weightMapResolution = vertexSize;
	data.weightMapPixels.resize(vertexSize * vertexSize * 4);

	FastNoiseLite noise;
	noise.SetSeed(settings.seed);
	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	noise.SetFractalType(FastNoiseLite::FractalType_FBm);
	noise.SetFractalOctaves(settings.octaves);
	noise.SetFractalGain(settings.persistence);
	noise.SetFractalLacunarity(settings.lacunarity);

	const int BLUR_RADIUS = 1;
	const float BLUR_STEP = 3.0f;

	float vertexSpacing = settings.chunkSize / static_cast<float>(vertexSize - 1);
	float logicalChunkSize = settings.chunkSize;

	for (int lZ = 0; lZ < vertexSize; ++lZ) {
		for (int lX = 0; lX < vertexSize; ++lX) {

			float localX = static_cast<float>(lX) * vertexSpacing;
			float localZ = static_cast<float>(lZ) * vertexSpacing;

			float worldX = (x * logicalChunkSize) + localX + settings.worldOffset.x;
			float worldZ = (z * logicalChunkSize) + localZ + settings.worldOffset.y;

			auto GetRawHeight = [&](float wX, float wZ) {
				// 1. Continent/Biome Mask (Using maskScale)
				float mask = noise.GetNoise(wX / settings.maskScale, wZ / settings.maskScale);

				mask = (mask + 1.0f) * 0.5f;

				mask = mask * mask * (3.0f - 2.0f * mask);

				// A. Domain Warping
				float offX = noise.GetNoise(wX * 0.8f, wZ * 0.8f) * settings.warpIntensity;
				float offZ = noise.GetNoise((wX + 52.4f) * 0.8f, (wZ + 52.4f) * 0.8f) * settings.warpIntensity;

				float wpX = (wX + offX) / settings.scale;
				float wpZ = (wZ + offZ) / settings.scale;

				// B. Ridge Noise
				float noiseVal = noise.GetNoise(wpX, wpZ);
				noiseVal = 1.0f - (noiseVal * noiseVal);

				// C. Exponent & Height (Multiply by the Mask!)
				// If mask is 0 (ocean/flat), height becomes 0. If mask is 1, full mountain height.
				float oceanDepth = -20.0f;
				float terrainHeight = std::pow(noiseVal, settings.exponent) * settings.maxHeight;

				float height = std::lerp(oceanDepth, terrainHeight, mask);

				// D. Terracing (Water erosion steps)
				float terraceHeight = 5.0f;
				float terraced = std::round(height / terraceHeight) * terraceHeight;

				// E. Blend terracing and apply Sea Level
				float finalHeight = std::lerp(height, terraced, 0.4f);

				return finalHeight + settings.seaLevel;
				};

			auto GetTerrainHeight = [&](float wX, float wZ) {
				if (BLUR_RADIUS <= 0) return GetRawHeight(wX, wZ);

				float sum = 0.0f;
				int count = 0;

				for (int dz = -BLUR_RADIUS; dz <= BLUR_RADIUS; ++dz) {
					for (int dx = -BLUR_RADIUS; dx <= BLUR_RADIUS; ++dx) {
						float sampleX = wX + (dx * BLUR_STEP);
						float sampleZ = wZ + (dz * BLUR_STEP);

						sum += GetRawHeight(sampleX, sampleZ);
						count++;
					}
				}

				return sum / static_cast<float>(count);
				};

			float h = GetTerrainHeight(worldX, worldZ);
			int idx = (lZ * vertexSize) + lX;

			auto& v = data.vertices[(lZ * vertexSize) + lX];
			v.position = glm::vec3(localX, h, localZ);
			v.uv = glm::vec2(lX / static_cast<float>(vertexSize - 1), lZ / static_cast<float>(vertexSize - 1));

			float eps = 1.0f;

			float hL = GetTerrainHeight(worldX - eps, worldZ);
			float hR = GetTerrainHeight(worldX + eps, worldZ);
			float hD = GetTerrainHeight(worldX, worldZ - eps);
			float hU = GetTerrainHeight(worldX, worldZ + eps);

			v.normal = glm::normalize(glm::vec3(hL - hR, 2.0f * eps, hD - hU));

			// Tangent/Bitangent calculation for grid
			v.tangent = glm::normalize(glm::vec3(2.0f * eps, hR - hL, 0.0f));
			v.bitangent = glm::normalize(glm::vec3(0.0f, hU - hD, 2.0f * eps));

			float slope = 1.0f - v.normal.y;
			float splatNoise = noise.GetNoise(worldX * 3.0f, worldZ * 3.0f);

			float snowLineBase = settings.seaLevel + (settings.maxHeight * 0.40f);

			float waterLevel = settings.seaLevel;


			float rockStart = 0.3f + (splatNoise * 0.04f);
			float snowStart = snowLineBase + (splatNoise * 3.5f);
			float dirtThreshold = settings.seaLevel + (splatNoise * 1.5f);

			float sandWeight = glm::smoothstep(waterLevel + 6.0f, waterLevel + 2.0f, h);
			float rockWeight = glm::smoothstep(rockStart, rockStart + 0.15f, slope);
			float snowWeight = glm::smoothstep(snowStart, snowStart + 2.0f, h);
			float combineOtherWeights = glm::clamp(rockWeight + snowWeight + sandWeight, 0.0f, 1.0f);

			float grassWeight = glm::smoothstep(waterLevel + 4.0f, waterLevel + 10.0f, h);
			grassWeight *= (1.0f - rockWeight);
			grassWeight *= (1.0f - snowWeight);

			float total = grassWeight + rockWeight + sandWeight + snowWeight;
			if (total > 1.0f)
			{
				float invTotal = 1.0f / total;
				grassWeight *= invTotal;
				rockWeight *= invTotal;
				sandWeight *= invTotal;
				snowWeight *= invTotal;
			}

			data.weightMapPixels[idx * 4 + 0] = static_cast<unsigned char>(glm::clamp(grassWeight, 0.0f, 1.0f) * 255.0f);
			data.weightMapPixels[idx * 4 + 1] = static_cast<unsigned char>(glm::clamp(rockWeight, 0.0f, 1.0f) * 255.0f);
			data.weightMapPixels[idx * 4 + 2] = static_cast<unsigned char>(glm::clamp(sandWeight, 0.0f, 1.0f) * 255.0f);
			data.weightMapPixels[idx * 4 + 3] = static_cast<unsigned char>(glm::clamp(snowWeight, 0.0f, 1.0f) * 255.0f);
		}
	}

	data.lodIndices.resize(NUM_LODS);

	for (int lod = 0; lod < NUM_LODS; ++lod) {
		int step = 1 << lod; // Step sizes: 1, 2, 4, 8

		for (int zIdx = 0; zIdx < vertexSize - step; zIdx += step) {
			for (int xIdx = 0; xIdx < vertexSize - step; xIdx += step) {
				int start = (zIdx * vertexSize) + xIdx;

				// Triangle 1
				data.lodIndices[lod].push_back(start);
				data.lodIndices[lod].push_back(start + (vertexSize * step));
				data.lodIndices[lod].push_back(start + step);

				// Triangle 2
				data.lodIndices[lod].push_back(start + step);
				data.lodIndices[lod].push_back(start + (vertexSize * step));
				data.lodIndices[lod].push_back(start + (vertexSize * step) + step);
			}
		}
	}

	int skirtStart = static_cast<int>(data.vertices.size());
	float skirtDrop = 25.0f;

	// Top Edge (Z = 0)
	for (int x = 0; x < vertexSize; ++x) {
		Pengu::Graphics::Vertex v = data.vertices[x]; v.position.y -= skirtDrop; data.vertices.push_back(v);
	}
	// Bottom Edge (Z = vertexSize - 1)
	for (int x = 0; x < vertexSize; ++x) {
		Pengu::Graphics::Vertex v = data.vertices[(vertexSize - 1) * vertexSize + x]; v.position.y -= skirtDrop; data.vertices.push_back(v);
	}
	// Left Edge (X = 0)
	for (int z = 0; z < vertexSize; ++z) {
		Pengu::Graphics::Vertex v = data.vertices[z * vertexSize]; v.position.y -= skirtDrop; data.vertices.push_back(v);
	}
	// Right Edge (X = vertexSize - 1)
	for (int z = 0; z < vertexSize; ++z) {
		Pengu::Graphics::Vertex v = data.vertices[z * vertexSize + (vertexSize - 1)]; v.position.y -= skirtDrop; data.vertices.push_back(v);
	}

	// --- 3. Stitch the Skirts to the LODs ---
	for (int lod = 0; lod < NUM_LODS; ++lod) {
		int step = 1 << lod;

		// Stitch Top Skirt
		for (int x = 0; x < vertexSize - step; x += step) {
			int topL = x;                           int topR = x + step;
			int botL = skirtStart + x;              int botR = skirtStart + x + step;
			data.lodIndices[lod].push_back(topL);   data.lodIndices[lod].push_back(topR);   data.lodIndices[lod].push_back(botL);
			data.lodIndices[lod].push_back(topR);   data.lodIndices[lod].push_back(botR);   data.lodIndices[lod].push_back(botL);
		}

		// Stitch Bottom Skirt
		int zOff = (vertexSize - 1) * vertexSize;
		int sOff = skirtStart + vertexSize;
		for (int x = 0; x < vertexSize - step; x += step) {
			int topL = zOff + x;                    int topR = zOff + x + step;
			int botL = sOff + x;                    int botR = sOff + x + step;
			data.lodIndices[lod].push_back(topL);   data.lodIndices[lod].push_back(botL);   data.lodIndices[lod].push_back(topR);
			data.lodIndices[lod].push_back(topR);   data.lodIndices[lod].push_back(botL);   data.lodIndices[lod].push_back(botR);
		}

		// Stitch Left Skirt
		sOff = skirtStart + (vertexSize * 2);
		for (int z = 0; z < vertexSize - step; z += step) {
			int topL = z * vertexSize;              int topR = (z + step) * vertexSize;
			int botL = sOff + z;                    int botR = sOff + z + step;
			data.lodIndices[lod].push_back(topL);   data.lodIndices[lod].push_back(botL);   data.lodIndices[lod].push_back(topR);
			data.lodIndices[lod].push_back(topR);   data.lodIndices[lod].push_back(botL);   data.lodIndices[lod].push_back(botR);
		}

		// Stitch Right Skirt
		sOff = skirtStart + (vertexSize * 3);
		for (int z = 0; z < vertexSize - step; z += step) {
			int topL = z * vertexSize + (vertexSize - 1);       int topR = (z + step) * vertexSize + (vertexSize - 1);
			int botL = sOff + z;                                int botR = sOff + z + step;
			data.lodIndices[lod].push_back(topL);   data.lodIndices[lod].push_back(topR);   data.lodIndices[lod].push_back(botL);
			data.lodIndices[lod].push_back(topR);   data.lodIndices[lod].push_back(botR);   data.lodIndices[lod].push_back(botL);
		}
	}
	return data;
}