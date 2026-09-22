/**
 * @file Material.hpp
 * @brief Material properties and texture mapping.
 */

#ifndef MATERIAL_HPP
#define MATERIAL_HPP 1

#include <memory>
#include "glm/glm.hpp"

namespace Pengu::Graphics { class Texture; }
namespace Pengu::Graphics { class Shader; }


namespace Pengu::Graphics {

	/**
	 * @class Material
	 * @brief Defines the visual properties of a mesh surface.
	 * 
	 * The Material class stores albedo color, PBR parameters (roughness, metallic),
	 * and pointers to textures for various maps.
	 */
	class Material {
	public:
		glm::vec4 albedoColor = { 1.0f,1.0f,1.0f,1.0f }; ///< Base color of the material.

		float roughness = 0.5f; ///< Surface roughness (0 = smooth, 1 = rough).
		float metallic = 0.0f;  ///< Metallic property (0 = dielectric, 1 = metallic).

		std::shared_ptr<Texture> albedoMap = nullptr;    ///< Base color texture map.
		std::shared_ptr<Texture> normalMap = nullptr;    ///< Normal map for surface detail.
		std::shared_ptr<Texture> roughnessMap = nullptr; ///< Texture map for roughness.
		std::shared_ptr<Texture> metallicMap = nullptr;  ///< Texture map for metallic properties.
		std::shared_ptr<Texture> aoMap = nullptr;        ///< Ambient Occlusion map.

		/**
		 * @brief Binds the material's properties and textures to a shader.
		 * @param s The shader to bind to.
		 */
		void Bind(const Shader& s) const;
	};
}//Pengu::Graphics::Material

#endif // !MATERIAL_HPP
