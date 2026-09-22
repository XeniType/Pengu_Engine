#include "Pengu_Engine/Graphics/Texture.hpp"
#include "gl/glew.h"
#include "Pengu_Engine/Misc/Logmacros.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "Pengu_Engine/Misc/stb/stb_image.h"

namespace Pengu::Graphics {

	bool Texture::LoadFromFile(const std::string& path)
	{
		stbi_set_flip_vertically_on_load(true);

		int channels;
		auto* data = stbi_load(path.c_str(), &t_width_, &t_height_, &channels, 0);
		if (!data) {
			LOG_ERROR("Texture Failed to load {}", path);
			return false;
		}

		GLenum internalFormat, dataFormat;
		if (channels == 4) {
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3) {
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}
		else {
			internalFormat = GL_R8;
			dataFormat = GL_RED;
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &t_id_);

		glTextureStorage2D(t_id_, 1, internalFormat, t_width_, t_height_);
		glTextureSubImage2D(t_id_, 0, 0, 0, t_width_, t_height_, dataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateTextureMipmap(t_id_);

		glTextureParameteri(t_id_, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(t_id_, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(t_id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(t_id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
		t_path_ = path;
		return true;
	}

	bool Texture::LoadFromMemory(unsigned char* data, int width, int height, int channels)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &t_id_);
		glTextureStorage2D(t_id_, 1, GL_RGBA8, width, height);
		glTextureSubImage2D(t_id_, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glTextureParameteri(t_id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(t_id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(t_id_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(t_id_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		return true;
	}

	void Texture::Bind(unsigned int slot)
	{
		glBindTextureUnit(slot, t_id_);
	}

	Texture::~Texture()
	{
		if (t_id_) glDeleteTextures(1, &t_id_);
	}
	std::shared_ptr<RawTextureData> Texture::LoadDataFromDisk(const std::string& path)
	{
		auto data = std::make_shared<RawTextureData>();
		data->path = path;

		stbi_set_flip_vertically_on_load(true);
		data->pixels = stbi_load(path.c_str(), &data->width, &data->height, &data->channels, 0);

		if (!data->pixels) {
			LOG_ERROR("Texture Failed to decode from disk: {}", path);
		}
		return data;
	}
	bool Texture::UploadToGPU(std::shared_ptr<RawTextureData> rawData)
	{
		if (!rawData || !rawData->pixels) return false;

		GLenum internalFormat, dataFormat;
		if (rawData->channels == 4) {
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (rawData->channels == 3) {
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}
		else {
			internalFormat = GL_R8;
			dataFormat = GL_RED;
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &t_id_);
		glTextureStorage2D(t_id_, 1, internalFormat, rawData->width, rawData->height);
		glTextureSubImage2D(t_id_, 0, 0, 0, rawData->width, rawData->height, dataFormat, GL_UNSIGNED_BYTE, rawData->pixels);
		glGenerateTextureMipmap(t_id_);

		glTextureParameteri(t_id_, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(t_id_, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(t_id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(t_id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		t_width_ = rawData->width;
		t_height_ = rawData->height;
		t_path_ = rawData->path;

		return true;
	}


	RawTextureData::~RawTextureData() {
		if (pixels) {
			stbi_image_free(pixels);
			pixels = nullptr;
		}
	}
}