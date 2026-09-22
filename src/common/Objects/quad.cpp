#include "Pengu_Engine/Objects/quad.hpp"

Quad::Quad(const float size)
{

	float hsize = size * 0.5f;

	glm::vec3 position[4] =
	{
			{-hsize, -hsize, 0.0f}, // Bottom Left
			{hsize, -hsize, 0.0f},  // Bottom right
			{hsize, hsize, 0.0f},   // Top right
			{-hsize, hsize, 0.0f}   // Top left
	};

	glm::vec3 normals[4] =
	{
			{0.0f, 0.0f, 1.0f}, // Bottom Left
			{0.0f, 0.0f, 1.0f}, // Bottom right
			{0.0f, 0.0f, 1.0f}, // Top right
			{0.0f, 0.0f, 1.0f}  // Top left
	};

	glm::vec2 uv[4] =
	{

			{0.0f,0.0f}, // Bottom Left
			{1.0f,0.0f}, // Bottom right
			{1.0f,1.0f}, // Top right
			{0.0f,1.0f}  // Top left

	};

	for (int i = 0; i < 4; i++) {
		Pengu::Graphics::Vertex v{
			position[i],
			normals[i],
			uv[i]
		};
		m_data.vertices.push_back(v);
	}

	unsigned int order[6] =
	{
			0,1,3,
			1,2,3
	};

	for (int i = 0; i < 6; i++) {
		m_data.indices.push_back(order[i]);
	}
}
