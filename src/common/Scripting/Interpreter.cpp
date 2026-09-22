#include "Pengu_Engine/Scripting/Interpreter.hpp"
#include "Pengu_Engine/Misc/Logmacros.hpp"
#include <stdio.h>
#include <string>

Interpreter::Interpreter()
{

	Lua.open_libraries(sol::lib::base);


}

Interpreter::~Interpreter()
{

}

void Interpreter::ExecScript(std::string scriptPath)
{

	sol::load_result script = Lua.load_file(scriptPath);

	if (!script.valid()) {
		sol::error err = script;
		ENGINE_ERROR("Failed to load script {}: {}", scriptPath, err.what());
		return;
	}

	sol::protected_function func = script;
	sol::protected_function_result result = func();

	if (!result.valid()) {
		sol::error err = result;
		ENGINE_ERROR("Euntime error in {}: {}", scriptPath, err.what());
	}

}
