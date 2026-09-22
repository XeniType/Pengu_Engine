#include "ScriptingScene.hpp"
#include "Pengu_Engine/Graphics/Rendering/Techniques/Unlit.hpp"
#include "Pengu_Engine/Components/TerrainComponent.hpp"
#include <random>

void Pengu::Scene::ScriptingScene::update(float dt)
{
	scriptsys->Update(dt);
}

void Pengu::Scene::ScriptingScene::onLoad(Pengu::Resources::ResourceManager& rm)
{
	m_resourceManager = &rm;
	onInitialize();
	auto& world = getWorld();

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distrib(-50, 50);


	auto frieren = rm.loadObject("../assets/object/frieren/frieren.gltf");

	std::vector<Entity> entities(100);

	for (auto& entity : entities) {
		entity = world.CreateEntity();

		world.AddComponent(entity, DrawableComponent(frieren));
		world.AddComponent(entity, TransformComponent{ glm::vec3(distrib(gen), distrib(gen), distrib(gen)),glm::vec3(1.0f), glm::vec3(-1.57f,0.0f,0.0f) });
		world.AddComponent(entity, ScriptingComponent("../assets/scripts/entities.lua"));
	}

	auto panda = rm.loadObject("../assets/object/redpanda/red-panda.obj");

	auto player = world.CreateEntity();

	world.AddComponent(player, DrawableComponent(panda));
	world.AddComponent(player, TransformComponent(glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f)));
	world.AddComponent(player, ScriptingComponent("../assets/scripts/player.lua"));
	world.AddComponent(player, TagComponent("Player"));

	scriptsys->Init(&m_engine->GetInput());

	scriptsys->Start();
}

void Pengu::Scene::ScriptingScene::onUnload()
{
	scriptsys->Finish();
}

void Pengu::Scene::ScriptingScene::onInitialize()
{
	auto& world = getWorld();
	world.RegisterComponent<DrawableComponent>();
	world.RegisterComponent<TransformComponent>();
	world.RegisterComponent<ScriptingComponent>();
	world.RegisterComponent<TagComponent>();

	scriptsys = world.RegisterSystem<Scripting>(world);
	{
		Signature signature;
		signature.set(world.GetComponentType<ScriptingComponent>());
		world.SetSystemSignature<Scripting>(signature);
	}
}
