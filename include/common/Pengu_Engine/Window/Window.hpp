/**
 * @file Window.hpp
 * @brief GLFW window wrapper for managing the application window and OpenGL context.
 */

#ifndef WINDOW_HPP
#define WINDOW_HPP 1


#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include <optional>

/**
 * @class Window
 * @brief Encapsulates a GLFW window and its associated OpenGL context.
 */
class Window {
public:
	/**
	 * @brief Constructs a new window and initializes the OpenGL context.
	 * @param width Initial window width.
	 * @param height Initial window height.
	 * @param name Window title.
	 */
	Window(int width, int height, const std::string& name);

	/// @name Deleted Operations
	/// @{
	Window(const Window& rvalue) = delete;
	Window& operator = (const Window& rvalue) = delete;
	/// @}

	/**
	 * @brief Move constructor.
	 */
	Window(Window&& rvalue) noexcept : window_(rvalue.window_)
	{
		rvalue.window_ = nullptr;
	};

	/**
	 * @brief Move assignment operator.
	 */
	Window& operator = (Window&& rvalue) noexcept
	{
		if (this != &rvalue)
		{
			// Clean up existing resource
			if (window_)
			{
				glfwDestroyWindow(window_);
			}

			// Transfer ownership
			window_ = rvalue.window_;

			// Leave source in valid empty state
			rvalue.window_ = nullptr;
		}

		return *this;
	};

	/**
	 * @brief Static callback for window resize events.
	 */
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

	/**
	 * @brief Static callback for window iconify/minimize events.
	 */
	static void window_iconify_callback(GLFWwindow* window, int iconified);

	/** @brief Gets the current window width. */
	int GetWidth();
	/** @brief Gets the current window height. */
	int GetHeight();

	/** @brief Gets the pointer to the GLFW window. */
	GLFWwindow* GetWindow() const;

	/** @brief Gets the pointer to the resource-sharing GLFW window. */
	GLFWwindow* GetResourceContext() const { return resourceContext_; };

	/**
	 * @brief Destructor that destroys the GLFW window.
	 */
	~Window();

private:
	GLFWwindow* window_ = nullptr;          ///< The main GLFW window pointer.
	GLFWwindow* resourceContext_ = nullptr; ///< Secondary window for shared resource loading.
};

#endif // !__WINDOW_HPP__ 1
