#include "scenes/JobSysScene.hpp"
#include "Pengu_Engine/Managers/Resource/ResourceManager.hpp"
#include "Pengu_Engine/Components/TransformComponent.hpp"

namespace Pengu::Scene {
	void JobSysScene::update(float dt)
	{

		if (m_engine->GetInput().isPressed(Action::Arrow_Right))
		{
			if (current_path < paths.size() - 1)
			{
				current_path += 1;
				m_resourceManager->LoadObjectAsync(paths[current_path], [this](std::shared_ptr<Pengu::Graphics::GameObject> newEntity)
					{
						auto& world = getWorld();
						auto entity = world.GetEntityByTag("Entity");
						world.GetComponent<DrawableComponent>(entity) = newEntity;
					});
			}
		}
		if (m_engine->GetInput().isPressed(Action::Arrow_Left))
		{
			if (current_path > 0)
			{
				current_path -= 1;
				m_resourceManager->LoadObjectAsync(paths[current_path], [this](std::shared_ptr<Pengu::Graphics::GameObject> newEntity)
					{
						auto& world = getWorld();
						auto entity = world.GetEntityByTag("Entity");
						world.GetComponent<DrawableComponent>(entity) = newEntity;
					});
			}
		}
	}

	void JobSysScene::onLoad(Pengu::Resources::ResourceManager& rm)
	{
		m_resourceManager = &rm;
		onInitialize();
		auto& world = getWorld();

		auto entity = world.CreateEntity();

		paths.push_back("../assets/object/frieren/frieren.gltf");
		paths.push_back("../assets/object/redpanda/red-panda.obj");
		paths.push_back("../assets/object/boat/boat.gltf");
		paths.push_back("../assets/object/skeleton/skeleton.gltf");
		paths.push_back("../assets/object/chicken/chicken.gltf");


		rm.LoadObjectAsync(paths[current_path], [this](std::shared_ptr<Pengu::Graphics::GameObject> model)
			{
				if (!model) {
					LOG_ERROR("Failed to load frieren.gltf");
					return; // Prevent crashing if the file is missing
				}
				auto& world = getWorld();

				auto entity = world.CreateEntity();
				world.AddComponent(entity, DrawableComponent(model));
				world.AddComponent(entity, TransformComponent{
						glm::vec3(0.0f),
						glm::vec3(1.0f),
						glm::vec3(0.0f)
					});
				world.AddComponent(entity, TagComponent("Entity"));
			});
	}

	void JobSysScene::onUnload()
	{


	}

	void JobSysScene::onInitialize()
	{
		auto& world = getWorld();

		// 1. ECS Setup
		world.RegisterComponent<DrawableComponent>();
		world.RegisterComponent<TransformComponent>();
		world.RegisterComponent<TagComponent>();
	}
}
