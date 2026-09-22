/**
 * @file Shader.hpp
 * @brief Simple OpenGL Shader program wrapper.
 */

#ifndef SHADER_HPP
#define SHADER_HPP 1

#include "gl/glew.h"

#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include <glm/glm.hpp>

namespace Pengu::Graphics {

	/**
	 * @class Shader
	 * @brief Manages the compilation, linking, and uniform distribution of GLSL shaders.
	 * 
	 * This class encapsulates an OpenGL Program ID and provides a high-level
	 * interface for sending data (uniforms) from the CPU to the GPU.
	 */
	class Shader {

	public:
		/**
		 * @brief Default constructor.
		 */
		Shader() : s_id(0) {}

		/**
		 * @brief Loads, compiles, and links a shader program from vertex and fragment source files.
		 * @param vertexPath Path to the vertex shader file.
		 * @param fragmentPath Path to the fragment shader file.
		 */
		void load(const std::string& vertexPath, const std::string& fragmentPath);

		/**
		 * @brief Activates the shader program.
		 */
		void bind() const { glUseProgram(s_id); }

		/**
		 * @brief Deactivates the shader program.
		 */
		void unbind() const { glUseProgram(0); }

		/**
		 * @brief Checks if the shader program is valid.
		 */
		bool isValid() const { return s_id != 0; }

		/**
		 * @brief Set a boolean uniform.
		 * @param name Uniform variable name.
		 * @param value Boolean value.
		 */
		void setBool(const std::string& name, bool value) const;

		/**
		 * @brief Set an integer uniform.
		 * @param name Uniform variable name.
		 * @param value Integer value.
		 */
		void setInt(const std::string& name, int value) const;

		/**
		 * @brief Set an unsigned integer uniform.
		 * @param name Uniform variable name.
		 * @param value Unsigned integer value.
		 */
		void setUInt(const std::string& name, unsigned int value) const;

		/**
		 * @brief Set a float uniform.
		 * @param name Uniform variable name.
		 * @param value Float value.
		 */
		void setFloat(const std::string& name, float value) const;

		/**
		 * @brief Set a 2D vector uniform.
		 * @param name Uniform variable name.
		 * @param value glm::vec2 value.
		 */
		void setVec2(const std::string& name, const glm::vec2& value) const;

		/**
		 * @brief Set a 2D vector uniform using raw components.
		 * @param name Uniform variable name.
		 * @param x X component.
		 * @param y Y component.
		 */
		void setVec2(const std::string& name, float x, float y) const;

		/**
		 * @brief Set a 3D vector uniform.
		 * @param name Uniform variable name.
		 * @param value glm::vec3 value.
		 */
		void setVec3(const std::string& name, const glm::vec3& value) const;

		/**
		 * @brief Set a 3D vector uniform using raw components.
		 * @param name Uniform variable name.
		 * @param x X component.
		 * @param y Y component.
		 * @param z Z component.
		 */
		void setVec3(const std::string& name, float x, float y, float z) const;

		/**
		 * @brief Set a 4D vector uniform.
		 * @param name Uniform variable name.
		 * @param value glm::vec4 value.
		 */
		void setVec4(const std::string& name, const glm::vec4& value) const;

		/**
		 * @brief Set a 4D vector uniform using raw components.
		 * @param name Uniform variable name.
		 * @param x X component.
		 * @param y Y component.
		 * @param z Z component.
		 * @param w W component.
		 */
		void setVec4(const std::string& name, float x, float y, float z, float w) const;

		/**
		 * @brief Set a 2x2 matrix uniform.
		 * @param name Uniform variable name.
		 * @param mat 2x2 matrix.
		 */
		void setMat2(const std::string& name, const glm::mat2& mat) const;

		/**
		 * @brief Set a 3x3 matrix uniform.
		 * @param name Uniform variable name.
		 * @param mat 3x3 matrix.
		 */
		void setMat3(const std::string& name, const glm::mat3& mat) const;

		/**
		 * @brief Set a 4x4 matrix uniform.
		 * @param name Uniform variable name.
		 * @param mat 4x4 matrix.
		 */
		void setMat4(const std::string& name, const glm::mat4& mat) const;

		/// @name Deleted Operations
		/// @{
		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;
		/// @}

		/**
		 * @brief Move constructor.
		 */
		Shader(Shader&& other) noexcept {
			s_id = other.s_id;
			s_locCache = std::move(other.s_locCache);
			other.s_id = 0;
		}

		/**
		 * @brief Move assignment operator.
		 */
		Shader& operator=(Shader&& other) noexcept {
			if (this != &other) {
				if (s_id) glDeleteProgram(s_id);
				s_id = other.s_id;
				s_locCache = std::move(other.s_locCache);
				other.s_id = 0;
			}
			return *this;
		}

		/**
		 * @brief Destructor that deletes the shader program from the GPU.
		 */
		~Shader() {
			if (s_id) glDeleteProgram(s_id);
		}
	private:
		unsigned int s_id; ///< Internal OpenGL Program ID handle.
		mutable std::unordered_map<std::string, int> s_locCache; ///< Cache of uniform locations.

		/**
		 * @brief Internal helper to get uniform location from name.
		 */
		int loc(const char* name);

		/**
		 * @brief Internal helper to compile a shader stage.
		 */
		unsigned int compile(const char* src, GLenum type);

		/**
		 * @brief Internal helper to link vertex and fragment shaders.
		 */
		void link(unsigned int vert, unsigned int frag);

		/**
		 * @brief Internal helper to read shader source file.
		 */
		std::string readFile(const std::string& path);

	};
}//Pengu::Graphics

#endif // !SHADER_HPP
