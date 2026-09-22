#include "Pengu_Engine/Systems/ScriptSystem.hpp"
#include "Pengu_Engine/Wrappers/LuaWrapper.hpp" 
#include "Pengu_Engine/Misc/Logmacros.hpp"
#include "Pengu_Engine/Inputs/Input.hpp"

#include "Pengu_Engine/Components/TransformComponent.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "sol/sol.hpp"

Scripting::Scripting(ECSManager& ecs) : m_world(ecs)
{
	globalLuaState_ = nullptr;
}

Scripting::~Scripting()
{
	Shutdown();
}

void Scripting::Shutdown()
{
	for (auto& entity : entities_) {
		auto& sc = m_world.GetComponent<ScriptingComponent>(entity);
		sc.onStart = sol::nil;
		sc.onUpdate = sol::nil;
		sc.onFinish = sol::nil;
		sc.instance = sol::nil;
		sc.env_ = sol::nil;
	}

	scriptCache_.clear();
	entities_.clear();
}

void Scripting::Init(Input* inp)
{
	globalLuaState_ = std::make_unique<sol::state>();

	globalLuaState_->open_libraries(sol::lib::base, sol::lib::math);

	globalLuaState_->set_exception_handler(
		[](lua_State*, sol::optional<const std::exception&> ex, sol::string_view desc) -> int {
			if (ex) {
				SCRP_ERROR("[Lua Exception] {0}", ex->what());
			}
			else {
				SCRP_ERROR("[Lua Panic] {0}", std::string(desc));
			}
			return 0;
		}
	);

	BindAPI(inp);
}

void Scripting::Update(double dt)
{
	for (auto& entity : entitiesToReload_) {
		if (entities_.find(entity) != entities_.end()) {
			auto& sc = m_world.GetComponent<ScriptingComponent>(entity);
			ReloadScript(entity, sc);
		}
	}

	entitiesToReload_.clear();

	for (auto& entity : entities_) {

		auto& sc = m_world.GetComponent<ScriptingComponent>(entity);

		if (!sc.env_) {
			LoadScript(entity, sc);
		}

		auto currentWriteTime = std::filesystem::last_write_time(sc.scriptPath_);
		if (currentWriteTime != sc.lastWriteTime) {
			entitiesToReload_.insert(entity);
			sc.lastWriteTime = currentWriteTime;
			continue;
		}

		if (sc.onUpdate.valid()) {
			LuaEntity luaEntity(entity, m_world);
			SafeCall(sc.onUpdate, sc.instance, luaEntity, dt);
		}
	}
}

void Scripting::Start()
{
	for (auto& entity : entities_) {

		auto& sc = m_world.GetComponent<ScriptingComponent>(entity);

		if (!sc.env_) {
			LoadScript(entity, sc);
		}

		auto currentWriteTime = std::filesystem::last_write_time(sc.scriptPath_);
		if (currentWriteTime != sc.lastWriteTime) {
			entitiesToReload_.insert(entity);
			sc.lastWriteTime = currentWriteTime;
			continue;
		}

		if (sc.onStart.valid()) {
			LuaEntity luaEntity(entity, m_world);
			SafeCall(sc.onStart, sc.instance, luaEntity);
		}
	}

}

void Scripting::Finish()
{

	for (auto& entity : entities_) {

		auto& sc = m_world.GetComponent<ScriptingComponent>(entity);

		if (!sc.env_) {
			LoadScript(entity, sc);
		}

		auto currentWriteTime = std::filesystem::last_write_time(sc.scriptPath_);
		if (currentWriteTime != sc.lastWriteTime) {
			entitiesToReload_.insert(entity);
			sc.lastWriteTime = currentWriteTime;
			continue;
		}

		if (sc.onFinish.valid()) {
			LuaEntity luaEntity(entity, m_world);
			SafeCall(sc.onFinish, sc.instance, luaEntity);
		}
	}

}

void Scripting::BindAPI(Input* inp)
{
	SCRP_INFO("sol2 version: {}", SOL_VERSION);
	bindGLM();

	bindEngine();

	bindLogger();

	sol::table input_table = globalLuaState_->create_named_table("Action");
	for (auto& e : action_enum) input_table[e.name] = e.value;

	globalLuaState_->set_function("IsKeyDown", [inp](int key) {
		return inp->isDown(static_cast<Action>(key));  // Or however you access your Input singleton
		});
	globalLuaState_->set_function("IsKeyPressed", [inp](int key) {
		return inp->isPressed(static_cast<Action>(key));
		});
}

void Scripting::bindGLM()
{
	/** VECTORS **/
	// vec2
	globalLuaState_->new_usertype<glm::vec2>("vec2",
		sol::call_constructor,
		sol::factories(
			[]() { return glm::vec2(0, 0); },
			[](float x, float y) { return glm::vec2(x, y); },
			[](float s) { return glm::vec2(s, s); }
		),
		"x", &glm::vec2::x,
		"y", &glm::vec2::y,
		// operators
		sol::meta_function::addition, sol::overload(
			[](const glm::vec2& a, const glm::vec2& b) { return a + b; },
			[](const glm::vec3& a, float s) { return glm::vec2(a.x + s, a.y + s); },
			[](float s, const glm::vec2& a) { return glm::vec2(s + a.x, s + a.y); }
		),
		sol::meta_function::subtraction, sol::overload(
			[](const glm::vec2& a, const glm::vec2& b) { return a - b; },
			[](const glm::vec3& a, float s) { return glm::vec2(a.x - s, a.y - s); },
			[](float s, const glm::vec2& a) { return glm::vec2(s - a.x, s - a.y); }
		),
		sol::meta_function::multiplication, sol::overload(
			[](const glm::vec2& a, const glm::vec2& b) { return a * b; },
			[](const glm::vec2& a, float s) { return a * s; },
			[](float s, const glm::vec2& a) { return s * a; }
		),
		sol::meta_function::unary_minus, [](const glm::vec2& a) { return -a; },
		sol::meta_function::to_string, [](const glm::vec2& a) {
			return "vec2(" + std::to_string(a.x) + ", " + std::to_string(a.y) + ")";
		}
	);
	// vec3 (you already have this, just add operators)
	globalLuaState_->new_usertype<glm::vec3>("vec3",
		sol::call_constructor,
		sol::factories(
			[]() { return glm::vec3(0, 0, 0); },
			[](float x, float y, float z) { return glm::vec3(x, y, z); },
			[](float s) { return glm::vec3(s, s, s); }
		),
		"x", &glm::vec3::x,
		"y", &glm::vec3::y,
		"z", &glm::vec3::z,
		// operators
		sol::meta_function::addition, sol::overload(
			[](const glm::vec3& a, const glm::vec3& b) { return a + b; },
			[](const glm::vec3& a, float s) { return glm::vec3(a.x + s, a.y + s, a.z + s); },
			[](float s, const glm::vec3& a) { return glm::vec3(s + a.x, s + a.y, s + a.z); }
		),
		sol::meta_function::subtraction, sol::overload(
			[](const glm::vec3& a, const glm::vec3& b) { return a - b; },
			[](const glm::vec3& a, float s) { return glm::vec3(a.x - s, a.y - s, a.z - s); },
			[](float s, const glm::vec3& a) { return glm::vec3(s - a.x, s - a.y, s - a.z); }
		),
		sol::meta_function::multiplication, sol::overload(
			[](const glm::vec3& a, const glm::vec3& b) { return a * b; },
			[](const glm::vec3& a, float s) { return a * s; },
			[](float s, const glm::vec3& a) { return s * a; }
		),
		sol::meta_function::unary_minus, [](const glm::vec3& a) { return -a; },
		sol::meta_function::to_string, [](const glm::vec3& a) {
			return "vec3(" + std::to_string(a.x) + ", " +
				std::to_string(a.y) + ", " +
				std::to_string(a.z) + ")";
		}
	);

	// vec4
	globalLuaState_->new_usertype<glm::vec4>("vec4",
		sol::call_constructor,
		sol::factories(
			[]() { return glm::vec4(0, 0, 0, 0); },
			[](float x, float y, float z, float w) { return glm::vec4(x, y, z, w); },
			[](float s) { return glm::vec4(s, s, s, s); }
		),
		"x", &glm::vec4::x,
		"y", &glm::vec4::y,
		"z", &glm::vec4::z,
		"w", &glm::vec4::w,
		// operators
		sol::meta_function::addition, sol::overload(
			[](const glm::vec4& a, const glm::vec4& b) { return a + b; },
			[](const glm::vec4& a, float s) { return glm::vec4(a.x + s, a.y + s, a.z + s, a.w + s); },
			[](float s, const glm::vec4& a) { return glm::vec4(s + a.x, s + a.y, s + a.z, s + a.w); }
		),
		sol::meta_function::subtraction, sol::overload(
			[](const glm::vec4& a, const glm::vec4& b) { return a - b; },
			[](const glm::vec4& a, float s) { return glm::vec4(a.x - s, a.y - s, a.z - s, a.w - s); },
			[](float s, const glm::vec4& a) { return glm::vec4(s - a.x, s - a.y, s - a.z, s - a.w); }
		),
		sol::meta_function::multiplication, sol::overload(
			[](const glm::vec4& a, const glm::vec4& b) { return a * b; },
			[](const glm::vec4& a, float s) { return a * s; },
			[](float s, const glm::vec4& a) { return s * a; }
		),
		sol::meta_function::unary_minus, [](const glm::vec4& a) { return -a; }
	);

	/** QUATERNION **/
	globalLuaState_->new_usertype<glm::quat>("quat",
		sol::constructors<glm::quat(), glm::quat(float, float, float, float)>(),
		"x", &glm::quat::x,
		"y", &glm::quat::y,
		"z", &glm::quat::z,
		"w", &glm::quat::w,
		sol::meta_function::multiplication, sol::overload(
			[](const glm::quat& a, const glm::quat& b) { return a * b; },
			[](const glm::quat& a, const glm::vec3& v) { return a * v; }
		)
	);

	/** Mathematics **/
	sol::table glm_table = globalLuaState_->create_named_table("glm");

	// common
	glm_table["length"] = sol::overload(
		[](const glm::vec2& v) { return glm::length(v); },
		[](const glm::vec3& v) { return glm::length(v); },
		[](const glm::vec4& v) { return glm::length(v); }
	);
	glm_table["normalize"] = sol::overload(
		[](const glm::vec2& v) { return glm::normalize(v); },
		[](const glm::vec3& v) { return glm::normalize(v); },
		[](const glm::vec4& v) { return glm::normalize(v); }
	);
	glm_table["dot"] = sol::overload(
		[](const glm::vec2& a, const glm::vec2& b) { return glm::dot(a, b); },
		[](const glm::vec3& a, const glm::vec3& b) { return glm::dot(a, b); }
	);
	glm_table["cross"] = [](const glm::vec3& a, const glm::vec3& b) { return glm::cross(a, b); };
	glm_table["distance"] = sol::overload(
		[](const glm::vec2& a, const glm::vec2& b) { return glm::distance(a, b); },
		[](const glm::vec3& a, const glm::vec3& b) { return glm::distance(a, b); }
	);
	glm_table["lerp"] = sol::overload(
		[](const glm::vec2& a, const glm::vec2& b, float t) { return glm::mix(a, b, t); },
		[](const glm::vec3& a, const glm::vec3& b, float t) { return glm::mix(a, b, t); },
		[](float a, float b, float t) { return glm::mix(a, b, t); }
	);
	glm_table["clamp"] = sol::overload(
		[](float v, float lo, float hi) { return glm::clamp(v, lo, hi); },
		[](const glm::vec3& v, float lo, float hi) { return glm::clamp(v, lo, hi); }
	);
	glm_table["reflect"] = [](const glm::vec3& i, const glm::vec3& n) { return glm::reflect(i, n); };
	glm_table["radians"] = [](float deg) { return glm::radians(deg); };
	glm_table["degrees"] = [](float rad) { return glm::degrees(rad); };

	// quaternion helpers
	glm_table["angleAxis"] = [](float angle, const glm::vec3& axis) { return glm::angleAxis(angle, axis); };
	glm_table["eulerAngles"] = [](const glm::quat& q) { return glm::eulerAngles(q); };
	glm_table["quatFromEuler"] = [](const glm::vec3& euler) { return glm::quat(euler); };
	glm_table["slerpQuat"] = [](const glm::quat& a, const glm::quat& b, float t) { return glm::slerp(a, b, t); };
}

void Scripting::bindEngine()
{
	globalLuaState_->new_usertype<TransformComponent>(
		"Transform",
		sol::no_constructor,
		"Position", &TransformComponent::position_,
		"Scale", &TransformComponent::scale_,
		"Rotation", &TransformComponent::rotation_
	);


	globalLuaState_->new_usertype<LuaEntity>(
		"Entity",

		"getTransform", [](LuaEntity& self) -> TransformComponent*
		{
			try {
				return &self.GetComponent<TransformComponent>();
			}
			catch (std::runtime_error& e) {
				ENGINE_WARNING(e.what());
				return  nullptr;
			}
		},

		"setTransform", [](LuaEntity& self, const TransformComponent& trans)
		{
			try {
				self.GetComponent<TransformComponent>() = trans;
			}
			catch (std::runtime_error& e) {
				ENGINE_WARNING(e.what());
			}
		},

		"getPosition", [](LuaEntity& self) -> glm::vec3&
		{
			return self.GetComponent<TransformComponent>().position_;
		},

		"setPosition", [](LuaEntity& self, const glm::vec3& pos)
		{
			self.GetComponent<TransformComponent>().position_ = pos;
		},

		"getRotation", [](LuaEntity& self) -> glm::vec3&
		{
			return self.GetComponent<TransformComponent>().rotation_;
		},

		"setRotation", [](LuaEntity& self, const glm::vec3& rot)
		{
			self.GetComponent<TransformComponent>().rotation_ = rot;
		},

		"getScale", [](LuaEntity& self) -> glm::vec3&
		{
			return self.GetComponent<TransformComponent>().scale_;
		},

		"setScale", [](LuaEntity& self, const glm::vec3& scl)
		{
			self.GetComponent<TransformComponent>().scale_ = scl;
		}
	);
}

void Scripting::bindLogger()
{
	sol::table log_table = globalLuaState_->create_named_table("Log");

	log_table["trace"] = [](const std::string& msg) {
		Logger::SCRIPT()->trace(msg);
		};
	log_table["info"] = [](const std::string& msg) {
		Logger::SCRIPT()->info(msg);
		};
	log_table["warn"] = [](const std::string& msg) {
		Logger::SCRIPT()->warn(msg);
		};
	log_table["error"] = [](const std::string& msg) {
		Logger::SCRIPT()->error(msg);
		};
}

void Scripting::LoadScript(Entity entity, ScriptingComponent& sc)
{
	sc.env_ = sol::environment{ *globalLuaState_, sol::create, globalLuaState_->globals() };

	sol::load_result loadedScript;
	auto it = scriptCache_.find(sc.scriptPath_);
	if (it != scriptCache_.end()) {
		loadedScript = globalLuaState_->load(it->second.as_string_view());
	}
	else {
		loadedScript = globalLuaState_->load_file(sc.scriptPath_);

		if (!loadedScript.valid()) {
			sol::error err = loadedScript;
			SCRP_ERROR("[Lua Load Error] {0} -> {1}", sc.scriptPath_, err.what());
			sc.env_ = sol::nil;
			return;
		}

		sol::bytecode bc = loadedScript.get<sol::function>().dump();
		scriptCache_[sc.scriptPath_] = std::move(bc);
		loadedScript = globalLuaState_->load(scriptCache_[sc.scriptPath_].as_string_view());
	}

	if (!loadedScript.valid()) {
		sol::error err = loadedScript;
		SCRP_ERROR("[Lua Load Error] {0} -> {1}", sc.scriptPath_, err.what());
		return;
	}

	sol::protected_function scriptFunc = loadedScript;
	sol::set_environment(sc.env_, scriptFunc);
	sol::protected_function_result result = scriptFunc();

	if (!result.valid()) {
		sol::error err = result;
		SCRP_ERROR("[Lua Runtime Error] {0} -> {1}", sc.scriptPath_, err.what());
		return;
	}

	if (result.get_type() != sol::type::table) {
		SCRP_ERROR("[Lua Error] {0} -> script must return a table", sc.scriptPath_);
		return;
	}

	sc.instance = result;

	sc.onStart = sc.instance["onStart"];
	sc.onUpdate = sc.instance["onUpdate"];
	sc.onFinish = sc.instance["onFinish"];

	sc.lastWriteTime = std::filesystem::last_write_time(sc.scriptPath_);

}

void Scripting::ReloadScript(Entity entity, ScriptingComponent& sc)
{

	sol::table state = globalLuaState_->create_table();

	if (sc.instance.valid()) {
		for (auto& kv : sc.instance)
		{
			sol::object value = kv.second;

			if (value.is<int>() ||
				value.is<float>() ||
				value.is<std::string>() ||
				value.is<bool>())
			{
				state[kv.first] = value;
			}
		}
	}

	scriptCache_.erase(sc.scriptPath_);

	sc.env_ = sol::nil;
	sc.instance = sol::nil;
	sc.onStart = sol::nil;
	sc.onUpdate = sol::nil;
	sc.onFinish = sol::nil;

	LoadScript(entity, sc);

	if (sc.instance.valid())
	{
		for (auto& kv : state)
		{
			sc.instance[kv.first] = kv.second;
		}
	}

}
