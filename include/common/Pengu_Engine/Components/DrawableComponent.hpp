/**
 * @file DrawableComponent.hpp
 * @brief Components for rendering game objects.
 */

#ifndef DRAWABLECOMPONENT_HPP
#define DRAWABLECOMPONENT_HPP 1

#include "Pengu_Engine/Graphics/GameObject.hpp"

#include <memory>

 /**
	* @struct DrawableComponent
	* @brief Component that allows a game object to be drawn.
	*/
struct DrawableComponent {
	/**
	 * @brief Default constructor.
	 */
	DrawableComponent() {};
	/**
	 * @brief Constructor with GameObject.
	 * @param gameobj Pointer to the game object to be drawn.
	 */
	DrawableComponent(std::shared_ptr<Pengu::Graphics::GameObject> gameobj) : gameObj{ gameobj } {};

	/** @brief Whether to draw normals for debugging. */
	bool drawNormals = false;
	/** @brief Whether to draw UVs for debugging. */
	bool drawUV = false;

	/** @brief The game object associated with this component. */
	std::shared_ptr<Pengu::Graphics::GameObject> gameObj;
};

/**
 * @struct TerrainDrawableComponent
 * @brief Component that allows terrain to be drawn.
 */
struct TerrainDrawableComponent {
	/**
	 * @brief Default constructor.
	 */
	TerrainDrawableComponent() {};
	/**
	 * @brief Constructor with GameObject.
	 * @param gameobj Pointer to the terrain game object.
	 */
	TerrainDrawableComponent(std::shared_ptr<Pengu::Graphics::GameObject> gameobj) : gameObj{ gameobj } {};

	/** @brief Whether to draw normals for debugging. */
	bool drawNormals = false;
	/** @brief Whether to draw UVs for debugging. */
	bool drawUV = false;

	/** @brief The game object associated with this component. */
	std::shared_ptr<Pengu::Graphics::GameObject> gameObj;
};

#endif // !DRAWABLECOMPONENT_HPP
