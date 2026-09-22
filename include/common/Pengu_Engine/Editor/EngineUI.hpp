/**
 * @file EngineUI.hpp
 * @brief Handles the engine's graphical user interface using ImGui.
 */

#ifndef ENGINEUI_HPP
#define ENGINEUI_HPP

#include "EditorState.hpp"
#include <filesystem>
#include <unordered_map>
#include "Pengu_Engine/Graphics/Texture.hpp"

namespace fs = std::filesystem;

struct GLFWwindow;
namespace Pengu::Core { class PenguEngine; }

/** @brief Payload identifier for texture drag-and-drop. */
inline constexpr const char* PAYLOAD_TEXTURE = "TEXTURE_ASSET";

/**
 * @struct TextureDragPayload
 * @brief Data structure for texture drag-and-drop operations.
 */
struct TextureDragPayload
{
	/** @brief Path to the texture asset. */
	char path[256];
};

/** @brief Payload identifier for mesh drag-and-drop. */
inline constexpr const char* PAYLOAD_MESH = "MESH_ASSET";

/**
 * @struct MeshDragPayload
 * @brief Data structure for mesh drag-and-drop operations.
 */
struct MeshDragPayload
{
	/** @brief Path to the mesh asset. */
	char path[256];
};

/**
 * @class EngineUI
 * @brief Manages the editor's user interface, including menus, hierarchy, and inspector.
 */
class EngineUI {
public:

	/**
	 * @brief Initializes the UI system.
	 * @param window Pointer to the GLFW window.
	 * @param engine Pointer to the engine instance.
	 */
	void init(GLFWwindow* window, Pengu::Core::PenguEngine* engine);

	/**
	 * @brief Renders the UI for the current frame.
	 * @param state Current state of the editor.
	 */
	void render(EditorState& state);

	/**
	 * @brief Shuts down the UI system and cleans up resources.
	 */
	void shutdown();

private:
	/** @brief Pointer to the engine instance. */
	Pengu::Core::PenguEngine* m_engine = nullptr;

	/**
	 * @brief Draws the main menu bar.
	 * @param state Current state of the editor.
	 */
	void drawMenuBar(EditorState& state);

	/**
	 * @brief Draws the entity hierarchy window.
	 * @param state Current state of the editor.
	 */
	void drawHierarchy(EditorState& state);
	/**
	 * @brief Draws the entity inspector window.
	 * @param state Current state of the editor.
	 */
	void drawInspector(EditorState& state);

	/**
	 * @brief Draws the performance profiler window.
	 * @param state Current state of the editor.
	 */
	void drawProfiler(EditorState& state);

	/**
	 * @brief Draws the camera settings window.
	 * @param state Current state of the editor.
	 */
	void drawCameraSettings(EditorState& state);

	/**
	 * @brief Draws the content browser window.
	 * @param state Current state of the editor.
	 */
	void drawContentBrowser(EditorState& state);
	
	/**
	 * @brief Gets the icon associated with a file system entry.
	 * @param entry The directory entry.
	 * @return unsigned int The texture ID of the icon.
	 */
	unsigned int getIconForEntry(const fs::directory_entry& entry);
	
	/**
	 * @brief Loads UI icons from disk.
	 */
	void loadIcons();

	/** @brief Root path for the content browser. */
	fs::path m_rootPath = "../assets";
	/** @brief Current directory in the content browser. */
	fs::path m_currentPath = "../assets";
	/** @brief Size of thumbnails in the content browser. */
	float m_thumbnailSize = 80.0f;

	/** @brief Map of icon names to their texture assets. */
	std::unordered_map<std::string, std::shared_ptr<Pengu::Graphics::Texture>> m_icons;
};

#endif // !ENGINEUI_HPP
