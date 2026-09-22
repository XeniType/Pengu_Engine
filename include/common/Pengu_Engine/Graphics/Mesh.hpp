/**
 * @file Mesh.hpp
 * @brief 3D mesh representation and OpenGL buffer management.
 */

#ifndef MESH_HPP
#define MESH_HPP 1

#include "glm/glm.hpp"
#include "GL/glew.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <vector>
#include <string>
#include "Pengu_Engine/Components/TransformComponent.hpp"

namespace Pengu::Graphics {

	/**
	 * @struct BoundingSphere
	 * @brief Represents a spherical bounding volume for culling and collision.
	 */
	struct BoundingSphere {
		glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f); ///< Center point of the sphere.
		float radius = 0.0f;                             ///< Radius of the sphere.
	};

	/**
	* @struct Vertex
	* @brief Representation of a single vertex point in 3D space.
	*/
	struct Vertex {
		glm::vec3 position;  ///< 3D coordinates of the vertex.
		glm::vec3 normal;    ///< Normal vector for lighting calculations.
		glm::vec2 uv;        ///< UV coordinates for texture mapping.
		glm::vec3 tangent;   ///< Tangent vector for normal mapping.
		glm::vec3 bitangent; ///< Bitangent vector for normal mapping.
	};

	/**
	* @class Mesh
	 * @brief Manages vertex data buffers and executes draw calls.
	 * 
	 * This class encapsulates the OpenGL VAO, VBO, and EBO/IBO to simplify
	 * the process of sending geometry to the GPU. It supports multiple levels 
	 * of detail (LOD) via multiple index buffers.
	*/
	class Mesh {

	public:
		/**
		 * @brief Default constructor.
		 */
		Mesh() = default;

		/**
		 * @brief Asynchronously loads mesh data from an Assimp aiMesh structure.
		 * @param mesh Pointer to the Assimp mesh.
		 * @return True if data was successfully processed.
		 */
		bool threadLoadFromAssimp(aiMesh* mesh);

		/**
		 * @brief Loads mesh data from an Assimp aiMesh structure and uploads to GPU.
		 * @param mesh Pointer to the Assimp mesh.
		 * @return True if successful.
		 */
		bool loadFromAssimp(aiMesh* mesh);

		/**
		 * @brief Renders the mesh.
		 * @param bWireFrame If true, renders in wireframe mode.
		 */
		void Draw(bool bWireFrame) const;

		/**
		 * @brief Renders the mesh at a specific LOD level.
		 * @param bWireFrame If true, renders in wireframe mode.
		 * @param lodLevel The LOD index to use.
		 */
		void Draw(bool bWireFrame, int lodLevel) const;

		/**
		 * @brief Checks if the mesh has valid data.
		 */
		bool isValid() const {
			return !m_indexCount.empty() && m_vertexCount > 0;
		}

		/** @brief Gets the number of indices for each LOD level. */
		std::vector<unsigned int> indexCount() const { return m_indexCount; }
		/** @brief Gets the total number of vertices in the mesh. */
		unsigned int vertexCount() const { return m_vertexCount; }
		/** @brief Checks if the mesh buffers have been uploaded to the GPU. */
		bool isUploaded() const { return m_vao != 0; }

		/**
		 * @brief Destructor that deletes OpenGL buffers.
		 */
		~Mesh();

		/// @name Deleted Operations
		/// @{
		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;
		/// @}

		/**
		 * @brief Move constructor.
		 */
		Mesh(Mesh&& other) noexcept {
			m_vao = other.m_vao;
			m_vbo = other.m_vbo;
			m_ebos = std::move(other.m_ebos);
			m_indexCount = std::move(other.m_indexCount);
			m_vertexCount = other.m_vertexCount;
			other.m_vao = 0;
			other.m_vbo = 0;
		}

		/**
		 * @brief Move assignment operator.
		 */
		Mesh& operator=(Mesh&& other) noexcept {
			if (this != &other) {
				if (m_vao) glDeleteVertexArrays(1, &m_vao);
				if (m_vbo) glDeleteBuffers(1, &m_vbo);
				if (!m_ebos.empty()) glDeleteBuffers((GLsizei)m_ebos.size(), m_ebos.data());

				m_vao = other.m_vao;
				m_vbo = other.m_vbo;
				m_ebos = std::move(other.m_ebos);
				m_indexCount = std::move(other.m_indexCount);
				m_vertexCount = other.m_vertexCount;

				other.m_vao = 0;
				other.m_vbo = 0;
				other.m_ebos.clear();
			}
			return *this;
		}

		/**
		 * @brief Uploads vertex and index data to the GPU (single LOD).
		 */
		void upload(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

		/**
		 * @brief Uploads vertex and index data to the GPU (multiple LODs).
		 */
		void upload(const std::vector<Vertex>& vertices,
			const std::vector<std::vector<unsigned int>>& lodIndices);

		/**
		 * @brief Calculates the local bounding sphere for the mesh.
		 */
		void CalculateBounds();

		/**
		 * @brief Transforms the local bounding sphere to world space.
		 * @param local The local bounding sphere.
		 * @param transform The transform component to apply.
		 * @return The world-space bounding sphere.
		 */
		BoundingSphere GetWorldBounds(const BoundingSphere& local, const TransformComponent& transform);
		
		BoundingSphere localBounds; ///< The local bounding sphere of the mesh.

		/**
		 * @brief Finalizes the upload of processed data to the GPU.
		 */
		void uploadToGPU();

	private:
		/**
		 * @brief Internal helper to process Assimp mesh data.
		 */
		void processMesh(aiMesh* mesh);

		std::vector<Vertex>   m_vertices;  ///< CPU-side vertex data.
		std::vector<unsigned int> m_indices; ///< CPU-side index data (for single LOD).

		unsigned int m_vao = 0; ///< OpenGL Vertex Array Object ID.
		unsigned int m_vbo = 0; ///< OpenGL Vertex Buffer Object ID.
		std::vector<unsigned int> m_ebos = {}; ///< OpenGL Element Buffer Object IDs (one per LOD).
		std::vector<unsigned int> m_indexCount = {}; ///< Number of indices per LOD.
		std::vector<std::vector<unsigned int>> m_lodIndices; ///< CPU-side index data per LOD.
		unsigned int m_vertexCount = 0; ///< Total vertex count.
	};
}//Pengu::Graphics

#endif // !MESH_HPP
