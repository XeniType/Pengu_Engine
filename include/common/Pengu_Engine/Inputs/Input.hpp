/**
 * @file Input.hpp
 * @brief Input management system for handling keyboard and mouse events.
 */

#ifndef INPUT_HPP
#define INPUT_HPP 1

#include "Pengu_Engine/Inputs/Actions.hpp"
#include <optional>
#include <unordered_map>
#include <map>
#include "GLFW/glfw3.h"

 /**
	* @enum Action
	* @brief Abstraction layer mapping physical key codes to engine actions.
	* 
	* This allows the rest of the engine to query for "Jump" instead of
	* a specific hardware key like "Space".
	*/
enum class Action {
	Up = DefaultActions::Up,
	Down = DefaultActions::Down,
	Left = DefaultActions::Left,
	Right = DefaultActions::Right,
	Jump = DefaultActions::Jump,
	Enter = DefaultActions::Enter,
	BackSpace = DefaultActions::BackSpace,
	Escape = DefaultActions::Escape,

	Shift = DefaultActions::Shift,

	Q = DefaultActions::Q,
	W = DefaultActions::W,
	E = DefaultActions::E,
	R = DefaultActions::R,
	T = DefaultActions::T,
	Y = DefaultActions::Y,
	U = DefaultActions::U,
	I = DefaultActions::I,
	O = DefaultActions::O,
	P = DefaultActions::P,
	A = DefaultActions::A,
	S = DefaultActions::S,
	D = DefaultActions::D,
	F = DefaultActions::F,
	G = DefaultActions::G,
	H = DefaultActions::H,
	J = DefaultActions::J,
	K = DefaultActions::K,
	L = DefaultActions::L,
	Z = DefaultActions::Z,
	X = DefaultActions::X,
	C = DefaultActions::C,
	V = DefaultActions::V,
	B = DefaultActions::B,
	N = DefaultActions::N,
	M = DefaultActions::M,

	Arrow_Up = DefaultActions::Arrow_Up,
	Arrow_Down = DefaultActions::Arrow_Down,
	Arrow_Left = DefaultActions::Arrow_Left,
	Arrow_Right = DefaultActions::Arrow_Right,

};

/**
 * @struct EnumPair
 * @brief Helper for mapping action names to enum values.
 */
struct EnumPair {
	const char* name; ///< Human-readable name.
	Action value;      ///< Enum value.
};

/**
 * @brief Static mapping table for actions.
 */
static const EnumPair action_enum[] = {
		{"Up", Action::Up},
		{"Down", Action::Down},
		{"Left", Action::Left},
		{"Right", Action::Right},
		{"Jump", Action::Jump},
		{"Enter", Action::Enter},
		{"BackSpace", Action::BackSpace},
		{"Escape", Action::Escape},
		{"Shift", Action::Shift},
		{"Q", Action::Q},
		{"W", Action::W},
		{"E", Action::E},
		{"R", Action::R},
		{"T", Action::T},
		{"Y", Action::Y},
		{"U", Action::U},
		{"I", Action::I},
		{"O", Action::O},
		{"P", Action::P},
		{"A", Action::A},
		{"S", Action::S},
		{"D", Action::D},
		{"F", Action::F},
		{"G", Action::G},
		{"H", Action::H},
		{"J", Action::J},
		{"K", Action::K},
		{"L", Action::L},
		{"Z", Action::Z},
		{"X", Action::X},
		{"C", Action::C},
		{"V", Action::V},
		{"B", Action::B},
		{"N", Action::N},
		{"M", Action::M},
		{"Arrow_Up", Action::Arrow_Up},
		{"Arrow_Down", Action::Arrow_Down},
		{"Arrow_Left", Action::Arrow_Left},
		{"Arrow_Right", Action::Arrow_Right}
};

/**
 * @class Input
 * @brief Handles all window-related input events and state tracking.
 * 
 * Manages key states (Pressed, Down, Released), mouse movement,
 * and scrolling. It uses a static dispatch pattern to bridge GLFW's C-style
 * callbacks with C++ member functions.
 */
class Input {
public:
	/**
	 * @brief Constructs an input listener for a specific window.
	 * @param w Pointer to the GLFW window to hook.
	 */
	Input(GLFWwindow* w);

	/// @name Deleted/Move Semantics
	/// @{
	Input(const Input& rvalue) = delete;
	Input& operator = (const Input& rvalue) = delete;

	/**
	 * @brief Move constructor.
	 */
	Input(Input&& rvalue) noexcept : window_(rvalue.window_)
	{
		rvalue.window_ = nullptr;
	};

	/**
	 * @brief Move assignment operator.
	 */
	Input& operator = (Input&& rvalue) noexcept 
	{
		if (this != &rvalue)
		{
			window_ = rvalue.window_;
			rvalue.window_ = nullptr;
		}

		return *this;
	};
	/// @}

	/** @brief Cleans up callbacks and unregisters the window. */
	~Input();

	/** @name Static Dispatch Callbacks
		 * Internal static methods that route C-style GLFW events to the correct Input instance.
		 * @{ */
	static void static_keyboardCallback(GLFWwindow* w, int key, int scancode, int action, int mods);
	static void static_mouseButtonCallback(GLFWwindow* w, int button, int action, int mods);
	static void static_cursorPosCallback(GLFWwindow* w, double xpos, double ypos);
	static void static_mouseScrollCallback(GLFWwindow* w, double xoff, double yoff);
	/** @} */

	/** @name Member Event Handlers
		 * Logic-specific handlers for processing raw hardware events.
		 * @{ */
	void keyboardCallback(int key, int scancode, int action, int mods);
	void mouseButtonCallback(int button, int action, int mods);
	void cursorPosCallback(double xpos, double ypos);
	void mouseScrollCallback(double xoff, double yoff);
	/** @} */

	/**
	 * @brief Resets per-frame input states (Pressed/Released flags and Deltas).
	 * @note Should be called at the end of every engine frame.
	 */
	void update();

	/** @name Keyboard Accessors
	 * @{
	 */
	 /** @brief True only on the frame the action was started. */
	bool isPressed(Action action) const;
	/** @brief True only on the frame the action was stopped. */
	bool isReleased(Action action) const;
	/** @brief True as long as the action key is held. */
	bool isDown(Action action) const;
	/** @} */

	/** @name Mouse Button Accessors
		 * @{ */
	bool isMousePressed(int action) const;
	bool isMouseReleased(int action) const;
	bool isMouseDown(int action) const;
	/** @} */

	/** @name Cursor/Scroll Data
		 * @{ */
	double mouseXPos() const;     ///< Absolute X position of cursor.
	double mouseYPos() const;     ///< Absolute Y position of cursor.
	double mouseDeltaX() const;   ///< Change in X since last frame.
	double mouseDeltaY() const;   ///< Change in Y since last frame.	
	double scrollXOffset() const; ///< Current horizontal scroll delta.
	double scrollYOffset() const; ///< Current vertical scroll delta.
	/** @} */

private:

	GLFWwindow* window_; ///< Pointer to the associated GLFW window.

};

#endif // !__INPUT_HPP__ 1
