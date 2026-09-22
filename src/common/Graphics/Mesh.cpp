#include "Pengu_Engine/Graphics/Mesh.hpp"
#include "Pengu_Engine/Misc/Logmacros.hpp"
#include "gl/glew.h"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

namespace Pengu::Graphics {
	bool Mesh::threadLoadFromAssimp(aiMesh* mesh)
	{
		processMesh(mesh);
		return false;
	}
	bool Mesh::loadFromAssimp(aiMesh* mesh)
	{
		processMesh(mesh);
		uploadToGPU();
		return true;
	}

	void Mesh::Draw(bool bWireFrame) const
	{

		if (bWireFrame) {
			glDisable(GL_DEPTH_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			glBindVertexArray(m_vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebos[0]);

			glDrawElements(GL_TRIANGLES, m_indexCount[0], GL_UNSIGNED_INT, 0);

			glBindVertexArray(0);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_DEPTH_TEST);
			return;
		}
		else
		{
			glBindVertexArray(m_vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebos[0]);
			glDrawElements(GL_TRIANGLES, m_indexCount[0], GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}

	void Mesh::Draw(bool bWireFrame, int lodLevel) const
	{
		if (lodLevel >= m_ebos.size()) lodLevel = (int)m_ebos.size() - 1;
		if (lodLevel < 0) lodLevel = 0;

		if (bWireFrame) {
			glDisable(GL_DEPTH_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			glBindVertexArray(m_vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebos[lodLevel]);

			glDrawElements(GL_TRIANGLES, m_indexCount[lodLevel], GL_UNSIGNED_INT, 0);

			glBindVertexArray(0);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_DEPTH_TEST);
			return;
		}
		else
		{
			glBindVertexArray(m_vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebos[lodLevel]);

			glDrawElements(GL_TRIANGLES, m_indexCount[lodLevel], GL_UNSIGNED_INT, 0);

			glBindVertexArray(0);

		}
	}

	Mesh::~Mesh()
	{
		if (m_vao) glDeleteVertexArrays(1, &m_vao);
		if (m_vbo) glDeleteBuffers(1, &m_vbo);
		if (!m_ebos.empty()) glDeleteBuffers((GLsizei)m_ebos.size(), m_ebos.data());
	}

	void Mesh::upload(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
	{
		std::vector<std::vector<unsigned int>> singleLOD = { indices };

		upload(vertices, singleLOD);
	}

	void Mesh::upload(const std::vector<Vertex>& vertices, const std::vector<std::vector<unsigned int>>& lodIndices)
	{
		m_vertices = vertices;
		m_lodIndices = lodIndices;
		CalculateBounds();
		uploadToGPU();
	}

	void Mesh::CalculateBounds()
	{
		if (m_vertices.empty()) return;

		// 1. Find the center (average of all vertices, or just the min/max midpoint)
		glm::vec3 minAABB = m_vertices[0].position;
		glm::vec3 maxAABB = m_vertices[0].position;

		for (const auto& v : m_vertices) {
			minAABB = glm::min(minAABB, v.position);
			maxAABB = glm::max(maxAABB, v.position);
		}

		localBounds.center = (minAABB + maxAABB) * 0.5f;

		// 2. Find the maximum distance from the center to any vertex
		float maxRadiusSq = 0.0f;
		for (const auto& v : m_vertices) {
			float distSq = glm::length2(v.position - localBounds.center); // Use length2 to avoid expensive sqrt inside loop
			if (distSq > maxRadiusSq) {
				maxRadiusSq = distSq;
			}
		}

		localBounds.radius = std::sqrt(maxRadiusSq);
	}

	BoundingSphere Mesh::GetWorldBounds(const BoundingSphere& local, const TransformComponent& transform)
	{
		{
			BoundingSphere world;

			// Move the center
			world.center = transform.position_ + (local.center * transform.scale_);

			// Scale the radius by the largest axis of the transform scale
			float maxScale = std::max({ transform.scale_.x, transform.scale_.y, transform.scale_.z });
			world.radius = local.radius * maxScale;

			return world;
		}
	}

	void Mesh::processMesh(aiMesh* mesh)
	{
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex v;

			v.position = {
					mesh->mVertices[i].x,
					mesh->mVertices[i].y,
					mesh->mVertices[i].z
			};

			v.normal = mesh->mNormals ? glm::vec3{
					mesh->mNormals[i].x,
					mesh->mNormals[i].y,
					mesh->mNormals[i].z
			} : glm::vec3{ 0.0f, 1.0f, 0.0f };

			v.uv = mesh->mTextureCoords[0] ? glm::vec2{
					mesh->mTextureCoords[0][i].x,
					mesh->mTextureCoords[0][i].y
			} : glm::vec2{ 0.0f };

			v.tangent = mesh->mTangents ? glm::vec3{
					mesh->mTangents[i].x,
					mesh->mTangents[i].y,
					mesh->mTangents[i].z
			} : glm::vec3{ 0.0f };

			v.bitangent = mesh->mBitangents ? glm::vec3{
					mesh->mBitangents[i].x,
					mesh->mBitangents[i].y,
					mesh->mBitangents[i].z
			} : glm::vec3{ 0.0f };

			m_vertices.push_back(v);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace& face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				m_indices.push_back(face.mIndices[j]);
		}

		m_lodIndices = { m_indices };
	}

	void Mesh::uploadToGPU()
	{
		m_vertexCount = (unsigned int)m_vertices.size();

		if (m_vao) glDeleteVertexArrays(1, &m_vao);
		if (m_vbo) glDeleteBuffers(1, &m_vbo);
		if (!m_ebos.empty()) glDeleteBuffers((GLsizei)m_ebos.size(), m_ebos.data());

		glCreateVertexArrays(1, &m_vao);
		glCreateBuffers(1, &m_vbo);

		m_ebos.resize(m_lodIndices.size());
		m_indexCount.resize(m_lodIndices.size());
		glCreateBuffers((GLsizei)m_ebos.size(), m_ebos.data());

		glNamedBufferData(m_vbo, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

		for (size_t i = 0; i < m_lodIndices.size(); ++i) {
			glNamedBufferData(m_ebos[i], m_lodIndices[i].size() * sizeof(unsigned int), m_lodIndices[i].data(), GL_STATIC_DRAW);
			m_indexCount[i] = static_cast<unsigned int>(m_lodIndices[i].size());
		}

		// Bind VBO to VAO
		glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(Vertex));

		// Position (location = 0)
		glEnableVertexArrayAttrib(m_vao, 0);
		glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
		glVertexArrayAttribBinding(m_vao, 0, 0);

		// Normal (location = 1)
		glEnableVertexArrayAttrib(m_vao, 1);
		glVertexArrayAttribFormat(m_vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
		glVertexArrayAttribBinding(m_vao, 1, 0);

		// UV (location = 2)
		glEnableVertexArrayAttrib(m_vao, 2);
		glVertexArrayAttribFormat(m_vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
		glVertexArrayAttribBinding(m_vao, 2, 0);

		// Tangent (location = 3)
		glEnableVertexArrayAttrib(m_vao, 3);
		glVertexArrayAttribFormat(m_vao, 3, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, tangent));
		glVertexArrayAttribBinding(m_vao, 3, 0);

		// Bitangent (location = 4)
		glEnableVertexArrayAttrib(m_vao, 4);
		glVertexArrayAttribFormat(m_vao, 4, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, bitangent));
		glVertexArrayAttribBinding(m_vao, 4, 0);

		m_vertices.clear();
		m_vertices.shrink_to_fit();
		m_lodIndices.clear();
		m_lodIndices.shrink_to_fit();
	}
}