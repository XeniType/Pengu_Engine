#include "Pengu_Engine/Scene/SceneManager.hpp"
#include "Pengu_Engine/Graphics/Rendering/Techniques/Unlit.hpp"
#include "Pengu_Engine/Graphics/Rendering/Techniques/Forward.hpp"
#include "Pengu_Engine/Graphics/Rendering/Techniques/ForwardShad.hpp"
#include "Pengu_Engine/Graphics/Rendering/Techniques/Deferred.hpp"
#include "Pengu_Engine/Graphics/Rendering/Renderer.hpp"
#include "Pengu_Engine/Scene/SceneBase.hpp"
#include "Pengu_Engine/Managers/Resource/ResourceManager.hpp"
#include "Pengu_Engine/Misc/Logmacros.hpp"
#include "Pengu_Engine/Misc/FramRate.hpp"
#include "Pengu_Engine/PenguEngine.hpp"
#include "Pengu_Engine/Systems/TerrainSystem.hpp"
#include "Pengu_Engine/Components/TerrainComponent.hpp"

#include <iostream>

using Pengu::Graphics::Rendering::UnlitPipeline;
using Pengu::Graphics::Rendering::ForwardPipeline;
using Pengu::Graphics::Rendering::ForwardShadPipeline;


namespace Pengu::Core {
	PenguEngine::PenguEngine(PenguEngine&& other) : m_window{ std::move(other.m_window) }, m_input{ std::move(other.m_input) }, m_camera{ std::move(other.m_camera) }, log_{ std::move(other.log_) }, m_resourceManager{ std::move(other.m_resourceManager) }, m_scenemanager{ std::move(other.m_scenemanager) }, m_renderer{ std::move(other.m_renderer) }, m_jobsystem{ std::move(other.m_jobsystem) } {

	}

	PenguEngine& PenguEngine::operator = (PenguEngine&& rvalue) {
		m_window = std::move(rvalue.m_window);
		m_input = std::move(rvalue.m_input);
		m_camera = std::move(rvalue.m_camera);
		log_ = std::move(rvalue.log_);
		m_resourceManager = std::move(rvalue.m_resourceManager);
		m_scenemanager = std::move(rvalue.m_scenemanager);
		m_renderer = std::move(rvalue.m_renderer);
		m_jobsystem = std::move(rvalue.m_jobsystem);

		return *this;
	}

	void PenguEngine::loadScene(std::unique_ptr<Pengu::Scene::SceneBase> scene)
	{
		ENGINE_INFO("Current Shader in use: {}", m_renderer->getPipeline()->getName());
		m_scenemanager->loadScene(std::move(scene), GetResourceManager());
	}

	void PenguEngine::update()
	{
		FrameRate::Get().FPSTick();

		float dt = static_cast<float>(FrameRate::Get().GetDeltaTime());

		int width = GetWindow().GetWidth();
		int height = GetWindow().GetHeight();

		GetResourceManager().UpdateMainThreadTasks();

		if (width > 0 && height > 0)
			GetCamera().CreatePerspective(width, height);

		if (GetInput().isMouseDown(1))
		{
			GetCamera().processInput(GetInput(), dt);
			GetCamera().processMouseMovement(
				static_cast<float>(GetInput().mouseDeltaX()),
				static_cast<float>(GetInput().mouseDeltaY()));

			GetCamera().Set_CamSpeed(static_cast<float>((GetInput().scrollYOffset() * 0.0005f)) + GetCamera().Get_CamSpeed());

			if (GetInput().isDown(Action::Z)) {
				GetCamera().processMouseScroll(-0.05f);
			}
			if (GetInput().isDown(Action::X)) {
				GetCamera().processMouseScroll(0.05f);
			}
		}
		else
		{
			GetCamera().zoom_ = 90.0f;
		}

		m_scenemanager->update(dt);

		if (auto* scene = getActiveScene())
		{
			m_renderer->render(*scene, m_camera);
		}
		GetInput().update();
	}

	PenguEngine::PenguEngine(std::unique_ptr<Window>     window,
		std::unique_ptr<Input>      input,
		Camera                      camera,
		Logger                      log,
		std::unique_ptr<Pengu::Resources::ResourceManager> rm,
		std::unique_ptr < Pengu::Scene::SceneManager > scenemanger,
		std::unique_ptr<Pengu::Graphics::Rendering::Renderer> renderer) : m_window{ std::move(window) }, m_input{ std::move(input) }, m_camera{
		std::move(camera)
		}, log_{ std::move(log) }, m_resourceManager{ std::move(rm) }, m_scenemanager{ std::move(scenemanger) }, m_renderer{ std::move(renderer) }, m_jobsystem(std::make_unique<JobSystem>()) {
		log_.Init();
		ENGINE_INFO("Logger Initialized {} / {}", __DATE__, __TIME__);
		ENGINE_INFO("JobSystem initialized with {} threads.", std::to_string(std::thread::hardware_concurrency()));
	}

	std::optional<PenguEngine> PenguEngine::create(const EngineConfig& config) {

		//Initializing GLFW Lib
		if (!glfwInit()) {
			ENGINE_WARNING("Problem making the program window")
				return std::nullopt;
		}

		auto window = std::make_unique<Window>(config.screen_width, config.screen_height, config.title);

		if (!window->GetWindow()) {
			ENGINE_WARNING("Failed to create GLFWWindow");
			return std::nullopt;
		}

		glewInit();

		auto input = std::make_unique<Input>(window->GetWindow());
		auto rm = std::make_unique<Pengu::Resources::ResourceManager>();
		auto scenemanager = std::make_unique<Pengu::Scene::SceneManager>();
		auto renderer = std::make_unique<Pengu::Graphics::Rendering::Renderer>();

		Camera cam;
		cam.zoom_ = config.fov;
		cam.CreatePerspective(config.screen_width, config.screen_height);
		cam.Set_CamSpeed(config.cam_speed);
		cam.Set_CamSens(config.cam_sens);

		std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
		std::cout << "GLSL Version:   " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

		return PenguEngine(
			std::move(window),
			std::move(input),
			std::move(cam),
			Logger{},
			std::move(rm),
			std::move(scenemanager),
			std::move(renderer)
		);
	}

	PenguEngine PenguEngine::startEngine(const EngineConfig& config)
	{
		std::optional<PenguEngine> mb_engine = PenguEngine::create(config);
		if (!mb_engine)
		{
			ENGINE_ERROR("PenguEngine: Failed to start engine");
			std::terminate();
		}

		PenguEngine engine = std::move(mb_engine.value());

		glfwSetWindowUserPointer(engine.m_window->GetWindow(), &engine);

		switch (config.pipeline)
		{
		case RenderPipeline::Unlit:
			engine.getRenderer().init(std::make_unique<Pengu::Graphics::Rendering::UnlitPipeline>(), engine.GetResourceManager(), { config.screen_height,config.screen_width });
			break;

		case RenderPipeline::Forward:
			engine.getRenderer().init(std::make_unique<Pengu::Graphics::Rendering::ForwardPipeline>(), engine.GetResourceManager(), { config.screen_height,config.screen_width });
			break;

		case RenderPipeline::ForwardShad:
			engine.getRenderer().init(std::make_unique<Pengu::Graphics::Rendering::ForwardShadPipeline>(), engine.GetResourceManager(), { config.screen_height,config.screen_width });
			engine.getRenderer().onResize(config.screen_width, config.screen_height);
			break;

		case RenderPipeline::Deferred:
			engine.getRenderer().init(std::make_unique<Pengu::Graphics::Rendering::DeferredPipeline>(), engine.GetResourceManager(), { config.screen_height,config.screen_width });
			break;

		default:
			engine.getRenderer().init(std::make_unique<Pengu::Graphics::Rendering::UnlitPipeline>(), engine.GetResourceManager(), { config.screen_height,config.screen_width });
			break;
		}

		engine.GetResourceManager().SetJobSystem(&engine.GetJobSystem());

		return engine;
	}

	bool PenguEngine::IsClosing() const
	{
		return glfwWindowShouldClose(m_window->GetWindow());

	}

	void PenguEngine::EndFrame()
	{
		glfwPollEvents();
		glfwSwapBuffers(m_window->GetWindow());
	}

	PenguEngine::~PenguEngine() {
		//Stop GLFW Lib
		if (m_window && m_window->GetWindow()) {
			glfwTerminate();
			ENGINE_INFO("Engine Closed Successfully")
		}
	}

	Pengu::Scene::SceneBase* PenguEngine::getActiveScene() {
		return m_scenemanager->getActiveScene();
	}
	Pengu::Resources::ResourceManager& PenguEngine::GetResourceManager() { return *m_resourceManager; }
	Pengu::Scene::SceneManager& PenguEngine::getSceneManager() { return *m_scenemanager; }
	Pengu::Graphics::Rendering::Renderer& PenguEngine::getRenderer() { return *m_renderer; }
}
