#include "Pengu_Engine/Inputs/Input.hpp"
#include "GLFW/glfw3.h"
#include <optional>
#include <iostream>

static std::unordered_map<GLFWwindow*, Input*> g_windowToInput;

static std::unordered_map<Action, bool> keyDown;
static std::unordered_map<Action, bool> keyPressed;
static std::unordered_map<Action, bool> keyReleased;

static std::map<Action, int> keyBindings_;

static std::unordered_map<int, bool> mouseDown;
static std::unordered_map<int, bool> mousePressed;
static std::unordered_map<int, bool> mouseReleased;

static double mouseX = 0.0;
static double mouseY = 0.0;
static double mouseDX = 0.0;
static double mouseDY = 0.0;

static double scrollX = 0.0;
static double scrollY = 0.0;

// Constructors and Destructors:
Input::Input(GLFWwindow* w) : window_{ w } {
	g_windowToInput[w] = this;
	glfwSetKeyCallback(w, static_keyboardCallback);
	glfwSetMouseButtonCallback(w, static_mouseButtonCallback);
	glfwSetCursorPosCallback(w, static_cursorPosCallback);
	glfwSetScrollCallback(w, static_mouseScrollCallback);

}

Input::~Input() {
	g_windowToInput.erase(window_);
}

/** Static Callbacks **/

void Input::static_keyboardCallback(GLFWwindow* w, int key, int scancode, int action, int mods)
{
	auto it = g_windowToInput.find(w);
	if (it != g_windowToInput.end())
		it->second->keyboardCallback(key, scancode, action, mods);
}

void Input::static_mouseButtonCallback(GLFWwindow* w, int button, int action, int mods)
{
	auto it = g_windowToInput.find(w);
	if (it != g_windowToInput.end())
		it->second->mouseButtonCallback(button, action, mods);
}
void Input::static_cursorPosCallback(GLFWwindow* w, double xpos, double ypos)
{
	auto it = g_windowToInput.find(w);
	if (it != g_windowToInput.end())
		it->second->cursorPosCallback(xpos, ypos);
}
void Input::static_mouseScrollCallback(GLFWwindow* w, double xoff, double yoff)
{
	auto it = g_windowToInput.find(w);
	if (it != g_windowToInput.end())
		it->second->mouseScrollCallback(xoff, yoff);
}

/** Callback Implementation **/

void Input::keyboardCallback(int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {
		keyPressed[(Action)key] = true;
		keyDown[(Action)key] = true;
	}
	else if (action == GLFW_RELEASE)
	{
		keyReleased[(Action)key] = true;
		keyDown[(Action)key] = false;
	}
}

void Input::mouseButtonCallback(int button, int action, int mods)
{

	if (action == GLFW_PRESS) {
		mousePressed[button] = true;
		mouseDown[button] = true;
	}
	else if (action == GLFW_RELEASE) {
		mouseReleased[button] = true;
		mouseDown[button] = false;
	}

}
void Input::cursorPosCallback(double xpos, double ypos)
{
	mouseDX = xpos - mouseX;
	mouseDY = ypos - mouseY;

	mouseX = xpos;
	mouseY = ypos;
}
void Input::mouseScrollCallback(double xoff, double yoff)
{
	scrollX += xoff;
	scrollY += yoff;
}

void Input::update()
{
	keyPressed.clear();
	keyReleased.clear();
	mousePressed.clear();
	mouseReleased.clear();

	mouseDX = 0.0;
	mouseDY = 0.0;

	scrollX = 0.0;
	scrollY = 0.0;
}

/** Key Accessours **/

bool Input::isPressed(Action action) const
{
	return keyPressed.contains(action) && keyPressed.at(action);
}

bool Input::isReleased(Action action) const
{
	return keyReleased.contains(action) && keyReleased.at(action);
}

bool Input::isDown(Action action) const
{
	return keyDown.contains(action) && keyDown.at(action);
}

/** Mouse Button Accessours **/
bool Input::isMousePressed(int action) const
{
	return mousePressed.contains(action) && mousePressed.at(action);
}
bool Input::isMouseReleased(int action) const
{
	return mouseReleased.contains(action) && mouseReleased.at(action);
}
bool Input::isMouseDown(int action) const
{
	return mouseDown.contains(action) && mouseDown.at(action);
}

/** Cursor Position Accessours **/

double Input::mouseXPos() const { return mouseX; };
double Input::mouseYPos() const { return mouseY; };

double Input::mouseDeltaX() const { return mouseDX; };
double Input::mouseDeltaY() const { return mouseDY; };

/** Mouse Scroll Accessours **/

double Input::scrollXOffset() const { return scrollX; };
double Input::scrollYOffset() const { return scrollY; };
