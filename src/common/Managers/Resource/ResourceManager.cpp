#include "Pengu_Engine/Managers/Resource/ResourceManager.hpp"
#include "Pengu_Engine/Misc/Logmacros.hpp"
#include "Pengu_Engine/Objects/plane.hpp"
#include "Pengu_Engine/Objects/cube.hpp"
#include "Pengu_Engine/Objects/quad.hpp"
#include "Pengu_Engine/Objects/sphere.hpp"
#include <filesystem>
#include <../src/vendor/FastNoiseLite/FastNoiseLite.h>
#include <glm/glm.hpp>

namespace fs = std::filesystem;

namespace Pengu::Resources {

	ResourceManager::~ResourceManager()
	{
		m_meshes.clear();
		m_textures.clear();
		m_shaders.clear();
		m_materials.clear();
	}

	void ResourceManager::UpdateMainThreadTasks()
	{
		std::function<void()> task;
		while (true) {
			{
				std::lock_guard<std::mutex> lock(m_taskMutex);
				if (m_mainThreadTasks.empty()) break;

				task = std::move(m_mainThreadTasks.front());
				m_mainThreadTasks.pop();
			}
			// Execute the task on the main thread
			task();
		}
	}

	std::shared_ptr<Pengu::Graphics::Mesh> ResourceManager::getMesh(const std::string& name,
		const std::vector<Pengu::Graphics::Vertex>& vertices,
		const std::vector<uint32_t>& indices)
	{
		// Return cached version if it exists
		auto it = m_meshes.find(name);
		if (it != m_meshes.end())
			if (auto mesh = it->second.lock())
				return mesh;

		auto mesh = std::make_shared<Pengu::Graphics::Mesh>();
		mesh->upload(vertices, indices);

		m_meshes[name] = mesh;
		LOG_INFO("ResourceManager Created mesh: {}", name);
		return mesh;
	}

	std::shared_ptr<Pengu::Graphics::Texture> ResourceManager::getTexture(const std::string& path)
	{

		auto it = m_textures.find(path);
		if (it != m_textures.end())
			if (auto tex = it->second.lock())
				return tex;

		auto texture = std::make_shared<Pengu::Graphics::Texture>();
		if (!texture->LoadFromFile(path)) {
			LOG_ERROR("ResourceManager Failed to load texture: {}", path);
			return nullptr;
		}

		m_textures[path] = texture;
		LOG_INFO("ResourceManager Loaded texture: {}", path);
		return texture;
	}

	std::shared_ptr<Pengu::Graphics::Shader> ResourceManager::getShader(const std::string& vertPath, const std::string& fragPath)
	{
		std::string key = vertPath + "|" + fragPath;

		auto it = m_shaders.find(key);
		if (it != m_shaders.end())
			if (auto shader = it->second.lock())
				return shader;

		auto shader = std::make_shared<Pengu::Graphics::Shader>();
		shader->load(vertPath, fragPath);

		m_shaders[key] = shader;
		LOG_INFO("ResourceManager Loaded shader: {}", key);
		return shader;
	}

	std::shared_ptr<Pengu::Graphics::Material> ResourceManager::getMaterial(const std::string& name)
	{
		auto it = m_materials.find(name);
		if (it != m_materials.end())
			if (auto mat = it->second.lock())
				return mat;

		auto material = std::make_shared<Pengu::Graphics::Material>();
		m_materials[name] = material;
		return material;
	}

	std::shared_ptr<Pengu::Graphics::GameObject> ResourceManager::loadObject(const std::string& path)
	{
		std::string absPath = fs::absolute(path).lexically_normal().string();
		std::replace(absPath.begin(), absPath.end(), '\\', '/');

		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(absPath,
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_JoinIdenticalVertices |
			aiProcess_CalcTangentSpace
		);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			LOG_ERROR("ResourceManager Assimp error: {}", importer.GetErrorString());
			return nullptr;
		}

		std::string dir = absPath.substr(0, absPath.find_last_of('/'));
		auto object = std::make_shared<Pengu::Graphics::GameObject>();

		for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
			aiMesh* aiM = scene->mMeshes[i];
			aiMaterial* aiMat = scene->mMaterials[aiM->mMaterialIndex];

			auto mesh = std::make_shared<Pengu::Graphics::Mesh>();
			mesh->loadFromAssimp(aiM);
			mesh->CalculateBounds();

			auto material = extractMaterial(aiMat, dir);

			object->addSubMesh(mesh, material);
		}

		LOG_INFO("ResourceManager Loaded entity: {} ({} submeshes)", path, object->subMeshes.size());
		return object;
	}

	void ResourceManager::LoadObjectAsync(const std::string& path, std::function<void(std::shared_ptr<Pengu::Graphics::GameObject>)> onComplete)
	{
		if (!m_jobSystem) {
			LOG_ERROR("ResourceManager: JobSystem is null! Cannot load async.");
			if (onComplete) onComplete(nullptr);
			return;
		}

		m_jobSystem->Enqueue([this, path, onComplete]() {

			// ==========================================
			// 1. WORKER THREAD (CPU ONLY)
			// ==========================================

			std::string absPath = fs::absolute(path).lexically_normal().string();
			std::replace(absPath.begin(), absPath.end(), '\\', '/');

			auto importer = std::make_shared<Assimp::Importer>();
			const aiScene* scene = importer->ReadFile(absPath,
				aiProcess_Triangulate | aiProcess_GenSmoothNormals |
				aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace
			);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
				LOG_ERROR("ResourceManager Assimp error: {}", importer->GetErrorString());

				if (onComplete) {
					std::lock_guard<std::mutex> lock(m_taskMutex);
					m_mainThreadTasks.push([onComplete]() { onComplete(nullptr); });
				}
				return;
			}

			std::string dir = absPath.substr(0, absPath.find_last_of('/'));
			auto object = std::make_shared<Pengu::Graphics::GameObject>();

			// Pre-process meshes on the worker thread
			std::vector<std::shared_ptr<Pengu::Graphics::Mesh>> preloadedMeshes;
			for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
				auto mesh = std::make_shared<Pengu::Graphics::Mesh>();
				mesh->threadLoadFromAssimp(scene->mMeshes[i]);
				mesh->CalculateBounds();
				preloadedMeshes.push_back(mesh);
			}

			// Pre-process textures on the worker thread
			std::vector<std::shared_ptr<Pengu::Graphics::RawTextureData>> preloadedTextures;
			for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
				aiMaterial* aiMat = scene->mMaterials[i];

				std::vector<aiTextureType> typesToLoad = {
				aiTextureType_BASE_COLOR,
				aiTextureType_DIFFUSE,
				aiTextureType_NORMALS,
				aiTextureType_METALNESS,
				aiTextureType_DIFFUSE_ROUGHNESS,
				aiTextureType_AMBIENT_OCCLUSION,
				aiTextureType_UNKNOWN // CRITICAL for packed glTF textures!
				};

				for (aiTextureType type : typesToLoad) {
					for (unsigned int j = 0; j < aiMat->GetTextureCount(type); j++) {
						aiString aiPath;
						aiMat->GetTexture(type, j, &aiPath);
						std::string fullPath = dir + "/" + aiPath.C_Str();

						// Prevent loading the same texture twice if multiple materials share it
						auto it = std::find_if(preloadedTextures.begin(), preloadedTextures.end(),
							[&fullPath](const std::shared_ptr<Pengu::Graphics::RawTextureData>& data) {
								return data->path == fullPath;
							});

						if (it == preloadedTextures.end()) {
							preloadedTextures.push_back(Pengu::Graphics::Texture::LoadDataFromDisk(fullPath));
						}
					}
				}
			}

			// ==========================================
			// 2. DISPATCH TO MAIN THREAD (GPU UPLOADS)
			// ==========================================

			{
				std::lock_guard<std::mutex> lock(m_taskMutex);
				m_mainThreadTasks.push([this, object, scene, importer, dir, preloadedMeshes, preloadedTextures, onComplete, path]() {

					// Main Thread OpenGL execution
					for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
						aiMesh* aiM = scene->mMeshes[i];
						aiMaterial* aiMat = scene->mMaterials[aiM->mMaterialIndex];

						auto mesh = preloadedMeshes[i];
						mesh->uploadToGPU();

						// extractMaterial now uses the data decoded by your worker thread
						auto material = extractMaterial(aiMat, dir, preloadedTextures);

						object->addSubMesh(mesh, material);
					}

					LOG_INFO("ResourceManager Async Loaded entity: {} ({} submeshes)", path, object->subMeshes.size());

					if (onComplete) {
						onComplete(object);
					}
					});
			}
			});
	}

	std::shared_ptr<Pengu::Graphics::GameObject> ResourceManager::CreatePlane(const std::string& name, int rows, int cols, int size, std::shared_ptr<Pengu::Graphics::Material> mat)
	{
		auto it = m_meshes.find(name);
		if (it != m_meshes.end())
			if (auto mesh = it->second.lock())
			{
				auto object = std::make_shared<Pengu::Graphics::GameObject>();
				object->addSubMesh(mesh, mat);
				return object;
			}

		Plane plane(rows, cols, size);
		auto mesh = std::make_shared<Pengu::Graphics::Mesh>();
		mesh->upload(plane.data_.vertices, plane.data_.indices);
		mesh->CalculateBounds();

		m_meshes[name] = mesh;

		auto object = std::make_shared<Pengu::Graphics::GameObject>();
		object->addSubMesh(mesh, mat);
		return object;
	}

	std::shared_ptr<Pengu::Graphics::GameObject> ResourceManager::CreateCube8v(const std::string& name, const float size, std::shared_ptr<Pengu::Graphics::Material> mat)
	{
		auto it = m_meshes.find(name);
		if (it != m_meshes.end())
			if (auto mesh = it->second.lock())
			{
				auto object = std::make_shared<Pengu::Graphics::GameObject>();
				object->addSubMesh(mesh, mat);
				return object;
			}

		Cube cube(size);
		cube.init8v();
		auto mesh = std::make_shared<Pengu::Graphics::Mesh>();
		mesh->upload(cube.m_data.vertices, cube.m_data.indices);

		m_meshes[name] = mesh;

		auto object = std::make_shared<Pengu::Graphics::GameObject>();
		object->addSubMesh(mesh, mat);
		return object;
	}

	std::shared_ptr<Pengu::Graphics::GameObject> ResourceManager::CreateCube24v(const std::string& name, const float size, std::shared_ptr<Pengu::Graphics::Material> mat)
	{
		auto it = m_meshes.find(name);
		if (it != m_meshes.end())
			if (auto mesh = it->second.lock())
			{
				auto object = std::make_shared<Pengu::Graphics::GameObject>();
				object->addSubMesh(mesh, mat);
				return object;
			}

		Cube cube(size);
		cube.init24v();
		auto mesh = std::make_shared<Pengu::Graphics::Mesh>();
		mesh->upload(cube.m_data.vertices, cube.m_data.indices);
		mesh->CalculateBounds();

		m_meshes[name] = mesh;

		auto object = std::make_shared<Pengu::Graphics::GameObject>();
		object->addSubMesh(mesh, mat);
		return object;
	}

	std::shared_ptr<Pengu::Graphics::GameObject> ResourceManager::CreateQuad(const std::string& name, const float size, std::shared_ptr<Pengu::Graphics::Material> mat)
	{
		auto it = m_meshes.find(name);
		if (it != m_meshes.end())
			if (auto mesh = it->second.lock())
			{
				auto object = std::make_shared<Pengu::Graphics::GameObject>();
				object->addSubMesh(mesh, mat);
				return object;
			}

		Quad quad(size);
		auto mesh = std::make_shared<Pengu::Graphics::Mesh>();
		mesh->upload(quad.m_data.vertices, quad.m_data.indices);
		mesh->CalculateBounds();

		m_meshes[name] = mesh;

		auto object = std::make_shared<Pengu::Graphics::GameObject>();
		object->addSubMesh(mesh, mat);
		return object;
	}

	std::shared_ptr<Pengu::Graphics::GameObject> ResourceManager::CreateSphere(const std::string& name, const float size, const int height, const int revs, std::shared_ptr<Pengu::Graphics::Material> mat)
	{
		auto it = m_meshes.find(name);
		if (it != m_meshes.end())
			if (auto mesh = it->second.lock())
			{
				auto object = std::make_shared<Pengu::Graphics::GameObject>();
				object->addSubMesh(mesh, mat);
				return object;
			}

		Sphere sphere(size, height, revs);
		auto mesh = std::make_shared<Pengu::Graphics::Mesh>();
		mesh->upload(sphere.m_data.vertices, sphere.m_data.indices);
		mesh->CalculateBounds();

		m_meshes[name] = mesh;

		auto object = std::make_shared<Pengu::Graphics::GameObject>();
		object->addSubMesh(mesh, mat);
		return object;
	}

	std::shared_ptr<Pengu::Graphics::GameObject> ResourceManager::createGameObjectFromData(const std::string& name,
		const std::vector<Pengu::Graphics::Vertex>& vertices,
		const std::vector<std::vector<unsigned int>>& lodIndices,
		std::shared_ptr<Pengu::Graphics::Material> mat)
	{
		// 1. Create the Mesh (as we did before)
		auto mesh = std::make_shared<Pengu::Graphics::Mesh>();
		mesh->upload(vertices, lodIndices);

		// Cache the mesh so we don't re-upload if not needed
		m_meshes[name] = mesh;

		// 2. Create the GameObject
		auto object = std::make_shared<Pengu::Graphics::GameObject>();

		// 3. Add the submesh using the provided material
		// This is what makes it "Drawable" in your engine's eyes
		object->addSubMesh(mesh, mat);

		return object;
	}

	void ResourceManager::updateGameObjectData(std::shared_ptr<Pengu::Graphics::GameObject> object,
		const std::vector<Pengu::Graphics::Vertex>& vertices,
		const std::vector<std::vector<unsigned int>>& lodIndices)
	{
		if (!object || object->subMeshes.empty()) return;

		object->subMeshes[0].mesh_->upload(vertices, lodIndices);
	}

	std::shared_ptr<Pengu::Graphics::Material> ResourceManager::extractMaterial(aiMaterial* aiMat,
		const std::string& dir,
		const std::vector<std::shared_ptr<Pengu::Graphics::RawTextureData>>& preloadedTextures)
	{
		// Use material name as cache key
		aiString aiName;
		aiMat->Get(AI_MATKEY_NAME, aiName);
		std::string name = dir + "/" + aiName.C_Str();

		// Return cached material if already loaded
		auto it = m_materials.find(name);
		if (it != m_materials.end()) {
			if (auto mat = it->second.lock()) {
				return mat;
			}
		}

		auto material = std::make_shared<Pengu::Graphics::Material>();

		// Helper to load a texture by type using the PRELOADED memory
		auto loadTex = [&](aiTextureType type) -> std::shared_ptr<Pengu::Graphics::Texture> {
			if (aiMat->GetTextureCount(type) == 0)
				return nullptr;

			aiString aiPath;
			aiMat->GetTexture(type, 0, &aiPath);
			std::string fullPath = dir + "/" + aiPath.C_Str();

			// 1. Check if the texture is ALREADY on the GPU (Global Engine Cache)
			auto texIt = m_textures.find(fullPath);
			if (texIt != m_textures.end()) {
				if (auto tex = texIt->second.lock()) {
					return tex;
				}
			}

			// 2. If not on GPU, find its raw pixel data in our preloaded array
			for (const auto& rawData : preloadedTextures) {
				if (rawData->path == fullPath) {

					// 3. Create the OpenGL Texture object and upload the pixels
					auto texture = std::make_shared<Pengu::Graphics::Texture>();
					if (texture->UploadToGPU(rawData)) {

						// 4. Cache it so other materials/models can reuse it!
						m_textures[fullPath] = texture;
						LOG_INFO("ResourceManager Uploaded texture to GPU: {}", fullPath);
						return texture;
					}
				}
			}

			LOG_ERROR("ResourceManager: Missing preloaded texture data for {}", fullPath);
			return nullptr;
			};

		// --- Assign Material Properties ---

		// Albedo
		material->albedoMap = loadTex(aiTextureType_DIFFUSE);
		if (!material->albedoMap) {
			aiColor4D color;
			if (AI_SUCCESS == aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_DIFFUSE, &color))
				material->albedoColor = { color.r, color.g, color.b, color.a };
		}

		//Normal Map (Critical for tangent-space lighting)
		material->normalMap = loadTex(aiTextureType_NORMALS);

		// Roughness
		float roughness = 0.5f;
		aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
		material->roughness = roughness;

		// Metallic
		float metallic = 0.0f;
		aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
		material->metallic = metallic;

		//Metallic Map
		material->metallicMap = loadTex(aiTextureType_METALNESS);
		if (!material->metallicMap) {
			float metallic = 0.0f;
			aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
			material->metallic = metallic;
		}

		//Roughness Map
		material->roughnessMap = loadTex(aiTextureType_DIFFUSE_ROUGHNESS);
		if (!material->roughnessMap) {
			float roughness = 0.5f;
			aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
			material->roughness = roughness;
		}

		//Ambient Occlusion Map
		material->aoMap = loadTex(aiTextureType_AMBIENT_OCCLUSION);

		// (If you add Normal Maps later, you just do: material->normalMap = loadTex(aiTextureType_NORMALS);)

		m_materials[name] = material;
		return material;
	}

	std::shared_ptr<Pengu::Graphics::Material> ResourceManager::extractMaterial(aiMaterial* aiMat, const std::string& dir)
	{
		aiString aiName;
		aiMat->Get(AI_MATKEY_NAME, aiName);
		std::string name = dir + "/" + aiName.C_Str();

		auto it = m_materials.find(name);
		if (it != m_materials.end())
			if (auto mat = it->second.lock())
				return mat;

		auto material = std::make_shared<Pengu::Graphics::Material>();

		// Helper to load a texture synchronously from the hard drive
		auto loadTex = [&](aiTextureType type) -> std::shared_ptr<Pengu::Graphics::Texture> {
			if (aiMat->GetTextureCount(type) == 0)
				return nullptr;
			aiString aiPath;
			aiMat->GetTexture(type, 0, &aiPath);

			// This calls your existing getTexture method, which blocks the thread to read the disk
			return getTexture(dir + "/" + aiPath.C_Str());
			};

		// Albedo
		material->albedoMap = loadTex(aiTextureType_BASE_COLOR);
		if (!material->albedoMap) {
			material->albedoMap = loadTex(aiTextureType_DIFFUSE); // Fallback for older .obj/.fbx
		}
		if (!material->albedoMap) {
			aiColor4D color(1.0f, 1.0f, 1.0f, 1.0f);
			aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_DIFFUSE, &color);
			material->albedoColor = { color.r, color.g, color.b, color.a };
		}

		//Normal Map (Critical for tangent-space lighting)
		material->normalMap = loadTex(aiTextureType_NORMALS);

		//Metallic Map
		material->metallicMap = loadTex(aiTextureType_METALNESS);
		if (!material->metallicMap) {
			float metallic = 0.0f;
			aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
			material->metallic = metallic;
		}

		//Roughness Map
		material->roughnessMap = loadTex(aiTextureType_DIFFUSE_ROUGHNESS);
		if (!material->roughnessMap) {
			float roughness = 0.5f;
			aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
			material->roughness = roughness;
		}

		//Ambient Occlusion Map
		material->aoMap = loadTex(aiTextureType_AMBIENT_OCCLUSION);

		m_materials[name] = material;
		return material;
	}

}//Pengu::Resources