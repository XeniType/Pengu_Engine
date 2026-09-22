/**
 * @file Interpreter.hpp
 * @brief Simple Lua interpreter using sol2.
 */

#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP 1

#include <string>
#include <sol/sol.hpp>

/**
 * @class Interpreter
 * @brief Encapsulates a Lua state and provides methods for script execution.
 */
class Interpreter {
public:
	/**
	 * @brief Constructs the interpreter and initializes the Lua state.
	 */
	Interpreter();

	/**
	 * @brief Destructor.
	 */
	~Interpreter();

	/**
	 * @brief Executes a Lua script from a file.
	 * @param scriptPath Path to the .lua file.
	 */
	void ExecScript(std::string scriptPath);

private:
	sol::state Lua; ///< The internal Lua state managed by sol2.

};

#endif // !INTERPRETER_HPP
