conan = {}
configs = {
    { name = "Debug",          conan_conf = "debug_x86_64" },
    { name = "Release",        conan_conf = "release_x86_64" },
    { name = "RelWithDebInfo", conan_conf = "relwithdebinfo_x86_64" },
}

-- Load all Conan generated files for each configuration
for _, cfg in ipairs(configs) do
    include("build/deps/" .. cfg.name .. "/conandeps.premake5.lua")
end

-- Apply Conan config for executables
function conan_config_exec()
    for _, cfg in ipairs(configs) do
        filter("configurations:" .. cfg.name)
            conan_setup(cfg.conan_conf)
        filter {}
    end
end

-- Apply Conan config for static libs (build settings only, no linking)
function conan_config_lib()
    for _, cfg in ipairs(configs) do
        filter("configurations:" .. cfg.name)
            conan_setup_build(cfg.conan_conf)
        filter {}
    end
end

workspace "GameEngine"
    configurations { "Debug", "Release", "RelWithDebInfo" }
    architecture "x64"
    location "build"
    cppdialect "C++20"
    startproject "Window"

    filter "configurations:Debug"
        defines { "DEBUG" }
        buildoptions{"/utf-8"}
        symbols "On"
        runtime "Debug"

    filter "configurations:Release"
        defines { "NDEBUG" }
        buildoptions{"/utf-8"}
        optimize "On"
        runtime "Release"

    filter "configurations:RelWithDebInfo"
        defines { "NDEBUG" }
        buildoptions{"/utf-8"}
        optimize "On"
        runtime "Release"
        symbols "On"

filter {}

function example_project(name, source)
    project(name)
        kind "ConsoleApp"
        language "C++"
        targetdir("build/%{prj.name}/%{cfg.buildcfg}")
        includedirs { "include/common/" }
        libdirs{ "lib/win64/%{cfg.buildcfg}" }
        links { "Pengu_Engine" }
        debugargs { "%{wks.location}/../examples/data" }
        conan_config_exec()
        files { source }
end

example_project("Lighting", "examples/light.cpp")
example_project("Scripting", "examples/scripting.cpp")
