/**
 * @file ResourceManager.hpp
 * @brief Centralized resource management for the engine.
 */

#ifndef RESOURCEMANAGER_HPP
#define RESOURCEMANAGER_HPP 1

#include <memory>
#include <string>
#include <unordered_map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Pengu_Engine/Graphics/Mesh.hpp"
#include "Pengu_Engine/Graphics/Texture.hpp"
#include "Pengu_Engine/Graphics/Material.hpp"
#include "Pengu_Engine/Graphics/Shader.hpp"
#include "Pengu_Engine/Graphics/GameObject.hpp"
#include "Pengu_Engine/Misc/JobSystem.hpp"

namespace Pengu::Resources {

	/**
	 * @class ResourceManager
	 * @brief Manages the lifecycle and caching of engine resources.
	 *
	 * The resource manager ensures that resources like meshes, textures, and shaders
	 * are only loaded once and can be shared across multiple objects. It also
	 * supports asynchronous resource loading via the job system.
	 */
	class ResourceManager {
	public:
		/**
		 * @brief Constructs the resource manager.
		 * @param jobSystem Optional pointer to the job system for async loading.
		 */
		ResourceManager(JobSystem* jobSystem = nullptr) : m_jobSystem(jobSystem) {}

		/// @name Deleted Operations
		/// @{
		ResourceManager(const ResourceManager& rvalue) = delete;
		ResourceManager& operator = (const ResourceManager& rvalue) = delete;
		/// @}

		/**
		 * @brief Move constructor.
		 * @param rvalue The resource manager to move from.
		 */
		ResourceManager(ResourceManager&& rvalue) noexcept : m_meshes(std::move(rvalue.m_meshes)),
			m_textures(std::move(rvalue.m_textures)), m_shaders(std::move(rvalue.m_shaders)),
			m_materials(std::move(rvalue.m_materials)), m_jobSystem(std::move(rvalue.m_jobSystem))
		{
		};

		/**
		 * @brief Move assignment operator.
		 * @param rvalue The resource manager to move from.
		 * @return Reference to this resource manager.
		 */
		ResourceManager& operator = (ResourceManager&& rvalue) noexcept
		{
			if (this != &rvalue)
			{
				m_meshes = std::move(rvalue.m_meshes);
				m_textures = std::move(rvalue.m_textures);
				m_shaders = std::move(rvalue.m_shaders);
				m_materials = std::move(rvalue.m_materials);
				m_jobSystem = std::move(rvalue.m_jobSystem);
			}

			return *this;
		};

		/**
		 * @brief Destructor.
		 */
		~ResourceManager();

		/**
		 * @brief Executes pending tasks that must run on the main thread (e.g., OpenGL calls).
		 */
		void UpdateMainThreadTasks();

		/**
		 * @brief Retrieves or creates a mesh.
		 * @param name Unique name for the mesh.
		 * @param vertices Vertex data.
		 * @param indices Index data.
		 * @return Shared pointer to the mesh.
		 */
		std::shared_ptr<Pengu::Graphics::Mesh> getMesh(const std::string& name,
			const std::vector<Pengu::Graphics::Vertex>& vertices,
			const std::vector<unsigned int>& indices);

		/**
		 * @brief Retrieves or loads a texture from a file.
		 * @param path Path to the texture file.
		 * @return Shared pointer to the texture.
		 */
		std::shared_ptr<Pengu::Graphics::Texture> getTexture(const std::string& path);

		/**
		 * @brief Retrieves or compiles a shader program.
		 * @param vertPath Path to the vertex shader file.
		 * @param fragPath Path to the fragment shader file.
		 * @return Shared pointer to the shader.
		 */
		std::shared_ptr<Pengu::Graphics::Shader> getShader(const std::string& vertPath,
			const std::string& fragPath);

		/**
		 * @brief Retrieves or creates a material.
		 * @param name Unique name for the material.
		 * @return Shared pointer to the material.
		 */
		std::shared_ptr<Pengu::Graphics::Material> getMaterial(const std::string& name);

		/**
		 * @brief Synchronously loads a 3D model file.
		 * @param path Path to the model file (e.g., .obj, .fbx, .gltf).
		 * @return Shared pointer to the root GameObject.
		 */
		std::shared_ptr<Pengu::Graphics::GameObject> loadObject(const std::string& path);

		/**
		 * @brief Asynchronously loads a 3D model file.
		 * @param path Path to the model file.
		 * @param onComplete Callback function executed when loading is finished.
		 */
		void LoadObjectAsync(const std::string& path, std::function<void(std::shared_ptr<Pengu::Graphics::GameObject>)> onComplete);

		/**
		 * @brief Creates a plane primitive.
		 * @param name Name of the object.
		 * @param rows Number of rows in the grid.
		 * @param cols Number of columns in the grid.
		 * @param size Side length of each grid cell.
		 * @param mat Material to apply.
		 * @return Shared pointer to the created GameObject.
		 */
		std::shared_ptr<Pengu::Graphics::GameObject> CreatePlane(const std::string& name, int rows, int cols, int size, std::shared_ptr<Pengu::Graphics::Material> mat);

		/**
		 * @brief Creates a cube primitive with 8 vertices (shared corners).
		 * @param name Name of the object.
		 * @param size Side length.
		 * @param mat Material to apply.
		 * @return Shared pointer to the created GameObject.
		 */
		std::shared_ptr<Pengu::Graphics::GameObject> CreateCube8v(const std::string& name, const float size, std::shared_ptr<Pengu::Graphics::Material> mat);

		/**
		 * @brief Creates a cube primitive with 24 vertices (distinct faces).
		 * @param name Name of the object.
		 * @param size Side length.
		 * @param mat Material to apply.
		 * @return Shared pointer to the created GameObject.
		 */
		std::shared_ptr<Pengu::Graphics::GameObject> CreateCube24v(const std::string& name, const float size, std::shared_ptr<Pengu::Graphics::Material> mat);

		/**
		 * @brief Creates a quad primitive.
		 * @param name Name of the object.
		 * @param size Side length.
		 * @param mat Material to apply.
		 * @return Shared pointer to the created GameObject.
		 */
		std::shared_ptr<Pengu::Graphics::GameObject> CreateQuad(const std::string& name, const float size, std::shared_ptr<Pengu::Graphics::Material> mat);

		/**
		 * @brief Creates a sphere primitive.
		 * @param name Name of the object.
		 * @param size Radius.
		 * @param height Vertical segments.
		 * @param revs Horizontal segments.
		 * @param mat Material to apply.
		 * @return Shared pointer to the created GameObject.
		 */
		std::shared_ptr<Pengu::Graphics::GameObject> CreateSphere(const std::string& name, const float size, const int height, const int revs, std::shared_ptr<Pengu::Graphics::Material> mat);

		/**
		 * @brief Creates a GameObject from raw vertex and index data.
		 * @param name Name of the object.
		 * @param vertices Vertex data.
		 * @param lodIndices Vector of index buffers for different LOD levels.
		 * @param mat Material to apply.
		 * @return Shared pointer to the created GameObject.
		 */
		std::shared_ptr<Pengu::Graphics::GameObject> createGameObjectFromData(
			const std::string& name,
			const std::vector<Pengu::Graphics::Vertex>& vertices,
			const std::vector<std::vector<unsigned int>>& lodIndices,
			std::shared_ptr<Pengu::Graphics::Material> mat
		);

		/**
		 * @brief Updates the mesh data for an existing GameObject.
		 * @param object The object to update.
		 * @param vertices New vertex data.
		 * @param lodIndices New index data.
		 */
		void updateGameObjectData(std::shared_ptr<Pengu::Graphics::GameObject> object,
			const std::vector<Pengu::Graphics::Vertex>& vertices,
			const std::vector<std::vector<unsigned int>>& lodIndices);


		/**
		 * @brief Sets the job system for asynchronous operations.
		 * @param js Pointer to the job system.
		 */
		void SetJobSystem(JobSystem* js) { m_jobSystem = js; }
	private:

		/**
		 * @brief Extracts material data from an Assimp material.
		 */
		std::shared_ptr<Pengu::Graphics::Material> extractMaterial(aiMaterial* aiMat,
			const std::string& dir,
			const std::vector<std::shared_ptr<Pengu::Graphics::RawTextureData>>& preloadedTextures);

		/**
		 * @brief Extracts material data from an Assimp material.
		 */
		std::shared_ptr<Pengu::Graphics::Material> extractMaterial(aiMaterial* aiMat,
			const std::string& dir);


		JobSystem* m_jobSystem = nullptr; ///< Pointer to the job system.

		std::mutex m_taskMutex; ///< Mutex protecting the main thread task queue.
		std::queue<std::function<void()>> m_mainThreadTasks; ///< Queue of tasks to be executed on the main thread.

		std::unordered_map<std::string, std::weak_ptr<Pengu::Graphics::Mesh>>     m_meshes;    ///< Cache of loaded meshes.
		std::unordered_map<std::string, std::weak_ptr<Pengu::Graphics::Texture>>  m_textures;  ///< Cache of loaded textures.
		std::unordered_map<std::string, std::weak_ptr<Pengu::Graphics::Shader>>   m_shaders;   ///< Cache of loaded shaders.
		std::unordered_map<std::string, std::weak_ptr<Pengu::Graphics::Material>> m_materials; ///< Cache of created materials.
	};
}// Pengu::Resources

#endif // !RESOURCEMANAGER_HPP
