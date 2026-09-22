#include "scenes/Demo_Scene.hpp"
#include "Pengu_Engine/Managers/Resource/ResourceManager.hpp"
#include "Pengu_Engine/Graphics/Rendering/Techniques/ForwardShad.hpp"
#include "Pengu_Engine/Components/TransformComponent.hpp"
#include "Pengu_Engine/Components/WaterComponent.hpp"
#include "Pengu_Engine/Systems/TerrainSystem.hpp"
#include "Pengu_Engine/Objects/skybox.hpp"

namespace Pengu::Scene {
	void DemoScene::update(float dt)
	{

		if (m_engine->GetInput().isPressed(Action::F))
		{
			if (bFollowTerrain) {
				bFollowTerrain = false;
			}
			else {
				bFollowTerrain = true;
			}
		}

		if (m_terrainSystem.get())
		{
			m_terrainSystem->update(dt, getWorld(), *m_engine, m_engine->GetCamera());
		}

		if (bFollowTerrain) {
			auto settingsEntities = getWorld().GetEntitiesWith<TerrainSettingsComponent>();
			if (!settingsEntities.empty()) {
				auto& terrainSettings = getWorld().GetComponent<TerrainSettingsComponent>(settingsEntities[0]);
				float lookAheadDistance = m_engine->GetCamera().movSpeed_ * lookAheadTime;

				float futureZ = m_engine->GetCamera().position_.z - lookAheadDistance;

				float currentTerrainHeight = TerrainSystem::GetHeight(m_engine->GetCamera().position_.x, m_engine->GetCamera().position_.z, terrainSettings);
				float futureTerrainHeight = TerrainSystem::GetHeight(m_engine->GetCamera().position_.x, futureZ, terrainSettings);

				float targetTerrainHeight = std::max(currentTerrainHeight, futureTerrainHeight);
				float targetY = targetTerrainHeight + terrainOffset;

				m_engine->GetCamera().position_.y = glm::mix(m_engine->GetCamera().position_.y, targetY, lerpFactor);
				m_engine->GetCamera().position_.z -= m_engine->GetCamera().movSpeed_ * dt;
			}
		}
	}

	void DemoScene::onLoad(Pengu::Resources::ResourceManager& rm)
	{
		m_resourceManager = &rm;
		onInitialize();
		auto& world = getWorld();
		auto& camera = m_engine->GetCamera();

		camera.position_ = glm::vec3(0.0f, 500.0f, 0.0f);

		auto sun = world.CreateEntity();
		world.AddComponent(sun, Lights(LightType::E_Directional));
		world.AddComponent(sun, TagComponent("Sun"));

		auto& sunLight = world.GetComponent<Lights>(sun);
		sunLight.direction_ = glm::vec3(1.0f, -1.0f, -0.0f);
		sunLight.color_ = glm::vec3(1.0f, 0.95f, 0.9f);
		sunLight.intensity_ = 10.0f;

		sunLight.shadowSoftness_ = 0.5f;
	}

	void DemoScene::onUnload()
	{


	}

	void DemoScene::onInitialize()
	{
		auto& world = getWorld();

		// 1. ECS Setup
		world.RegisterComponent<DrawableComponent>();
		world.RegisterComponent<TransformComponent>();
		world.RegisterComponent<Lights>();
		world.RegisterComponent<TagComponent>();
		world.RegisterComponent<TerrainSettingsComponent>();
		world.RegisterComponent<TerrainChunkComponent>();
		world.RegisterComponent<TerrainDrawableComponent>();
		world.RegisterComponent<Pengu::Components::WaterComponent>();

		// 2. Systems Setup
		auto lightsys = world.RegisterSystem<LightSystem>();
		{
			Signature signature;
			signature.set(world.GetComponentType<Lights>());
			world.SetSystemSignature<LightSystem>(signature);
		}
		setLightSystem(lightsys);

		m_terrainSystem = world.RegisterSystem<TerrainSystem>();
		{
			Signature signature;
			signature.set(world.GetComponentType<TerrainSettingsComponent>());
			world.SetSystemSignature<TerrainSystem>(signature);
		}

		auto terrainEntity = world.CreateEntity();
		TerrainSettingsComponent terrainConfig;
		{
			/* Base Layer*/
			terrainConfig.baseLayer =
			{
				TerrainTextureLayer{
					.name = "Dirt",
					.albedoMap = m_resourceManager->getTexture("../assets/textures/terrain/dirt.png"),
					.tiling = 15.0f
				}
			};

			// Layer 0: Grass (Red channel)
			terrainConfig.layers.push_back(
				TerrainTextureLayer{
					.name = "Grass",
					.albedoMap = m_resourceManager->getTexture("../assets/textures/terrain/grass.png"),
					.tiling = 10.0f
				}
			);

			// Layer 1: Rock (Green channel - also used for Slope in shader)
			terrainConfig.layers.push_back(
				TerrainTextureLayer{
					.name = "Rock",
					.albedoMap = m_resourceManager->getTexture("../assets/textures/terrain/rock.png"),
					.tiling = 10.0f
				}
			);

			// Layer 2: Sand (Blue channel)
			terrainConfig.layers.push_back(
				TerrainTextureLayer{
					.name = "Sand",
					.albedoMap = m_resourceManager->getTexture("../assets/textures/terrain/sand.png"),
					.tiling = 20.0f
				}
			);

			// Layer 3: Snow (Alpha channel - also used for Height in shader)
			terrainConfig.layers.push_back(
				TerrainTextureLayer{
					.name = "Snow",
					.albedoMap = m_resourceManager->getTexture("../assets/textures/terrain/snow.png"),
					.tiling = 5.0f
				}
			);
		}

		// Apply your specific LightScene settings
		terrainConfig.settings.scale = 50.5f;
		terrainConfig.settings.maxHeight = 1000.0f;
		terrainConfig.settings.octaves = 6;
		terrainConfig.settings.persistence = 0.50f;
		terrainConfig.settings.lacunarity = 2.00f;
		terrainConfig.settings.exponent = 6.5f;
		terrainConfig.settings.seaLevel = 0.25f;
		terrainConfig.settings.maskScale = 600.0f;
		terrainConfig.settings.warpIntensity = 30.0f;
		terrainConfig.settings.noiseTextureID = m_terrainSystem->GenerateWarpNoiseTexture();

		terrainConfig.settings.chunkSize = 128;
		terrainConfig.settings.vertexSize = 64;

		// Set view distance and material
		terrainConfig.loadRadius = 20;
		terrainConfig.viewDistance = 15;

		auto mat = m_resourceManager->getMaterial("Grass");
		mat->albedoColor = glm::vec4(0.0f, 1.0f, 0.3f, 1.0f);
		terrainConfig.terrainMaterial = mat;

		// Attach it to the entity
		world.AddComponent(terrainEntity, TerrainSettingsComponent());
		world.GetComponent<TerrainSettingsComponent>(terrainEntity) = terrainConfig;
		world.AddComponent(terrainEntity, TagComponent("Terrain Manager"));

		// 3. Water Setup
		auto waterEntity = world.CreateEntity();
		Pengu::Components::WaterComponent water;
		water.height = terrainConfig.settings.seaLevel;
		water.color = { 0.1f, 0.4f, 0.7f, 0.6f };
		water.waveSpeed = 1.0f;
		water.waveStrength = 0.1f;
		water.tiling = 0.0f;
		water.reflectivity = 0.180f;
		water.maxStep = 280;

		water.ssrEnabled = true;

		water.noiseTexture = terrainConfig.settings.noiseTextureID;

		world.AddComponent(waterEntity, water);
		world.AddComponent(waterEntity, TagComponent("Water"));

		std::vector<std::string> skyboxFaces = {
				"../assets/textures/skybox/px.png",
				"../assets/textures/skybox/nx.png",
				"../assets/textures/skybox/py.png",
				"../assets/textures/skybox/ny.png",
				"../assets/textures/skybox/pz.png",
				"../assets/textures/skybox/nz.png"
		};

		SetSkyBox(std::make_unique<SkyBox>(skyboxFaces));
	}
}
