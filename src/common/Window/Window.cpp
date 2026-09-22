#include "Pengu_Engine/Window/Window.hpp"
#include "Pengu_Engine/PenguEngine.hpp"
#include <stdexcept>


Window::~Window() {
	if (window_) {
		glfwDestroyWindow(window_);
		window_ = nullptr;
	}
}

int Window::GetWidth() {
	int value;
	glfwGetWindowSize(window_, &value, NULL);
	return value;
}

int Window::GetHeight() {
	int value;
	glfwGetWindowSize(window_, NULL, &value);
	return value;
}

GLFWwindow* Window::GetWindow() const
{
	return window_;
}

Window::Window(int width, int height, const std::string& name)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window_ = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);

	if (!window_) {
		glfwTerminate();
		throw std::runtime_error("Failed to create window");
	}

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	resourceContext_ = glfwCreateWindow(1, 1, "ResourceLoader", NULL, window_);

	if (!resourceContext_) {
		glfwTerminate();
		throw std::runtime_error("Failed to create resource window");
	}

	glfwMakeContextCurrent(NULL);

	glfwMakeContextCurrent(window_);

	glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);
	glfwSetWindowIconifyCallback(window_, window_iconify_callback);

	glfwSwapInterval(0);
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{

	glViewport(0, 0, width, height);

	auto* engine = static_cast<Pengu::Core::PenguEngine*>(glfwGetWindowUserPointer(window));
	if (engine) {
		engine->onResize(width, height);
	}

}

void Window::window_iconify_callback(GLFWwindow* window, int iconified)
{
	if (iconified) {
		if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
			glfwWaitEvents();
		}
	}
	else {
		// Restored
	}
}

