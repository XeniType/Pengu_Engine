#include "scenes/DeferredScene.hpp"
#include "Pengu_Engine/Managers/Resource/ResourceManager.hpp"
#include "Pengu_Engine/Graphics/Rendering/Techniques/ForwardShad.hpp"
#include "Pengu_Engine/Components/TransformComponent.hpp"
#include "Pengu_Engine/Components/WaterComponent.hpp"
#include "Pengu_Engine/Systems/TerrainSystem.hpp"
#include "Pengu_Engine/Objects/skybox.hpp"

namespace Pengu::Scene {
	void DeferredScene::update(float dt)
	{

	}

	void DeferredScene::onLoad(Pengu::Resources::ResourceManager& rm)
	{
		m_resourceManager = &rm;
		onInitialize();
		auto& world = getWorld();
		auto& camera = m_engine->GetCamera();

		camera.position_ = glm::vec3(0.0f, 5.0f, 0.0f);

		auto mat = rm.getMaterial("PlaneMat");
		mat->albedoMap = rm.getTexture("../assets/textures/terrain/rock/rock_diff.png");
		mat->normalMap = rm.getTexture("../assets/textures/terrain/rock/rock_nor.png");;
		mat->roughnessMap = rm.getTexture("../assets/textures/terrain/rock/rock_rough.png");;
		mat->aoMap = rm.getTexture("../assets/textures/terrain/rock/rock_ao.png");;

		auto plane = rm.CreatePlane("plane", 2, 2, 100, mat);

		auto cupModel = rm.loadObject("../assets/object/cup/cup.obj");

		auto cup = world.CreateEntity();
		world.AddComponent(cup, DrawableComponent(cupModel));
		world.AddComponent(cup, TransformComponent{
				glm::vec3(0.0f,2.0f,0.0f),
				glm::vec3(10.0f),
				glm::vec3(0.0f)
			});
		world.AddComponent(cup, TagComponent("Cup"));

		auto groud = world.CreateEntity();
		world.AddComponent(groud, DrawableComponent(plane));
		world.AddComponent(groud, TransformComponent(
			glm::vec3(50.0f, 0.0f, 50.f),
			glm::vec3(1.0f),
			glm::vec3(0.0f)
		));
		world.AddComponent(groud, TagComponent("Ground"));

		auto sun = world.CreateEntity();
		world.AddComponent(sun, Lights(LightType::E_Directional));
		world.GetComponent<Lights>(sun).direction_ = glm::vec3(0.0f, -1.0f, 1.0f);
		world.GetComponent<Lights>(sun).intensity_ = 2.5f;
		world.GetComponent<Lights>(sun).shadowSoftness_ = 50.0f;
		world.AddComponent(sun, TagComponent("Sun"));

	}

	void DeferredScene::onUnload()
	{


	}

	void DeferredScene::onInitialize()
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
