/**
 * @file WaterComponent.hpp
 * @brief Component for rendering water surfaces.
 */

#ifndef WATERCOMPONENT_HPP
#define WATERCOMPONENT_HPP 1

#include <glm/glm.hpp>
#include <memory>
#include "Pengu_Engine/Graphics/Texture.hpp"

namespace Pengu::Components {

	/**
	 * @struct WaterComponent
	 * @brief Component that defines properties of a water surface.
	 */
	struct WaterComponent {
		/** @brief Height level of the water surface. */
		float height = 0.0f;
		/** @brief Color of the water. */
		glm::vec4 color = { 0.0f, 0.3f, 0.5f, 0.6f };

		/** @brief Speed of the waves. */
		float waveSpeed = 0.05f;
		/** @brief Strength of the waves. */
		float waveStrength = 0.02f;
		/** @brief Tiling factor for water textures. */
		float tiling = 4.0f;

		/** @brief Reflectivity of the water surface. */
		float reflectivity = 0.5f;
		/** @brief Shininess for specular highlights. */
		float shininess = 128.0f;

		/** @brief Noise texture used for wave generation. */
		std::shared_ptr<Pengu::Graphics::Texture> noiseTexture;

		/** @brief Whether Screen Space Reflections (SSR) are enabled. */
		bool ssrEnabled = true;

		/** @brief Maximum number of steps for SSR. */
		int maxStep = 40;
		/** @brief Step size for SSR. */
		float stepSize = 0.5;
		/** @brief Thickness for SSR intersection. */
		float thickness = 0.5;
	};

}

#endif // !WATERCOMPONENT_HPP
