/**
 * @file sphere.hpp
 * @brief Sphere primitive generation.
 */

#ifndef SPHERE_HPP
#define SPHERE_HPP 1

#include "Pengu_Engine/Graphics/Mesh.hpp"

/**
 * @struct SphereData
 * @brief Container for generated sphere vertex and index data.
 */
struct SphereData {
	std::vector<Pengu::Graphics::Vertex> vertices; ///< List of vertices.
	std::vector<unsigned int> indices;            ///< List of indices.
};

/**
 * @class Sphere
 * @brief Helper class for generating UV-sphere mesh data.
 */
class Sphere {
public:
	/**
	 * @brief Constructs and generates a sphere.
	 * @param size Radius of the sphere.
	 * @param height Vertical segments.
	 * @param revs Horizontal segments (revolutions).
	 */
	Sphere(const float size, const int height, const int revs);
	
	SphereData m_data; ///< The generated data.
};

#endif // !SPHERE_HPP
