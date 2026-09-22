#include "scenes/LightingScene.hpp"
#include "Pengu_Engine/Managers/Resource/ResourceManager.hpp"
#include "Pengu_Engine/Graphics/Rendering/Techniques/ForwardShad.hpp"
#include "Pengu_Engine/Components/TransformComponent.hpp"
#include "Pengu_Engine/Components/WaterComponent.hpp"
#include "Pengu_Engine/Systems/TerrainSystem.hpp"
#include "Pengu_Engine/Objects/skybox.hpp"

namespace Pengu::Scene {
	void LightingScene::update(float dt)
	{
		static float timeOfDay = 0.0f;

		timeOfDay += dt * 0.0005f;

		auto& world = getWorld();

		auto sunEntity = world.GetEntityByTag("Directional");

		if (world.HasComponent<Lights>(sunEntity))
		{
			auto& sunLight = world.GetComponent<Lights>(sunEntity);

			float x = std::cos(timeOfDay);
			float y = std::sin(timeOfDay);

			float z = 0.2f;

			sunLight.direction_ = glm::normalize(glm::vec3(-x, -y, -z));
			glm::vec3 middayColor = glm::vec3(1.0f, 0.96f, 0.9f);
			glm::vec3 sunsetColor = glm::vec3(1.0f, 0.35f, 0.0f);
			glm::vec3 nightColor = glm::vec3(0.05f, 0.05f, 0.15f);

			glm::vec3 finalColor;

			if (y > 0.2f)
			{
				float blend = glm::smoothstep(0.2f, 0.6f, y);
				finalColor = glm::mix(sunsetColor, middayColor, blend);
			}
			else if (y > -0.1f)
			{
				float blend = glm::smoothstep(-0.1f, 0.2f, y);
				finalColor = glm::mix(nightColor, sunsetColor, blend);
			}
			else
			{
				finalColor = nightColor;
			}

			sunLight.color_ = finalColor;
		}

	}

	void LightingScene::onLoad(Pengu::Resources::ResourceManager& rm)
	{
		m_resourceManager = &rm;
		onInitialize();
		auto& world = getWorld();
		auto& camera = m_engine->GetCamera();

		camera.position_ = glm::vec3(-5.0f, 10.0f, 25.0f);

		auto mat = rm.getMaterial("Ground");
		mat->albedoColor = glm::vec4(1.0f);
		auto plane = rm.CreatePlane("Ground", 100, 100, 1, mat);

		auto ground = world.CreateEntity();
		world.AddComponent(ground, DrawableComponent(plane));
		world.AddComponent(ground, TransformComponent());
		world.AddComponent(ground, TagComponent("Ground"));

		rm.LoadObjectAsync("../assets/object/frieren/frieren.gltf", [this](std::shared_ptr<Pengu::Graphics::GameObject> model)
			{
				if (!model) {
					LOG_ERROR("Failed to load frieren.gltf");
					return;
				}
				auto& world = getWorld();

				auto entity = world.CreateEntity();
				world.AddComponent(entity, DrawableComponent(model));
				world.AddComponent(entity, TransformComponent{
						glm::vec3(7.0f,8.0f,0.0f),
						glm::vec3(2.0f),
						glm::vec3(-90.0f,-90.0f,0.0f)
					});
				world.AddComponent(entity, TagComponent("frieren_1"));

				entity = world.CreateEntity();
				world.AddComponent(entity, DrawableComponent(model));
				world.AddComponent(entity, TransformComponent{
						glm::vec3(-7.0f,8.0f,0.0f),
						glm::vec3(2.0f),
						glm::vec3(-90.0f,90.0f,0.0f)
					});
				world.AddComponent(entity, TagComponent("frieren_2"));

				entity = world.CreateEntity();
				world.AddComponent(entity, DrawableComponent(model));
				world.AddComponent(entity, TransformComponent{
						glm::vec3(0.0f,8.0f,7.0f),
						glm::vec3(2.0f),
						glm::vec3(-90.0f,180.0f,0.0f)
					});
				world.AddComponent(entity, TagComponent("frieren_3"));

				entity = world.CreateEntity();
				world.AddComponent(entity, DrawableComponent(model));
				world.AddComponent(entity, TransformComponent{
						glm::vec3(0.0f,8.0f,-7.0f),
						glm::vec3(2.0f),
						glm::vec3(-90.0f,0.0f,0.0f)
					});
				world.AddComponent(entity, TagComponent("frieren_4"));

				entity = world.CreateEntity();
				world.AddComponent(entity, DrawableComponent(model));
				world.AddComponent(entity, TransformComponent{
						glm::vec3(0.0f,8.0f,30.0f),
						glm::vec3(2.0f),
						glm::vec3(-90.0f,0.0f,0.0f)
					});
				world.AddComponent(entity, TagComponent("frieren_5"));
			});

		auto sun = world.CreateEntity();
		world.AddComponent(sun, Lights(LightType::E_Directional));
		world.AddComponent(sun, TagComponent("Directional"));

		auto point = world.CreateEntity();
		world.AddComponent(point, Lights(LightType::E_Point));
		world.AddComponent(point, TagComponent("Point"));
		world.GetComponent<Lights>(point).color_ = glm::vec3(0.9f, 0.9f, 0.98f);
		world.GetComponent<Lights>(point).position_ = glm::vec3(0.0f, 10.0f, 0.0f);
		world.GetComponent<Lights>(point).radius_ = 30.0f;


		auto spot = world.CreateEntity();
		world.AddComponent(spot, Lights(LightType::E_Spot));
		world.AddComponent(spot, TagComponent("Spot"));
		world.GetComponent<Lights>(spot).color_ = glm::vec3(0.0f, 0.0f, 1.0f);
		world.GetComponent<Lights>(spot).position_ = glm::vec3(-11.0f, 15.0f, 25.0f);
		world.GetComponent<Lights>(spot).direction_ = glm::vec3(0.3f, -0.4f, 0.2f);
		world.GetComponent<Lights>(spot).innerCutoff_ = 1.0f;
		world.GetComponent<Lights>(spot).outerCutoff_ = 50.0f;

	}

	void LightingScene::onUnload()
	{


	}

	void LightingScene::onInitialize()
	{
		auto& world = getWorld();

		// 1. ECS Setup
		world.RegisterComponent<DrawableComponent>();
		world.RegisterComponent<TransformComponent>();
		world.RegisterComponent<Lights>();
		world.RegisterComponent<TagComponent>();

		// 2. Systems Setup
		auto lightsys = world.RegisterSystem<LightSystem>();
		{
			Signature signature;
			signature.set(world.GetComponentType<Lights>());
			world.SetSystemSignature<LightSystem>(signature);
		}
		setLightSystem(lightsys);

	}
}
