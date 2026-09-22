/**
 * @file quad.hpp
 * @brief Quad primitive generation.
 */

#ifndef QUAD_HPP
#define QUAD_HPP

#include "Pengu_Engine/Graphics/Mesh.hpp"

/**
 * @struct QuadData
 * @brief Container for generated quad vertex and index data.
 */
struct QuadData {
	std::vector<Pengu::Graphics::Vertex> vertices; ///< List of vertices.
	std::vector<unsigned int> indices;            ///< List of indices.
};

/**
 * @class Quad
 * @brief Helper class for generating a simple quad (2 triangles).
 */
class Quad {
public:
	/**
	 * @brief Constructs and generates a quad.
	 * @param size Side length of the quad.
	 */
	Quad(const float size);
	
	QuadData m_data; ///< The generated data.
};

#endif // !QUAD_HPP
