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
    staticruntime "off"
    startproject "Window"

    -- warnings "Extra"
    -- fatalwarnings { "All" }

    buildoptions{"/utf-8"}

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        runtime "Debug"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        runtime "Release"

    filter "configurations:RelWithDebInfo"
        defines { "NDEBUG" }
        optimize "On"
        runtime "Release"
        symbols "On"

filter {}

function example_project(name, source)
    project(name)
        kind "ConsoleApp"
        language "C++"
        targetdir("build/%{prj.name}/%{cfg.buildcfg}")
        includedirs { "include/common/", "assets/"}
        links { "Pengu_Engine" }
        debugargs { "%{wks.location}/../examples/data" }
        conan_config_exec()
        files { source }
end

project "Pengu_Engine"
    kind "StaticLib"
    targetdir "build/%{cfg.buildcfg}"
    includedirs { "include/common/" , "assets/"}
    conan_config_lib()
    files {
        "premake5.lua",
        "src/common/build/conanfile.txt",

        "src/**.cpp",
        "src/**.h",

        "include/common/Pengu_Engine/**.hpp",
        "src/vendor/**.hpp",

        "assets/**",
    }
    
    example_project("Window",   "examples/window.cpp")
    example_project("Triangle", "examples/triangle.cpp")
    example_project("Inputs",   "examples/input.cpp")
    example_project("Asset_Loader", "examples/asset_loader.cpp")
    example_project("Job_System", "examples/jobsystem.cpp")
    example_project("ECSManager", "examples/ecsmanager.cpp")
    example_project("Scripting", "examples/scripting.cpp")
    example_project("Shadow_Mapping", "examples/shadow_mapping.cpp")
    example_project("Deferred_Rendering", "examples/deferred_rendering.cpp")
    example_project("DEMO", "examples/DEMO.cpp")
