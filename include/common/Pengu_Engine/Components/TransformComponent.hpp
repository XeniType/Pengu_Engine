/**
 * @file TransformComponent.hpp
 * @brief Component for entity transformations (position, rotation, scale).
 */

#ifndef TRANSFORMCOMPONENT_HPP
#define TRANSFORMCOMPONENT_HPP 1

#include "glm/glm.hpp"

/**
 * @struct TransformComponent
 * @brief Component that defines the position, rotation, and scale of an entity.
 */
struct TransformComponent {
	/**
	 * @brief Default constructor.
	 */
	TransformComponent() {};
	/**
	 * @brief Constructor with position, scale, and rotation.
	 * @param pos Initial position.
	 * @param scl Initial scale.
	 * @param rot Initial rotation.
	 */
	TransformComponent(glm::vec3 pos, glm::vec3 scl, glm::vec3 rot) : position_{ pos }, scale_{ scl }, rotation_{ rot } {};

	/** @brief Model transformation matrix. */
	glm::mat4 model_ = glm::mat4(1.0f);

	/** @brief Position in world space. */
	glm::vec3 position_ = glm::vec3(0.0f);
	/** @brief Scale of the entity. */
	glm::vec3 scale_ = glm::vec3(1.0f);
	/** @brief Rotation in Euler angles. */
	glm::vec3 rotation_ = glm::vec3(0.0f);
};

#endif // !TRANSFORMCOMPONENT_HPP
