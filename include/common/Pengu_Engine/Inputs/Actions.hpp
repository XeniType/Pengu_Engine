/**
 * @file Actions.hpp
 * @brief Default key bindings using GLFW key codes.
 */

#ifndef ACTIONS_HPP
#define ACTIONS_HPP 1

#include "GLFW/glfw3.h"

/**
 * @enum DefaultActions
 * @brief Maps engine-friendly action names to literal GLFW key constants.
 */
enum class DefaultActions {

	/** Common Keys **/

	Up = GLFW_KEY_W,
	Down = GLFW_KEY_S,
	Left = GLFW_KEY_A,
	Right = GLFW_KEY_D,
	Jump = GLFW_KEY_SPACE,
	Enter = GLFW_KEY_ENTER,
	BackSpace = GLFW_KEY_BACKSPACE,
	Escape = GLFW_KEY_ESCAPE,
	Shift = GLFW_KEY_LEFT_SHIFT,

	/** Keyboard Keys **/

	Q = GLFW_KEY_Q,
	W = GLFW_KEY_W,
	E = GLFW_KEY_E,
	R = GLFW_KEY_R,
	T = GLFW_KEY_T,
	Y = GLFW_KEY_Y,
	U = GLFW_KEY_U,
	I = GLFW_KEY_I,
	O = GLFW_KEY_O,
	P = GLFW_KEY_P,
	A = GLFW_KEY_A,
	S = GLFW_KEY_S,
	D = GLFW_KEY_D,
	F = GLFW_KEY_F,
	G = GLFW_KEY_G,
	H = GLFW_KEY_H,
	J = GLFW_KEY_J,
	K = GLFW_KEY_K,
	L = GLFW_KEY_L,
	Z = GLFW_KEY_Z,
	X = GLFW_KEY_X,
	C = GLFW_KEY_C,
	V = GLFW_KEY_V,
	B = GLFW_KEY_B,
	N = GLFW_KEY_N,
	M = GLFW_KEY_M,

	/** Arrow Keys **/

	Arrow_Up = GLFW_KEY_UP,
	Arrow_Down = GLFW_KEY_DOWN,
	Arrow_Right = GLFW_KEY_RIGHT,
	Arrow_Left = GLFW_KEY_LEFT,

};

#endif // !ACTIONS_HPP
