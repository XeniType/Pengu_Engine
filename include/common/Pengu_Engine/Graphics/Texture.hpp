/**
 * @file Texture.hpp
 * @brief OpenGL texture wrapper for 2D images.
 */

#ifndef TEXTURE_HPP
#define TEXTURE_HPP 1

#include "GL/glew.h"
#include <string>
#include <memory>

namespace Pengu::Graphics {

	/**
	 * @struct RawTextureData
	 * @brief Container for image data loaded from disk before it is sent to the GPU.
	 */
	struct RawTextureData {
		unsigned char* pixels = nullptr; ///< Raw pixel data buffer.
		int width = 0;                   ///< Image width in pixels.
		int height = 0;                  ///< Image height in pixels.
		int channels = 0;                ///< Number of color channels (e.g., 3 for RGB, 4 for RGBA).
		std::string path;                ///< Source file path.

		/**
		 * @brief Destructor that frees the pixel buffer.
		 */
		~RawTextureData();
	};

	/**
	 * @class Texture
	 * @brief Manages an OpenGL texture object.
	 */
	class Texture {
	public:
		/**
		 * @brief Default constructor.
		 */
		Texture() = default;

		/**
		 * @brief Loads a texture directly from a file and uploads it to the GPU.
		 * @param path Path to the image file.
		 * @return True if successful, false otherwise.
		 */
		bool LoadFromFile(const std::string& path);

		/**
		 * @brief Loads a texture from raw memory data.
		 * @param data Pointer to the raw pixel data.
		 * @param width Image width.
		 * @param height Image height.
		 * @param channels Number of channels.
		 * @return True if successful, false otherwise.
		 */
		bool LoadFromMemory(unsigned char* data, int width, int height, int channels);

		/**
		 * @brief Binds the texture to a specific OpenGL texture unit.
		 * @param slot The texture unit slot (default is 0).
		 */
		void Bind(unsigned int slot = 0);

		/**
		 * @brief Gets the source file path of the texture.
		 * @return Constant reference to the path string.
		 */
		const std::string& path() const { return t_path_; }

		/**
		 * @brief Gets the internal OpenGL texture ID.
		 * @return The OpenGL texture handle.
		 */
		unsigned int getID() const { return t_id_; }

		/**
		 * @brief Destructor that deletes the texture from the GPU.
		 */
		~Texture();

		/// @name Deleted Operations
		/// @{
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		/// @}

		/**
		 * @brief Move constructor.
		 */
		Texture(Texture&& other) noexcept
			: t_id_(other.t_id_), t_width_(other.t_width_), t_height_(other.t_height_), t_path_(std::move(other.t_path_)) {
			other.t_id_ = 0;
			other.t_width_ = 0;
			other.t_height_ = 0;
		}

		/**
		 * @brief Move assignment operator.
		 */
		Texture& operator=(Texture&& other) noexcept {
			if (this != &other) {
				if (t_id_) glDeleteTextures(1, &t_id_);
				t_id_ = other.t_id_;
				t_width_ = other.t_width_;
				t_height_ = other.t_height_;
				t_path_ = std::move(other.t_path_);
				other.t_id_ = 0;
				other.t_width_ = 0;
				other.t_height_ = 0;
			}
			return *this;
		}

		/**
		 * @brief Static helper to load image data from disk without creating an OpenGL texture.
		 * @param path Path to the image file.
		 * @return Shared pointer to the raw texture data.
		 */
		static std::shared_ptr<RawTextureData> LoadDataFromDisk(const std::string& path);

		/**
		 * @brief Uploads pre-loaded raw data to the GPU.
		 * @param rawData Shared pointer to the raw data container.
		 * @return True if successful, false otherwise.
		 */
		bool UploadToGPU(std::shared_ptr<RawTextureData> rawData);

	private:
		unsigned int t_id_ = 0;  ///< OpenGL texture ID.
		int t_width_ = 0;        ///< Texture width.
		int t_height_ = 0;       ///< Texture height.
		std::string t_path_;     ///< Source file path.
	};
}//Pengu::Graphics::Texture

#endif // !TEXTURE_HPP
