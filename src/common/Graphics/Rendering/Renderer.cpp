#include "Pengu_Engine/Graphics/Rendering/Renderer.hpp"
#include "Pengu_Engine/Scene/SceneBase.hpp"
#include "Pengu_Engine/Managers/Resource/ResourceManager.hpp"
#include "Pengu_Engine/Graphics/Rendering/RenderPipeline.hpp"
#include "Pengu_Engine/Misc/Logmacros.hpp"

namespace Pengu::Graphics::Rendering {
	void Renderer::init(std::unique_ptr<RenderPipeline> pipeline, Pengu::Resources::ResourceManager& rm, glm::vec2 screenSize)
	{

		if (!pipeline)
		{
			ENGINE_WARNING("Renderer: initial pipeline cannot be null");
			return;
		}

		m_rm = &rm;
		m_height = static_cast<int>(screenSize.x);
		m_width = static_cast<int>(screenSize.y);
		setPipeline(std::move(pipeline));
	}

	void Renderer::setPipeline(std::unique_ptr<RenderPipeline> newPipeline)
	{

		if (!newPipeline)
		{
			ENGINE_WARNING("Renderer: cannot set a null pipeline");
			return;
		}

		if (!m_rm)
		{
			ENGINE_WARNING("Renderer: init() must be called before setPipeline()");
			return;
		}

		if (m_pipeline)
		{
			m_pipeline->cleanup();
			m_pipeline.reset();
		}

		m_pipeline = std::move(newPipeline);
		m_pipeline->init(*m_rm);

		if (m_width > 0 && m_height > 0) m_pipeline->onResize(m_width, m_height);

		m_pipeline->reload(*m_rm);
	}

	void Renderer::render(Pengu::Scene::SceneBase& scene, Camera& camera)
	{
		if (!m_pipeline || !m_pipeline->isInitialized()) {
			ENGINE_WARNING("Renderer: no initialized pipeline set before render()");
			return;
		}

		m_pipeline->onPreRender(scene, camera);
		m_pipeline->render(scene, camera);
	}

	void Renderer::onResize(int width, int height)
	{
		m_width = width;
		m_height = height;

		if (m_pipeline)
		{
			m_pipeline->onResize(width, height);
		}
	}

	void Renderer::reload()
	{
		m_pipeline->reload(*m_rm);
	}

	void Renderer::shutdown()
	{
		if (m_pipeline) {
			m_pipeline->cleanup();
			m_pipeline.reset();
		}
	}
}