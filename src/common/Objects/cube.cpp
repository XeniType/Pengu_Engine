#include "Pengu_Engine/Objects/cube.hpp"

void Cube::init8v()
{
	float hsize = m_size * 0.5f;

	glm::vec3 position[8] =
	{
			{-hsize, -hsize, hsize}, // Bottom Left near
			{ hsize, -hsize, hsize},  // Bottom right near
			{ hsize, hsize, hsize},   // Top right near
			{-hsize, hsize, hsize},  // Top left near

			{-hsize, -hsize, -hsize},// Bottom Left near
			{hsize, -hsize, -hsize}, // Bottom right near
			{hsize, hsize, -hsize},  // Top right near
			{-hsize, hsize, -hsize}  // Top left near
	};

	glm::vec3 normals[8] =
	{
			{0.0f, 0.0f, 1.0f}, // Bottom Left
			{0.0f, 0.0f, 1.0f}, // Bottom right
			{0.0f, 0.0f, 1.0f}, // Top right
			{0.0f, 0.0f, 1.0f},  // Top left

			{0.0f, 0.0f, 1.0f},
			{0.0f, 0.0f, 1.0f},
			{0.0f, 0.0f, 1.0f},
			{0.0f, 0.0f, 1.0f}
	};

	glm::vec2 uv[8] =
	{

			{0.0f,0.0f}, // Bottom Left
			{1.0f,0.0f}, // Bottom right
			{1.0f,1.0f}, // Top right
			{0.0f,1.0f},  // Top left

			{0.0f,0.0f},
			{1.0f,0.0f},
			{1.0f,1.0f},
			{0.0f,1.0f}

	};

	for (int i = 0; i < 8; i++) {

		Pengu::Graphics::Vertex v;
		v.position = position[i];
		v.normal = normals[i];
		v.uv = uv[i];

		m_data.vertices.push_back(v);
	}

	unsigned int order[36] =
	{
			0,1,3,
			1,2,3,
			1,5,2,
			5,6,2,
			5,4,6,
			4,7,6,
			4,0,7,
			0,3,7,
			3,2,7,
			2,6,7,
			0,4,1,
			1,4,5
	};

	for (int i = 0; i < 36; i++) {
		m_data.indices.push_back(order[i]);
	}
}

void Cube::init24v()
{
	float hsize = m_size * 0.5f;

	glm::vec3 position[24] =
	{
			{hsize, hsize, hsize},       // 0
			{hsize, -hsize, hsize},      // 1
			{-hsize, -hsize, hsize},     // 2
			{-hsize, hsize, hsize},      // 3

			{hsize, hsize, -hsize},     // 4
			{hsize, -hsize, -hsize},    // 5
			{-hsize, -hsize, -hsize},   // 6
			{-hsize, hsize, -hsize},    // 7

			{hsize, hsize, hsize},       // 8  (0)
			{hsize, -hsize, hsize},      // 9  (1)
			{hsize, -hsize, -hsize},     // 10 (2)
			{hsize, hsize, -hsize},      // 11 (3)

			{-hsize, -hsize, hsize},      // 12 (4)
			{-hsize, hsize, hsize},       // 13 (5)
			{-hsize, hsize, -hsize},    // 14 (6)
			{-hsize, -hsize, -hsize},     // 15 (7)

			{hsize, hsize, hsize},      // 16 (0)
			{-hsize, hsize, hsize},     // 17 (1)
			{-hsize, hsize, -hsize},    // 18 (2)
			{hsize, hsize, -hsize},     // 19 (3)

			{hsize, -hsize, hsize},     // 20 (4)
			{-hsize, -hsize, hsize},    // 21 (5)
			{-hsize, -hsize, -hsize},   // 22 (6)
			{hsize, -hsize, -hsize},    // 23 (7)
	};

	glm::vec3 normals[24] =
	{
		 {0.0f, 0.0f, 1.0f},  // 0
		 {0.0f, 0.0f, 1.0f},  // 1
		 {0.0f, 0.0f, 1.0f},  // 2
		 {0.0f, 0.0f, 1.0f},  // 3

		 {0.0f, 0.0f, -1.0f},  // 4
		 {0.0f, 0.0f, -1.0f},  // 5
		 {0.0f, 0.0f, -1.0f},  // 6
		 {0.0f, 0.0f, -1.0f},  // 7

		 {1.0f, 0.0f, 0.0f},  // 8  (0)
		 {1.0f, 0.0f, 0.0f},  // 9  (1)
		 {1.0f, 0.0f, 0.0f},  // 10 (2)
		 {1.0f, 0.0f, 0.0f},  // 11 (3)

		 {-1.0f, 0.0f, 0.0f},  // 12 (4)
		 {-1.0f, 0.0f, 0.0f},  // 13 (5)
		 {-1.0f, 0.0f, 0.0f},  // 14 (6)
		 {-1.0f, 0.0f, 0.0f},  // 15 (7)

		 {0.0f, 1.0f, 0.0f},  // 16 (0)
		 {0.0f, 1.0f, 0.0f},  // 17 (1)
		 {0.0f, 1.0f, 0.0f},  // 18 (2)
		 {0.0f, 1.0f, 0.0f},  // 19 (3)

		 {0.0f, -1.0f, 0.0f},  // 20 (4)
		 {0.0f, -1.0f, 0.0f},  // 21 (5)
		 {0.0f, -1.0f, 0.0f},  // 22 (6)
		 {0.0f, -1.0f, 0.0f}   // 23 (7)
	};

	glm::vec2 uv[24] =
	{

			{0.0f,0.0f}, // 0
			{0.0f,0.0f}, // 1
			{0.0f,0.0f}, // 2
			{1.0f,0.0f}, // 3

			{1.0f,0.0f}, // 4
			{1.0f,0.0f}, // 5
			{1.0f,1.0f}, // 6
			{1.0f,1.0f}, // 7

			{1.0f,1.0f}, // 8  (0)
			{0.0f,1.0f}, // 9  (1)
			{0.0f,1.0f}, // 10 (2)
			{0.0f,1.0f}, // 11 (3)

			{0.0f,0.0f}, // 12 (4)
			{0.0f,0.0f}, // 13 (5)
			{0.0f,0.0f}, // 14 (6)
			{1.0f,0.0f}, // 15 (7)

			{1.0f,0.0f}, // 16 (0)
			{1.0f,0.0f}, // 17 (1)
			{1.0f,1.0f}, // 18 (2)
			{1.0f,1.0f}, // 19 (3)

			{1.0f,1.0f}, // 20 (4)
			{0.0f,1.0f}, // 21 (5)
			{0.0f,1.0f}, // 22 (6)
			{0.0f,1.0f}  // 23 (7)
	};

	for (int i = 0; i < 24; i++) {

		Pengu::Graphics::Vertex v;
		v.position = position[i];
		v.normal = normals[i];
		v.uv = uv[i];

		m_data.vertices.push_back(v);
	}

	unsigned int order[36] =
	{
			 2, 1, 0,
			 0, 3, 2,
			 4, 5, 6,
			 6, 7, 4,
			 10, 11, 9,
			 11, 8, 9,
			 15, 12, 14,
			 12, 13, 14,
			 16, 19, 18,
			 18, 17, 16,
			 23, 20, 21,
			 21, 22, 23,

	};

	for (int i = 0; i < 36; i++) {
		m_data.indices.push_back(order[i]);
	}
}
