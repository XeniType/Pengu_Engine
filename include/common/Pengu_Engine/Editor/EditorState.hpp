/**
 * @file EditorState.hpp
 * @brief Structure for maintaining the state of the engine editor.
 */

#ifndef EDITORSTATE_HPP
#define EDITORSTATE_HPP 1

#include "Pengu_Engine/Managers/ECS/Entity.hpp"

/**
 * @struct EditorState
 * @brief Maintains the current state and settings of the engine editor.
 */
struct EditorState
{
	/** @brief Current width of the viewport. */
	float viewportWidth = 1280.0f;
	/** @brief Current height of the viewport. */
	float viewportHeight = 720.0f;

	/** @brief ID of the currently selected entity. */
	Entity selectedEntityID = { 0xFFFFFFFF, 0 };

	/** @brief Whether to show the profiler window. */
	bool  showProfiler = false;
	/** @brief Whether to show the inspector window. */
	bool  showInspector = false;
	/** @brief Whether to show the hierarchy window. */
	bool  showHierarchy = false;
	/** @brief Whether to show the content browser window. */
	bool  showContentBrowser = false;
	/** @brief Whether to show the camera settings window. */
	bool  showCameraSettings = false;

	/** @brief Duration of the last frame in milliseconds. */
	float lastFrameMs = 0.0f;
	/** @brief Current frames per second. */
	float fps = 0.0f;
};

#endif // !EDITORSTATE_HPP
