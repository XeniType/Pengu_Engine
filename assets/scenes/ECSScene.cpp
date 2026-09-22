#include "scenes/ECSScene.hpp"
#include "Pengu_Engine/Managers/Resource/ResourceManager.hpp"
#include "Pengu_Engine/Components/TransformComponent.hpp"

#include <random>

#define QUANTITY 500

namespace Pengu::Scene {
	void ECSScene::update(float dt)
	{
		for (auto& entity : getRenderables()) {
			if (getWorld().GetComponent<TagComponent>(entity).tag == "Player")
			{
				if (m_engine->GetInput().isDown(Action::Arrow_Up)) {
					getWorld().GetComponent<TransformComponent>(entity).position_.z += 0.005f * dt;
				}
				if (m_engine->GetInput().isDown(Action::Arrow_Down)) {
					getWorld().GetComponent<TransformComponent>(entity).position_.z -= 0.005f * dt;
				}
				if (m_engine->GetInput().isDown(Action::Arrow_Right)) {
					getWorld().GetComponent<TransformComponent>(entity).position_.x += 0.005f * dt;
				}
				if (m_engine->GetInput().isDown(Action::Arrow_Left)) {
					getWorld().GetComponent<TransformComponent>(entity).position_.x -= 0.005f * dt;
				}
				if (m_engine->GetInput().isDown(Action::Q)) {
					getWorld().GetComponent<TransformComponent>(entity).position_.y += 0.005f * dt;
				}
				if (m_engine->GetInput().isDown(Action::E)) {
					getWorld().GetComponent<TransformComponent>(entity).position_.y -= 0.005f * dt;
				}
			}
			else {
				getWorld().GetComponent<TransformComponent>(entity).rotation_.z += 0.001f * dt;
			}
		}
	}

	void ECSScene::onLoad(Pengu::Resources::ResourceManager& rm)
	{
		m_resourceManager = &rm;
		onInitialize();
		auto& world = getWorld();
		auto& camera = m_engine->GetCamera();

		auto ezreal = rm.loadObject("../assets/object/frieren/frieren.gltf");

		auto cube = rm.loadObject("../assets/object/redpanda/red-panda.obj");

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distrib(-50, 50);

		std::vector<Entity> entities(QUANTITY);

		for (auto& entity : entities) {
			entity = world.CreateEntity();

			world.AddComponent(entity, DrawableComponent(ezreal));
			world.AddComponent(entity, TransformComponent{ glm::vec3(distrib(gen), distrib(gen), distrib(gen)),glm::vec3(1.0f), glm::vec3(-1.51f,0.0f,0.0f) });
			world.AddComponent(entity, TagComponent("Entities"));
		}

		auto player = world.CreateEntity();

		world.AddComponent(player, DrawableComponent(cube));
		world.AddComponent(player, TransformComponent(glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f)));
		world.AddComponent(player, TagComponent("Player"));

	}

	void ECSScene::onUnload()
	{


	}

	void ECSScene::onInitialize()
	{
		auto& world = getWorld();

		// 1. ECS Setup
		world.RegisterComponent<DrawableComponent>();
		world.RegisterComponent<TransformComponent>();
		world.RegisterComponent<TagComponent>();
	}
}
