markdown
# How to Package Your C++ Game Engine as a Distributable Library
### Using Premake | Targeting Windows & Linux

---

## Overview

By the end of this tutorial, your engine will be packaged like this:

```
MyEngine/
├── include/          ← public headers only
├── lib/
│   ├── windows-x64/
│   │   ├── MyEngine.lib   (static)
│   │   └── MyEngine.dll   (dynamic)
│   └── linux-x64/
│       ├── libMyEngine.a  (static)
│       └── libMyEngine.so (dynamic)
├── examples/
│   └── hello_world/
├── LICENSE
└── README.md
```

---

## Step 1: Separate Your Public API from Internal Code

Before building anything, you need to decide what users of your library will be able to *see and call*.

**1.1 — Create an `include/` folder at your project root:**
```
MyEngine/
├── include/
│   └── MyEngine/
│       ├── Engine.h
│       ├── Window.h
│       └── Renderer.h
└── src/
    ├── Engine.cpp
    ├── Window.cpp
    └── Renderer.cpp
```

**1.2 — Only put public-facing headers in `include/`.** Internal headers that users should never touch stay in `src/`.

**1.3 — Add an export macro to your headers.** This is required to expose symbols from a DLL on Windows:

```cpp
// include/MyEngine/API.h

#pragma once

#ifdef _WIN32
  #ifdef MYENGINE_BUILD_DLL
    #define MYENGINE_API __declspec(dllexport)
  #else
    #define MYENGINE_API __declspec(dllimport)
  #endif
#else
  #define MYENGINE_API __attribute__((visibility("default")))
#endif
```

**1.4 — Apply the macro to every public class and function:**

```cpp
// include/MyEngine/Engine.h

#pragma once
#include "API.h"

namespace MyEngine {
    class MYENGINE_API Engine {
    public:
        void Init();
        void Run();
        void Shutdown();
    };
}
```

---

## Step 2: Write Your Premake5 Build Script

**2.1 — Make sure you have Premake5 installed.**
- Download from: https://premake.github.io/download
- Place `premake5` (or `premake5.exe`) somewhere on your PATH, or in your project root.

**2.2 — Create `premake5.lua` in your project root:**

```lua
-- premake5.lua

workspace "MyEngine"
    architecture "x86_64"
    configurations { "Debug", "Release" }
    platforms { "Static", "Shared" }

-- ============================================================
-- The Engine Library
-- ============================================================
project "MyEngine"
    location "build"
    language "C++"
    cppdialect "C++17"

    -- Output directories
    targetdir ("dist/lib/%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}")
    objdir    ("build/obj/%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}")

    -- Source files
    files {
        "include/**.h",
        "src/**.h",
        "src/**.cpp"
    }

    -- Public include path
    includedirs { "include" }

    -- Platform: Static Library
    filter "platforms:Static"
        kind "StaticLib"

    -- Platform: Shared/Dynamic Library
    filter "platforms:Shared"
        kind "SharedLib"
        defines { "MYENGINE_BUILD_DLL" }

    -- Windows settings
    filter "system:windows"
        systemversion "latest"
        defines { "MYENGINE_PLATFORM_WINDOWS" }

    -- Linux settings
    filter "system:linux"
        defines { "MYENGINE_PLATFORM_LINUX" }
        buildoptions { "-fvisibility=hidden" }  -- hides symbols by default
        linkoptions  { "-Wl,-soname,libMyEngine.so" }

    -- Debug config
    filter "configurations:Debug"
        defines { "MYENGINE_DEBUG" }
        symbols "On"
        optimize "Off"
        targetsuffix "-d"   -- e.g. MyEngine-d.lib for debug builds

    -- Release config
    filter "configurations:Release"
        defines { "MYENGINE_RELEASE", "NDEBUG" }
        optimize "Speed"
        symbols "Off"

-- ============================================================
-- Example / Test App (optional, helps you verify it works)
-- ============================================================
project "HelloWorld"
    location "build"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    targetdir "dist/examples/hello_world"
    objdir    "build/obj/examples"

    files { "examples/hello_world/**.cpp" }

    includedirs { "include" }

    -- Link against the static build of the engine
    filter "platforms:Static"
        links { "MyEngine" }

    filter "system:windows"
        libdirs { "dist/lib/windows-x86_64/%{cfg.buildcfg}" }

    filter "system:linux"
        libdirs { "dist/lib/linux-x86_64/%{cfg.buildcfg}" }
```

---

## Step 3: Build the Library

### On Windows

**3.1 — Generate Visual Studio project files:**
```bash
premake5 vs2022
```

**3.2 — Build from the command line (or open the .sln in Visual Studio):**
```bash
# Static Release build
msbuild build/MyEngine.sln /p:Configuration=Release /p:Platform=Static

# Shared (DLL) Release build
msbuild build/MyEngine.sln /p:Configuration=Release /p:Platform=Shared
```

Your output will appear in:
```
dist/lib/windows-x86_64/Release/MyEngine.lib
dist/lib/windows-x86_64/Release/MyEngine.dll
```

---

### On Linux

**3.1 — Generate GNU Makefiles:**
```bash
premake5 gmake2
```

**3.2 — Build:**
```bash
# Static Release build
make config=release_static MyEngine

# Shared (.so) Release build
make config=release_shared MyEngine
```

Your output will appear in:
```
dist/lib/linux-x86_64/Release/libMyEngine.a
dist/lib/linux-x86_64/Release/libMyEngine.so
```

---

## Step 4: Write a Simple Example Project

This helps you verify the library works — and gives users something to copy from.

**4.1 — Create `examples/hello_world/main.cpp`:**

```cpp
#include <MyEngine/Engine.h>
#include 

int main() {
    MyEngine::Engine engine;
    engine.Init();
    std::cout << "Engine initialized successfully!" << std::endl;
    engine.Run();
    engine.Shutdown();
    return 0;
}
```

**4.2 — Also include a simple `premake5.lua` inside the example folder** so users can build it standalone:

```lua
-- examples/hello_world/premake5.lua

workspace "HelloWorld"
    architecture "x86_64"
    configurations { "Release" }

project "HelloWorld"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    files { "main.cpp" }
    includedirs { "../../include" }

    filter "system:windows"
        libdirs { "../../lib/windows-x64" }
        links   { "MyEngine" }

    filter "system:linux"
        libdirs { "../../lib/linux-x64" }
        links   { "MyEngine" }
        linkoptions { "-Wl,-rpath,." }  -- finds .so in same dir at runtime
```

---

## Step 5: Assemble the Distribution Package

**5.1 — Create the final folder structure users will receive:**

```bash
mkdir -p MyEngine-v1.0/{include,lib/windows-x64,lib/linux-x64,examples/hello_world}
```

**5.2 — Copy the files in:**

```bash
# Headers
cp -r include/MyEngine  MyEngine-v1.0/include/

# Windows binaries
cp dist/lib/windows-x86_64/Release/MyEngine.lib  MyEngine-v1.0/lib/windows-x64/
cp dist/lib/windows-x86_64/Release/MyEngine.dll  MyEngine-v1.0/lib/windows-x64/

# Linux binaries
cp dist/lib/linux-x86_64/Release/libMyEngine.a   MyEngine-v1.0/lib/linux-x64/
cp dist/lib/linux-x86_64/Release/libMyEngine.so  MyEngine-v1.0/lib/linux-x64/

# Example
cp -r examples/hello_world  MyEngine-v1.0/examples/

# Docs
cp README.md LICENSE  MyEngine-v1.0/
```

**5.3 — Zip it up:**

```bash
# Windows (PowerShell)
Compress-Archive -Path MyEngine-v1.0 -DestinationPath MyEngine-v1.0-win-linux.zip

# Linux
zip -r MyEngine-v1.0-win-linux.zip MyEngine-v1.0/
```

---

## Step 6: Write a README for Your Users

Your README should answer these four questions immediately:

```markdown
# MyEngine v1.0

A [brief description] game engine.

## Requirements
- C++17 compiler
- Premake5 (for building examples)

## Quick Start

### Windows
1. Copy `lib/windows-x64/MyEngine.dll` next to your executable
2. Link against `lib/windows-x64/MyEngine.lib`
3. Add `include/` to your include path

### Linux
1. Link against `lib/linux-x64/libMyEngine.a` (static)
   or `libMyEngine.so` (dynamic)
2. Add `include/` to your include path

## Example
See `examples/hello_world/` for a full working project.

## Dependencies
- [List any third-party libs you depend on, e.g. GLFW 3.3, OpenGL]
```

---

## Step 7: Checklist Before Sending

Run through this before sharing:

- [ ] All public classes/functions have `MYENGINE_API` applied
- [ ] `include/` only contains headers users need (no internal headers)
- [ ] Both Debug and Release builds are present (or at minimum Release)
- [ ] The `hello_world` example compiles and runs cleanly on a **fresh machine**
- [ ] Your README lists all dependencies
- [ ] A LICENSE file is included
- [ ] The `.dll` / `.so` is included alongside the `.lib` / `.a` (users need both at runtime for shared builds)

---

## Common Pitfalls

| Problem | Cause | Fix |
|---|---|---|
| `__declspec` errors on Linux | Missing platform guard | Wrap in `#ifdef _WIN32` |
| Linker can't find symbols | Forgot `MYENGINE_API` on a class | Add macro to the class declaration |
| `.dll` not found at runtime | DLL not next to the `.exe` | Copy `.dll` into the same folder as the user's executable |
| `.so` not found at runtime | Library path not set | Use `-rpath` linker flag or `export LD_LIBRARY_PATH=.` |
| Missing symbols in static lib | Build used wrong config | Rebuild with `platforms:Static` explicitly |

---

*That's it! You now have a fully packaged, distributable C++ game engine library.*