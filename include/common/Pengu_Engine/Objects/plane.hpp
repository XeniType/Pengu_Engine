/**
 * @file plane.hpp
 * @brief Plane primitive generation.
 */

#ifndef PLANE_HPP
#define PLANE_HPP 1

#include "Pengu_Engine/Graphics/Mesh.hpp"

/**
 * @struct PlaneData
 * @brief Container for generated plane vertex and index data.
 */
struct PlaneData {
	std::vector<Pengu::Graphics::Vertex> vertices; ///< List of vertices.
	std::vector<unsigned int> indices;            ///< List of indices.
};

/**
 * @class Plane
 * @brief Helper class for generating a grid-based plane mesh.
 */
class Plane {

public:
	/**
	 * @brief Constructs and generates a plane.
	 * @param rows Number of rows in the grid.
	 * @param cols Number of columns in the grid.
	 * @param size Side length of each grid cell.
	 */
	Plane(int rows, int cols, int size);
	
	PlaneData data_; ///< The generated data.
};

#endif // !PLANE_HPP
