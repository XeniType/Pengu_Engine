/**
 * @file LightComponent.hpp
 * @brief Components for light sources in the scene.
 */

#ifndef LIGHTCOMPONENT_HPP
#define LIGHTCOMPONENT_HPP 1

#include "gl/glew.h"
#include "glm/glm.hpp"
#include <vector>

/**
 * @enum LightType
 * @brief Types of light sources supported by the engine.
 */
enum class LightType {
	E_Directional,
	E_Point,
	E_Spot
};

/**
 * @struct Lights
 * @brief Component representing a light source.
 */
struct Lights {

	/**
	 * @brief Default constructor.
	 */
	Lights() {};

	/**
	 * @brief Constructor with LightType.
	 * @param lightType_ The type of light to create.
	 */
	Lights(LightType lightType_)
	{
		switch (lightType_)
		{
		case LightType::E_Directional:
			type_ = 0;
			break;
		case LightType::E_Point:
			type_ = 1;
			break;
		case LightType::E_Spot:
			type_ = 2;
			break;
		default:
			type_ = 1;
			break;
		}
	};

	/** @brief Numeric type identifier (0: Dir, 1: Point, 2: Spot). */
	int type_ = 0;

	/** @brief Position of the light source. */
	glm::vec3 position_ = { 0.0f,0.0f,0.0f };
	/** @brief Direction of the light source. */
	glm::vec3 direction_ = glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f));
	/** @brief Color of the light. */
	glm::vec3 color_ = { 1.0f,1.0f,1.0f };
	/** @brief Intensity of the light. */
	float intensity_ = 1.0f;
	/** @brief Shininess factor for specular highlights. */
	float shin_ = 32.0f;

	/** @brief Ambient strength. */
	float ambiStr_ = 0.1f;
	/** @brief Specular strength. */
	float specStr_ = 0.5f;

	// Point
	/** @brief Radius of the point light. */
	float radius_ = 10.0f;

	// Spot
	/** @brief Inner cutoff angle for spot lights. */
	float innerCutoff_ = glm::cos(glm::radians(12.5f));
	/** @brief Outer cutoff angle for spot lights. */
	float outerCutoff_ = glm::cos(glm::radians(17.5f));


	// Attenuation
	/** @brief Constant attenuation factor. */
	float constant_ = 1.0f;
	/** @brief Linear attenuation factor. */
	float linear_ = 0.022f;
	/** @brief Quadratic attenuation factor. */
	float quadratic_ = 0.0019f;

	/** @brief Shadow softness factor. */
	float shadowSoftness_ = 1.0f;
	/** @brief Whether the light casts shadows. */
	bool castsShadows = true;
	/** @brief Matrix for light-space transformations. */
	glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);

	/** @brief Cascade matrices for shadow mapping. */
	std::vector<glm::mat4> cascadeSpaceMatrices;
	/** @brief Splitting planes for cascaded shadows. */
	std::vector<float> cascadeSplits;
};

#endif // !LIGHTCOMPONENT_HPP
