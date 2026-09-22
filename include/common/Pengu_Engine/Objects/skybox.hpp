/**
 * @file skybox.hpp
 * @brief Skybox rendering and management.
 */

#ifndef SKYBOX_HPP
#define SKYBOX_HPP 1

#include "Pengu_Engine/Graphics/Shader.hpp"
#include <glm/glm.hpp>
#include <vector>

/**
 * @class SkyBox
 * @brief Manages a cubemap-based skybox.
 */
class SkyBox {

public:
	/**
	 * @brief Constructs a skybox from 6 image paths.
	 * @param facePaths Paths to the 6 textures (PX, NX, PY, NY, PZ, NZ).
	 */
	SkyBox(const std::vector<std::string>& facePaths);

	/**
	 * @brief Destructor that cleans up GPU resources.
	 */
	~SkyBox();

	/// @name Deleted Operations
	/// @{
	SkyBox(const SkyBox&) = delete;
	SkyBox& operator=(const SkyBox&) = delete;
	/// @}

	/**
	 * @brief Renders the skybox.
	 * @param skyboxShader The shader to use for rendering.
	 * @param view The view matrix (should have translation removed).
	 * @param projection The projection matrix.
	 */
	void Render(Pengu::Graphics::Shader& skyboxShader, glm::mat4 view, glm::mat4 projection);

	glm::vec3 fogColor = glm::vec3(0.5f, 0.6f, 0.7f); ///< Color of the fog at the horizon.
	float fogDensity = 0.0015f;                       ///< Density of the horizon fog.

private:
	unsigned int vao_; ///< OpenGL VAO ID.
	unsigned int vbo_; ///< OpenGL VBO ID.
	unsigned int cubemapTextureID_; ///< OpenGL Texture ID for the cubemap.

	/**
	 * @brief Sets up the cube geometry for the skybox.
	 */
	void SetupCube();

	/**
	 * @brief Loads the 6 textures into a single OpenGL cubemap.
	 * @param faces Paths to the 6 face textures.
	 * @return The OpenGL texture ID.
	 */
	unsigned int LoadCubemap(const std::vector<std::string>& faces);
};

#endif // !SKYBOX_HPP
