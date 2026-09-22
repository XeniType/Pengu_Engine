#include "Pengu_Engine/Scene/Scene.hpp"
#include "Pengu_Engine/Managers/Resource/ResourceManager.hpp"

namespace Pengu::Scene {
	void Scene::update(float dt)
	{
		auto& world = getWorld();
		auto& inp = m_engine->GetInput();
		auto& camera = m_engine->GetCamera();

		if (inp.isPressed(Action::C)) {
			auto FlashLight = world.CreateEntity();
			world.AddComponent(FlashLight, Lights(LightType::E_Spot));
			world.AddComponent(FlashLight, TransformComponent(camera.position_, glm::vec3(1.0f), glm::normalize(camera.front_)));
			world.GetComponent<Lights>(FlashLight).color_ = glm::vec3(0.5f, 0.5f, 0.5f);
			world.GetComponent<Lights>(FlashLight).radius_ = 50.0f;
			world.GetComponent<Lights>(FlashLight).innerCutoff_ = glm::cos(glm::radians(12.0f));
			world.GetComponent<Lights>(FlashLight).outerCutoff_ = glm::cos(glm::radians(17.0f));
			world.GetComponent<Lights>(FlashLight).shadowSoftness_ = 0.5f;
		}
	}

	void Scene::onLoad(Pengu::Resources::ResourceManager& rm)
	{
		auto& world = getWorld();

		getWorld().RegisterComponent<DrawableComponent>();
		getWorld().RegisterComponent<TransformComponent>();
		getWorld().RegisterComponent<Lights>();

		auto lightsys = getWorld().RegisterSystem<LightSystem>();
		{
			Signature signature;
			signature.set(getWorld().GetComponentType<Lights>());
			signature.set(getWorld().GetComponentType<TransformComponent>());
			getWorld().SetSystemSignature<LightSystem>(signature);
		}
		setLightSystem(lightsys);

		auto planeMat = rm.getMaterial("Plane_Material");
		planeMat->albedoMap = nullptr;
		planeMat->albedoColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		auto plane = rm.CreatePlane("Plane", 100, 100, 1, planeMat);

		auto cubeMat = rm.getMaterial("Cube_Material");
		cubeMat->albedoMap = nullptr;
		cubeMat->albedoColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		auto cube8 = rm.CreateCube8v("Cube_8", 2.5f, cubeMat);

		cubeMat->albedoColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		auto cube24 = rm.CreateCube24v("Cube_24", 2.5f, cubeMat);

		auto panda = rm.loadObject("../resource/object/redpanda/red-panda.obj");

		auto player = getWorld().CreateEntity();

		getWorld().AddComponent(player, DrawableComponent(panda));
		getWorld().AddComponent(player, TransformComponent(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.5f), glm::vec3(0.0f)));

		auto floor = getWorld().CreateEntity();

		getWorld().AddComponent(floor, DrawableComponent(plane));
		getWorld().AddComponent(floor, TransformComponent(glm::vec3(-1.0f), glm::vec3(1.0f), glm::vec3(0.0f)));

		auto box = getWorld().CreateEntity();

		getWorld().AddComponent(box, DrawableComponent(cube8));
		getWorld().AddComponent(box, TransformComponent(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f)));

		box = getWorld().CreateEntity();

		getWorld().AddComponent(box, DrawableComponent(cube24));
		getWorld().AddComponent(box, TransformComponent(glm::vec3(-5.0f, 0.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f)));
	}


	void Scene::onUnload()
	{


	}
}
