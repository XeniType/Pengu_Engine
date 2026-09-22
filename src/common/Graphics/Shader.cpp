#include "Pengu_Engine/Graphics/Shader.hpp"
#include "Pengu_Engine/Misc/Logmacros.hpp"
#include <iostream>
namespace Pengu::Graphics {
	void Shader::load(const std::string& vertexPath, const std::string& fragmentPath)
	{

		std::string vertSrc = readFile(vertexPath);
		std::string fragSrc = readFile(fragmentPath);

		if (vertSrc.empty() || fragSrc.empty()) {
			LOG_ERROR("Shader Failed to read shader files");
			return;
		}

		unsigned int vert = compile(vertSrc.c_str(), GL_VERTEX_SHADER);
		unsigned int frag = compile(fragSrc.c_str(), GL_FRAGMENT_SHADER);

		if (!vert || !frag) {
			glDeleteShader(vert);
			glDeleteShader(frag);
			return;
		}

		link(vert, frag);

		glDeleteShader(vert);
		glDeleteShader(frag);
	}

	void Shader::setBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(s_id, name.c_str()), (int)value);
	}
	// ------------------------------------------------------------------------
	void Shader::setInt(const std::string& name, int value) const
	{
		GLint loc = glGetUniformLocation(s_id, name.c_str());
		glUniform1i(loc, value);
	}
	// ------------------------------------------------------------------------
	void Shader::setUInt(const std::string& name, unsigned int value) const
	{
		GLint loc = glGetUniformLocation(s_id, name.c_str());
		glUniform1ui(loc, value);
	}
	// ------------------------------------------------------------------------
	void Shader::setFloat(const std::string& name, float value) const
	{
		GLint loc = glGetUniformLocation(s_id, name.c_str());
		glUniform1f(loc, value);
	}
	// ------------------------------------------------------------------------
	void Shader::setVec2(const std::string& name, const glm::vec2& value) const
	{
		glUniform2fv(glGetUniformLocation(s_id, name.c_str()), 1, &value[0]);
	}
	void Shader::setVec2(const std::string& name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(s_id, name.c_str()), x, y);
	}
	// ------------------------------------------------------------------------
	void Shader::setVec3(const std::string& name, const glm::vec3& value) const
	{
		GLint loc = glGetUniformLocation(s_id, name.c_str());
		glUniform3fv(loc, 1, &value[0]);
	}
	void Shader::setVec3(const std::string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(s_id, name.c_str()), x, y, z);
	}
	// ------------------------------------------------------------------------
	void Shader::setVec4(const std::string& name, const glm::vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(s_id, name.c_str()), 1, &value[0]);
	}
	void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
	{
		glUniform4f(glGetUniformLocation(s_id, name.c_str()), x, y, z, w);
	}
	// ------------------------------------------------------------------------
	void Shader::setMat2(const std::string& name, const glm::mat2& mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(s_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(s_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(s_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	int Shader::loc(const char* name)
	{
		auto it = s_locCache.find(name);
		if (it != s_locCache.end())
			return it->second;

		int location = glGetUniformLocation(s_id, name);
		if (location == -1)
			LOG_WARNING("Shader Uniform not found: {}", name);

		s_locCache[name] = location;
		return location;
	}

	unsigned int Shader::compile(const char* src, GLenum type)
	{
		unsigned int id = glCreateShader(type);
		glShaderSource(id, 1, &src, nullptr);
		glCompileShader(id);

		int success;
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		if (!success) {
			char log[1024];
			glGetShaderInfoLog(id, 1024, nullptr, log);
			LOG_ERROR("Shader Compile error ({}): {}",
				type == GL_VERTEX_SHADER ? "vertex" : "fragment", log);
			glDeleteShader(id);
			return 0;
		}

		return id;
	}

	void Shader::link(unsigned int vert, unsigned int frag)
	{
		s_id = glCreateProgram();
		glAttachShader(s_id, vert);
		glAttachShader(s_id, frag);
		glLinkProgram(s_id);

		int success;
		glGetProgramiv(s_id, GL_LINK_STATUS, &success);
		if (!success) {
			char log[1024];
			glGetProgramInfoLog(s_id, 1024, nullptr, log);
			LOG_ERROR("Shader Link error: {}", log);
			glDeleteProgram(s_id);
			s_id = 0;
		}
	}

	std::string Shader::readFile(const std::string& path)
	{
		std::ifstream file(path);
		if (!file.is_open()) {
			LOG_ERROR("Shader Could not open file: {}", path);
			return "";
		}

		std::stringstream ss;
		ss << file.rdbuf();
		return ss.str();
	}
}