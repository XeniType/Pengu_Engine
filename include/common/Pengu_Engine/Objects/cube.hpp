/**
 * @file cube.hpp
 * @brief Cube primitive generation.
 */

#ifndef CUBE_HPP
#define CUBE_HPP 1

#include "Pengu_Engine/Graphics/Mesh.hpp"

/**
 * @struct CubeData
 * @brief Container for generated cube vertex and index data.
 */
struct CubeData {
	std::vector<Pengu::Graphics::Vertex> vertices; ///< List of vertices.
	std::vector<unsigned int> indices;            ///< List of indices.
};

/**
 * @class Cube
 * @brief Helper class for generating cube mesh data.
 */
class Cube
{

public:
	/**
	 * @brief Constructs a cube generator with a specific size.
	 * @param size Side length of the cube.
	 */
	Cube(const float size) : m_size(size) {};

	/**
	 * @brief Initializes a cube with 8 vertices (shared corners).
	 */
	void init8v();

	/**
	 * @brief Initializes a cube with 24 vertices (distinct faces for correct normals/UVs).
	 */
	void init24v();

	float m_size;    ///< Side length of the cube.
	CubeData m_data; ///< The generated data.
};

#endif // !CUBE_HPP
