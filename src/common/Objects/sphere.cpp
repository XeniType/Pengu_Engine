#include "Pengu_Engine/Objects/sphere.hpp"

Sphere::Sphere(const float size, const int height, const int revs)
{
	float Alpha = (3.1415f / height);
	float Omega = ((2.0f * 3.1415f) / revs);

	for (int i = 0; i < height + 1; i++)
	{
		float angle_alpha = Alpha * i - 1.5708f;
		float radius_xz = cosf(angle_alpha);

		float y = sinf(angle_alpha) * size;

		for (int j = 0; j < revs + 1; j++)
		{
			Pengu::Graphics::Vertex aux;

			float angle_omega = Omega * j;
			float x = cosf(angle_omega) * radius_xz * size;
			float z = sinf(angle_omega) * radius_xz * size;

			aux.position = { x,y,z };
			aux.normal = glm::normalize(glm::vec3{ x,y,z });
			aux.uv = { static_cast<float>(j / (revs)), 1.0f - static_cast<float>(i / (height)) };
			m_data.vertices.push_back(aux);
		}
	}

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < revs + 1; j++) {

			m_data.indices.push_back(i * (revs + 1) + j);
			m_data.indices.push_back((i * (revs + 1) + j) + (revs + 1));
			m_data.indices.push_back(((j + 1) % (revs + 1) + i * (revs + 1)) + (revs + 1));

			m_data.indices.push_back(i * (revs + 1) + j);
			m_data.indices.push_back(((j + 1) % (revs + 1) + i * (revs + 1)) + (revs + 1));
			m_data.indices.push_back((j + 1) % (revs + 1) + i * (revs + 1));
		}
	}

}
