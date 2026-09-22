#include "Pengu_Engine/Editor/EngineUI.hpp"
#include "Pengu_Engine/PenguEngine.hpp"

#include "Pengu_Engine/Graphics/Rendering/Techniques/Unlit.hpp"
#include "Pengu_Engine/Graphics/Rendering/Techniques/Forward.hpp"
#include "Pengu_Engine/Graphics/Rendering/Techniques/ForwardShad.hpp"
#include "Pengu_Engine/Graphics/Rendering/Techniques/Deferred.hpp"

#include "Pengu_Engine/Components/WaterComponent.hpp"

#include "../assets/scenes/DeferredScene.hpp"
#include "../assets/scenes/ECSScene.hpp"
#include "../assets/scenes/JobSysScene.hpp"
#include "../assets/scenes/LightingScene.hpp"
#include "../assets/scenes/ScriptingScene.hpp"

#include "../assets/scenes/Demo_Scene.hpp"

#include "../Misc/imgui/imgui.h"
#include "../Misc/imgui/imgui_impl_glfw.h"
#include "../Misc/imgui/imgui_impl_opengl3.h"

#include "Pengu_Engine/Misc/Logmacros.hpp"

#include "glfw/glfw3.h"

static NoiseSettings Alpine_Peaks{
	.scale = 10.0f,
	.maxHeight = 120.0f,
	.octaves = 5,
	.persistence = 0.6f,
	.lacunarity = 2.0f,
	.exponent = 3.5f,
	.seaLevel = 0.0f,
	.maskScale = 500.0f,
	.worldOffset = { 0,0 },
	.warpIntensity = 25.0f,
	.chunkSize = 64,
	.vertexSize = 64
};

static NoiseSettings Rolling_Highlands{
	.scale = 10.0f,
	.maxHeight = 40.0f,
	.octaves = 3,
	.persistence = 0.4f,
	.lacunarity = 2.0f,
	.exponent = 1.2f,
	.seaLevel = 0.0f,
	.maskScale = 500.0f,
	.worldOffset = { 0,0 },
	.warpIntensity = 5.0f,
	.chunkSize = 64,
	.vertexSize = 64
};

static NoiseSettings Scattered_Archipelago{
	.scale = 50.0f,
	.maxHeight = 30.0f,
	.octaves = 2,
	.persistence = 0.5f,
	.lacunarity = 2.0f,
	.exponent = 2.5f,
	.seaLevel = -15.0f,
	.maskScale = 800.0f,
	.worldOffset = { 0,0 },
	.warpIntensity = 15.0f,
	.chunkSize = 64,
	.vertexSize = 64
};

static NoiseSettings Mesas{
	.scale = 10.0f,
	.maxHeight = 60.0f,
	.octaves = 4,
	.persistence = 0.45f,
	.lacunarity = 2.0f,
	.exponent = 5.0f,
	.seaLevel = 0.0f,
	.maskScale = 500.0f,
	.worldOffset = { 0,0 },
	.warpIntensity = 40.0f,
	.chunkSize = 64,
	.vertexSize = 64
};

void EngineUI::init(GLFWwindow* window, Pengu::Core::PenguEngine* engine)
{
	m_engine = engine;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	// Dark theme
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 4.f;
	style.FrameRounding = 3.f;
	style.FramePadding = { 6.f, 4.f };
	style.ItemSpacing = { 8.f, 6.f };
	style.WindowBorderSize = 1.f;

	// Subtle Pengu colour accent (green tint matching your architecture)
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.30f, 0.20f, 1.f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.31f, 1.00f, 0.69f, 1.f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.31f, 1.00f, 0.69f, 1.f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.28f, 0.18f, 1.f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.15f, 0.45f, 0.28f, 1.f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.28f, 0.18f, 1.f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.15f, 0.45f, 0.28f, 1.f);

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	loadIcons();
}

void EngineUI::render(EditorState& state)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	drawMenuBar(state);
	if (state.showInspector)
		drawInspector(state);
	if (state.showProfiler)
		drawProfiler(state);
	if (state.showHierarchy)
		drawHierarchy(state);
	if (state.showContentBrowser)
		drawContentBrowser(state);
	if (state.showCameraSettings)
		drawCameraSettings(state);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EngineUI::shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	m_engine = nullptr;
}

void EngineUI::drawMenuBar(EditorState& state)
{
	if (!ImGui::BeginMainMenuBar()) return;

	auto& world = m_engine->getActiveScene()->getWorld();

	if (ImGui::BeginMenu("File")) {
		if (ImGui::BeginMenu("Open Scene...")) {
			if (ImGui::MenuItem("ECS Scene"))
			{
				m_engine->loadScene(std::make_unique<Pengu::Scene::ECSScene>(m_engine));
			}
			if (ImGui::MenuItem("Scripting Scene"))
			{
				m_engine->loadScene(std::make_unique<Pengu::Scene::ScriptingScene>(m_engine));
			}
			if (ImGui::MenuItem("JobSystem Scene"))
			{
				m_engine->loadScene(std::make_unique<Pengu::Scene::JobSysScene>(m_engine));
			}
			if (ImGui::MenuItem("Lighting Scene"))
			{
				m_engine->loadScene(std::make_unique<Pengu::Scene::LightingScene>(m_engine));
			}
			if (ImGui::MenuItem("Deferred Scene"))
			{
				m_engine->loadScene(std::make_unique<Pengu::Scene::DeferredScene>(m_engine));
			}
			if (ImGui::MenuItem("Demo Scene"))
			{
				m_engine->loadScene(std::make_unique<Pengu::Scene::DemoScene>(m_engine));
			}
			ImGui::EndMenu();
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Quit")) {
			glfwSetWindowShouldClose(m_engine->GetWindow().GetWindow(), true);
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("View")) {
		ImGui::MenuItem("Profiler", nullptr, &state.showProfiler);
		ImGui::MenuItem("Inspector", nullptr, &state.showInspector);
		ImGui::MenuItem("Hierarchy", nullptr, &state.showHierarchy);
		ImGui::MenuItem("Content Browser", nullptr, &state.showContentBrowser);
		ImGui::MenuItem("Camera Settings", nullptr, &state.showCameraSettings);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Scene")) {
		if (ImGui::BeginMenu("Pipeline")) {
			if (ImGui::MenuItem("Unlit")) {
				m_engine->getRenderer().setPipeline(std::make_unique<Pengu::Graphics::Rendering::UnlitPipeline>());
			}
			if (ImGui::MenuItem("Forward")) {
				m_engine->getRenderer().setPipeline(std::make_unique<Pengu::Graphics::Rendering::ForwardPipeline>());
			}
			if (ImGui::MenuItem("Forward + Shadows")) {
				m_engine->getRenderer().setPipeline(std::make_unique<Pengu::Graphics::Rendering::ForwardShadPipeline>());
			}
			if (ImGui::MenuItem("Deferred")) {
				m_engine->getRenderer().setPipeline(std::make_unique<Pengu::Graphics::Rendering::DeferredPipeline>());
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Draw Mode")) {
			ImGui::Checkbox("Wire Frame", &m_engine->getRenderer().getPipeline()->bWireFrame);
			ImGui::Checkbox("Draw Normals", &m_engine->getRenderer().getPipeline()->bDrawNorms);
			ImGui::Checkbox("Draw UV", &m_engine->getRenderer().getPipeline()->bDrawUV);
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Reload Shaders")) {
			m_engine->getRenderer().reload();
		}
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}

void EngineUI::drawInspector(EditorState& state)
{
	ImGui::SetNextWindowSize({ 300.f, 400.f }, ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Inspector")) { ImGui::End(); return; }

	auto& world = m_engine->getActiveScene()->getWorld();

	// FIX 1: Use isValid() instead of < 0
	if (!state.selectedEntityID.isValid())
	{
		ImGui::TextDisabled("No entity selected.");
		ImGui::End();
		return;
	}

	if (world.IsComponentRegistered<TagComponent>() && world.HasComponent<TagComponent>(state.selectedEntityID))
	{
		auto& n = world.GetComponent<TagComponent>(state.selectedEntityID);
		static int  lastSelectedID = -1;
		static char nameBuffer[128] = {};

		if (state.selectedEntityID.id != lastSelectedID)
		{
			// FIX 2: Use snprintf instead of broken strncpy_s
			snprintf(nameBuffer, sizeof(nameBuffer), "%s", n.tag.c_str());
			lastSelectedID = state.selectedEntityID.id;
		}

		// FIX 3: Properly print the entity ID
		ImGui::Text("Entity #%u", state.selectedEntityID.id);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(-1.f);

		if (ImGui::InputText("##EntityName", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			n.tag = nameBuffer;
		}
		if (ImGui::Button("Delete Entity"))
		{
			m_engine->getActiveScene()->getWorld().DestroyEntity(state.selectedEntityID);
			state.selectedEntityID = { 0xFFFFFFFF, 0 }; // Deselect upon deletion
		}
	}
	else {
		// FIX 4: Properly print the entity ID
		ImGui::Text("Entity #%u", state.selectedEntityID.id);

		if (ImGui::Button("Delete Entity"))
		{
			m_engine->getActiveScene()->getWorld().DestroyEntity(state.selectedEntityID);
			state.selectedEntityID = { 0xFFFFFFFF, 0 }; // Deselect upon deletion
		}
	}
	ImGui::Separator();

	if (world.IsComponentRegistered<TransformComponent>() && world.HasComponent<TransformComponent>(state.selectedEntityID))
	{
		auto& t = world.GetComponent<TransformComponent>(state.selectedEntityID);
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::DragFloat3("Position", &t.position_.x, 0.001f);
			ImGui::DragFloat3("Rotation", &t.rotation_.x, 0.001f);
			ImGui::DragFloat3("Scale", &t.scale_.x, 0.001f);
		}
	}
	ImGui::Separator();

	if (world.IsComponentRegistered<DrawableComponent>() && world.HasComponent<DrawableComponent>(state.selectedEntityID))
	{
		auto& d = world.GetComponent<DrawableComponent>(state.selectedEntityID);

		if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("Mesh: ");
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.f));
			ImGui::Button("Drop Mesh##meshslot", ImVec2(-1.f, 0.f));
			ImGui::PopStyleColor();

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(PAYLOAD_MESH))
				{
					IM_ASSERT(payload->DataSize == sizeof(MeshDragPayload));
					const auto* data = static_cast<const MeshDragPayload*>(payload->Data);

					auto newMesh = m_engine->GetResourceManager().loadObject(data->path);
					if (newMesh)
						d.gameObj = newMesh;
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::Checkbox("Visible", &d.gameObj->subMeshes.at(0).visible);
			for (auto& mesh : d.gameObj->subMeshes)
				mesh.visible = d.gameObj->subMeshes.at(0).visible;
		}
	}
	ImGui::Separator();

	if (world.IsComponentRegistered<DrawableComponent>() && world.HasComponent<DrawableComponent>(state.selectedEntityID))
	{
		auto& m = world.GetComponent<DrawableComponent>(state.selectedEntityID);
		if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::ColorPicker4("Albedo", &m.gameObj->subMeshes.at(0).material_->albedoColor.x);
			ImGui::DragFloat("Metallic", &m.gameObj->subMeshes.at(0).material_->metallic, 0.01f);

			for (auto& met : m.gameObj->subMeshes) {
				met.material_->metallic = m.gameObj->subMeshes.at(0).material_->metallic;
			}

			ImGui::DragFloat("Roughness", &m.gameObj->subMeshes.at(0).material_->roughness, 0.01f);

			for (auto& rough : m.gameObj->subMeshes) {
				rough.material_->roughness = m.gameObj->subMeshes.at(0).material_->roughness;
			}

			ImGui::Separator();
			ImGui::Text("Albedo Texture");

			auto& mat = m.gameObj->subMeshes.at(0).material_;
			ImVec2 slotSize(48, 48);

			if (mat->albedoMap)
			{
				ImGui::Image((ImTextureID)(intptr_t)mat->albedoMap->getID(), slotSize);
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.f));
				ImGui::Button("##notex", slotSize);
				ImGui::PopStyleColor();
			}

			if (mat->albedoMap && ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", mat->albedoMap->path().c_str());

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(PAYLOAD_TEXTURE))
				{
					IM_ASSERT(payload->DataSize == sizeof(TextureDragPayload));
					const auto* data = static_cast<const TextureDragPayload*>(payload->Data);

					auto newTex = m_engine->GetResourceManager().getTexture(data->path);
					for (auto& submesh : m.gameObj->subMeshes)
						submesh.material_->albedoMap = newTex;
				}
				ImGui::EndDragDropTarget();
			}

			if (ImGui::BeginPopupContextItem("##texslot_ctx"))
			{
				if (ImGui::MenuItem("Clear texture"))
					for (auto& submesh : m.gameObj->subMeshes)
						submesh.material_->albedoMap = nullptr;
				ImGui::EndPopup();
			}
		}
	}
	ImGui::Separator();

	if (world.IsComponentRegistered<Lights>() && world.HasComponent<Lights>(state.selectedEntityID))
	{
		auto& l = world.GetComponent<Lights>(state.selectedEntityID);
		if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::ColorPicker3("Light Color", &l.color_.x);
			ImGui::Combo("Light Type", &l.type_, { "Directional\0Point\0Spot\0\0" });
			ImGui::DragFloat3("Direction", &l.direction_.x, 0.01f, -1.0f, 1.0f);
			if (l.type_ != (int)LightType::E_Directional) {
				ImGui::DragFloat3("Light Position", &l.position_.x, 0.01f);
			}
			ImGui::DragFloat("Intensity", &l.intensity_, 0.001f, 0.0f, 5.0f);
			ImGui::DragFloat("Shininess", &l.shin_, 0.1f);
			if (l.type_ == (int)LightType::E_Spot) {
				ImGui::DragFloat("Inner Cone", &l.innerCutoff_, 0.1f);
				ImGui::DragFloat("Outer Cone", &l.outerCutoff_, 0.1f);
			}
			if (l.type_ != (int)LightType::E_Directional) {
				ImGui::DragFloat("Radius", &l.radius_, 0.1f);
			}
		}
	}
	ImGui::Separator();

	if (world.IsComponentRegistered<TerrainSettingsComponent>() && world.HasComponent<TerrainSettingsComponent>(state.selectedEntityID))
	{
		auto& TerrSett = world.GetComponent<TerrainSettingsComponent>(state.selectedEntityID);

		ImGui::InputInt("Terrain Seed", &TerrSett.settings.seed);

		if (ImGui::CollapsingHeader("Terrain Generation Presets", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::Button("Alpine Peaks Preset")) {
				TerrSett.settings = Alpine_Peaks;
				TerrSett.bReloadChunks = true;
			}

			if (ImGui::Button("Rolling Highlands Preset")) {
				TerrSett.settings = Rolling_Highlands;
				TerrSett.bReloadChunks = true;
			}

			if (ImGui::Button("Scattered Archipelagos Preset")) {
				TerrSett.settings = Scattered_Archipelago;
				TerrSett.bReloadChunks = true;
			}

			if (ImGui::Button("Mesas Preset")) {
				TerrSett.settings = Mesas;
				TerrSett.bReloadChunks = true;
			}
		}

		ImGui::Separator();
		if (ImGui::CollapsingHeader("Terrain Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::DragFloat("Scale", &TerrSett.settings.scale, 1.0f, 1.0f, 100.0f);
			ImGui::DragFloat("Max Height", &TerrSett.settings.maxHeight, 1.0f, 1.0f, 100.0f);
			ImGui::DragInt("Octaves", &TerrSett.settings.octaves, 1.0f, 1, 8);
			ImGui::DragFloat("Persistance", &TerrSett.settings.persistence, 0.5f, 0.0f, 10.0f);
			ImGui::DragFloat("Lucunatiry", &TerrSett.settings.lacunarity, 1.0f, 1.0f, 10.0f);

			ImGui::DragFloat("Exponent", &TerrSett.settings.exponent, 1.0f, 1.0f, 50.0f);
			ImGui::DragFloat("Sea Level", &TerrSett.settings.seaLevel, 1.0f, -50.0f, 50.0f);
			ImGui::DragFloat("Mask Scale", &TerrSett.settings.maskScale, 10.0f, 10.0f, 1000.0f);
			ImGui::DragFloat("Warp Inetnsity", &TerrSett.settings.warpIntensity, 1.0f, 1.0f, 50.0f);
			ImGui::DragFloat("Chunk Size", &TerrSett.settings.chunkSize, 1.0f, 1, 526);

			ImGui::InputInt("Load Radius", &TerrSett.loadRadius);
			ImGui::InputInt("View Distance", &TerrSett.viewDistance);

			ImGui::InputInt("Size of Vertexes", &TerrSett.settings.vertexSize);

			if (ImGui::Button("Apply Changes")) {
				TerrSett.bReloadChunks = true;
			}
		}

	}

	if (world.IsComponentRegistered<Pengu::Components::WaterComponent>() && world.HasComponent<Pengu::Components::WaterComponent>(state.selectedEntityID))
	{
		auto& water = world.GetComponent<Pengu::Components::WaterComponent>(state.selectedEntityID);

		if (ImGui::CollapsingHeader("Water Settings", ImGuiTreeNodeFlags_DefaultOpen)) {

			ImGui::SliderFloat("Water Level (Height)", &water.height, 0.0f, 100.0f);
			ImGui::ColorPicker4("Water Color", &water.color.x);
			ImGui::SliderFloat("Wave Speed", &water.waveSpeed, 0.0f, 1.0f);
			ImGui::SliderFloat("Wave Strenght", &water.waveStrength, 0.0f, 1.0f);
			ImGui::SliderFloat("Water Tiling", &water.tiling, 0.0f, 100.0f);
			ImGui::SliderFloat("Water Reflectivity", &water.reflectivity, 0.0f, 1.0f);
		}

		if (ImGui::CollapsingHeader("SSR Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Checkbox("SSR Enable", &water.ssrEnabled);
			ImGui::InputInt("Max Step", &water.maxStep);
			ImGui::InputFloat("Step Size", &water.stepSize);
			ImGui::InputFloat("Thickness", &water.thickness);
		}
	}
	ImGui::End();
}

void EngineUI::drawHierarchy(EditorState& state)
{
	ImGui::SetNextWindowSize({ 300.f, 400.f }, ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Hierarchy")) { ImGui::End(); return; }

	auto& world = m_engine->getSceneManager().getActiveScene()->getWorld();

	for (auto& entity : world.GetAllEntities())
	{
		std::string tag = "";
		if (world.HasComponent<TagComponent>(entity)) {
			tag = world.GetComponent<TagComponent>(entity).tag;
		}
		else {
			tag = std::format("Entity {}", entity.id); // Requires #include <format>
		}

		bool selected = (state.selectedEntityID == entity);

		if (ImGui::Selectable(tag.c_str(), selected))
			state.selectedEntityID = entity;

		if (selected)
			ImGui::SetItemDefaultFocus();
	}

	ImGui::End();
}

void EngineUI::drawContentBrowser(EditorState& state)
{
	ImGui::SetNextWindowSize({ 600.f, 200.f }, ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Content Browser")) { ImGui::End(); return; }

	if (m_currentPath != m_rootPath) {
		if (ImGui::Button("<-"))
			m_currentPath = m_currentPath.parent_path();
		ImGui::SameLine();
	}
	ImGui::Text("%s", m_currentPath.string().c_str());
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 160.f);
	ImGui::SetNextItemWidth(150.f);
	ImGui::SliderFloat("##thumbsize", &m_thumbnailSize, 32.f, 128.f);
	ImGui::Separator();

	float cellSize = m_thumbnailSize + 16.f;
	float panelWidth = ImGui::GetContentRegionAvail().x;
	int   columns = std::max(1, (int)(panelWidth / cellSize));

	ImGui::Columns(columns, nullptr, false);

	ImGui::Columns(columns, nullptr, false);

	for (const auto& entry : fs::directory_iterator(m_currentPath))
	{
		const auto& path = entry.path();
		std::string filename = path.filename().string();
		bool        isDir = entry.is_directory();
		std::string ext = path.extension().string();
		bool        isTexture = (ext == ".png" || ext == ".jpg" || ext == ".tga");
		bool isMesh = (ext == ".obj" || ext == ".fbx" || ext == ".gltf");

		ImGui::PushID(filename.c_str());

		unsigned int icon = getIconForEntry(entry);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::ImageButton("##icon", (ImTextureID)(intptr_t)icon,
			ImVec2(m_thumbnailSize, m_thumbnailSize));
		ImGui::PopStyleColor();

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			if (isTexture)
			{
				TextureDragPayload payload{};
				std::string normalizedPath = path.lexically_normal().string();
				std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');
				strncpy_s(payload.path, path.string().c_str(), sizeof(payload.path) - 1);
				ImGui::SetDragDropPayload(PAYLOAD_TEXTURE, &payload, sizeof(payload));
			}
			else if (isMesh)
			{
				MeshDragPayload payload{};
				ImGui::SetTooltip("Path: %s", path.string().c_str());
				std::string normalizedPath = path.lexically_normal().string();
				std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');
				strncpy_s(payload.path, path.string().c_str(), sizeof(payload.path) - 1);
				ImGui::SetDragDropPayload(PAYLOAD_MESH, &payload, sizeof(payload));
			}

			ImGui::Image((ImTextureID)(intptr_t)icon, ImVec2(32, 32));
			ImGui::SameLine();
			ImGui::TextUnformatted(filename.c_str());

			ImGui::EndDragDropSource();
		}

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			if (isDir)
				m_currentPath /= path.filename();
		}

		std::string label = filename.size() > 12
			? filename.substr(0, 10) + ".."
			: filename;
		ImGui::TextWrapped("%s", label.c_str());

		ImGui::NextColumn();
		ImGui::PopID();
	}
	ImGui::Columns(1);
	ImGui::End();
}



unsigned int EngineUI::getIconForEntry(const fs::directory_entry& entry)
{
	if (entry.is_directory()) return m_icons["folder"]->getID();

	std::string ext = entry.path().extension().string();

	if (ext == ".png" || ext == ".jpg" || ext == ".tga") return m_icons["image"]->getID();
	if (ext == ".obj" || ext == ".fbx" || ext == ".gltf") return m_icons["mesh"]->getID();

	return m_icons["file"]->getID();
}

void EngineUI::loadIcons()
{
	auto& rm = m_engine->GetResourceManager();

	auto loadIcon = [&](const std::string& path) -> std::shared_ptr<Pengu::Graphics::Texture> {
		auto tex = rm.getTexture(path);
		return tex;

		unsigned int fallback;
		glGenTextures(1, &fallback);
		glBindTexture(GL_TEXTURE_2D, fallback);
		unsigned char magenta[] = { 255, 0, 255, 255 };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, magenta);
		return nullptr;
		};

	m_icons["folder"] = loadIcon("../assets/editor/icons/folder.png");
	m_icons["image"] = loadIcon("../assets/editor/icons/image.png");
	m_icons["mesh"] = loadIcon("../assets/editor/icons/mesh.png");
	m_icons["file"] = loadIcon("../assets/editor/icons/file.png");
}

void EngineUI::drawProfiler(EditorState& state)
{
	// --- Accumulate history ---
	static float fpsHistory[90] = {};
	static float msHistory[90] = {};
	static int   historyOffset = 0;
	static float accumulator = 0.f;

	accumulator += state.lastFrameMs;
	if (accumulator >= 16.f) {
		fpsHistory[historyOffset] = state.fps;
		msHistory[historyOffset] = state.lastFrameMs;
		historyOffset = (historyOffset + 1) % 90;
		accumulator = 0.f;
	}

	// --- Always pin to top-right of the actual OS window ---
	ImGuiIO& io = ImGui::GetIO();
	float windowWidth = io.DisplaySize.x;

	constexpr float PAD = 10.f;
	constexpr float panelWidth = 280.f;
	constexpr float panelHeight = 180.f;

	ImGui::SetNextWindowPos(
		{ windowWidth - panelWidth - PAD, 30.f },
		ImGuiCond_Always
	);
	ImGui::SetNextWindowSize({ panelWidth, panelHeight }, ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.85f);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus;

	if (!ImGui::Begin("##profiler", nullptr, flags)) { ImGui::End(); return; }

	// --- Text stats ---
	ImGui::TextColored({ 0.31f, 1.f, 0.69f, 1.f }, "FPS     %d", (int)state.fps);
	if (state.lastFrameMs > 16.6f)  // below 60 FPS
		ImGui::TextColored({ 1.f, 0.3f, 0.3f, 1.f }, "Frame   %.2f ms  !", state.lastFrameMs);
	else
		ImGui::Text("Frame   %.2f ms", state.lastFrameMs);
	ImGui::Separator();

	// --- FPS graph ---
	char fpsOverlay[32];
	snprintf(fpsOverlay, sizeof(fpsOverlay), "FPS: %d", (int)state.fps);

	ImGui::PlotLines("##fps",
		fpsHistory, 90,
		historyOffset,
		fpsOverlay,
		0.f, 200.f,
		ImVec2(ImGui::GetContentRegionAvail().x, 70.f)
	);

	// --- Frame time graph ---
	char msOverlay[32];
	snprintf(msOverlay, sizeof(msOverlay), "%.2f ms", state.lastFrameMs);

	ImGui::PlotLines("##ms",
		msHistory, 90,
		historyOffset,
		msOverlay,
		0.f, 33.f,
		ImVec2(ImGui::GetContentRegionAvail().x, 40.f)
	);

	ImGui::End();
}

void EngineUI::drawCameraSettings(EditorState& state)
{

	auto& camera = m_engine->GetCamera();

	if (!ImGui::Begin("##Camera Settings", nullptr)) { ImGui::End(); return; }

	ImGui::Text("Camera Sensetivity:");
	ImGui::SameLine();
	ImGui::SliderFloat("##Sensistivity", &camera.mouseSens_, 0.1f, 5.0f, "%.1f");

	ImGui::Text("Camera Speed:");
	ImGui::SameLine();
	ImGui::SliderFloat("##Speed", &camera.movSpeed_, 0.01f, 0.5f, "%.2f");

	ImGui::Text("Camera Zoom:");
	ImGui::SameLine();
	ImGui::SliderFloat("##Zoom", &camera.zoom_, 1.0f, 180.0f, "%.1f");

	ImGui::End();
}
