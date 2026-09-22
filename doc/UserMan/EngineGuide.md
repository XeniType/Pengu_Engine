# Pengu Engine — Beginner's Guide

Welcome to **Pengu Engine**! This guide will walk you through running it, changing render modes, and writing Lua scripts for your entities.

---

## Table of Contents

1. [Starting the Engine](#1-starting-the-engine)
2. [Render Modes & Switching Renderers](#2-render-modes--switching-renderers)
3. [Creating a Scene](#3-creating-a-scene)
4. [Lua Scripting](#4-lua-scripting)
5. [Scripting API Reference](#5-scripting-api-reference)

---

## 1. Starting the Engine

The engine is started from `main.cpp` using a static factory method. There is no manual initialization required — `PenguEngine::startEngine()` handles all subsystem construction internally.

### Step 1: Configure the Engine

Define an `EngineConfig` struct with your desired window settings. All fields have sensible defaults except for the window dimensions and title:

```cpp
#include "Pengu_Engine/PenguEngine.hpp"

using Pengu::Core::PenguEngine;
using Pengu::Core::EngineConfig;

static EngineConfig config {
    .screen_width  = 1280,
    .screen_height = 960,
    .title         = "My Game"
};
```

The full list of configurable fields is:

| Field | Type | Default | Description |
|---|---|---|---|
| `screen_width` | `unsigned int` | — | Window width in pixels |
| `screen_height` | `unsigned int` | — | Window height in pixels |
| `title` | `string` | — | Window title bar text |
| `cam_speed` | `float` | `0.005f` | Camera movement speed |
| `fov` | `float` | `90.0f` | Field of view in degrees |
| `cam_sens` | `float` | `0.1f` | Mouse look sensitivity |

---

### Step 2: Start the Engine

Call `PenguEngine::startEngine()` with your config. This initializes the window, input, renderer, resource manager, and all other subsystems. If anything fails, the engine will log an error and terminate.

```cpp
auto engine = PenguEngine::startEngine(config);
```

---

### Step 3: Load a Scene

Call `engine.loadScene()` with an instance of your scene. The engine automatically initializes the default render pipeline (`ForwardShadPipeline`) at this point.

```cpp
#include "Pengu_Engine/Scene/LightScene.hpp"

engine.loadScene(std::make_unique<Pengu::Scene::LightScene>(&engine));
```

> Pass `&engine` to the scene constructor so the scene has access to engine subsystems such as the camera, input, and renderer.

---

### Step 4: Run the Game Loop

Run the main loop until the window is closed or the Escape key is pressed. Each iteration calls `engine.update()` to process logic and render the frame, followed by `engine.EndFrame()` to swap buffers and poll events.

```cpp
while (!engine.IsClosing() && !engine.GetInput().isDown(Action::Escape))
{
    engine.update();
    engine.EndFrame();
}
```

---

### Full `main.cpp` Example

```cpp
#include "Pengu_Engine/PenguEngine.hpp"
#include "Pengu_Engine/Scene/LightScene.hpp"

using Pengu::Core::PenguEngine;
using Pengu::Core::EngineConfig;

static EngineConfig config {
    .screen_width  = 1280,
    .screen_height = 960,
    .title         = "Pengu_Engine"
};

int main()
{
    auto engine = PenguEngine::startEngine(config);

    engine.loadScene(std::make_unique<Pengu::Scene::LightScene>(&engine));

    while (!engine.IsClosing() && !engine.GetInput().isDown(Action::Escape))
    {
        engine.update();
        engine.EndFrame();
    }
}
```

---

### What `update()` and `EndFrame()` Do

Each frame is split into two calls:

| Call | What it does |
|---|---|
| `engine.update()` | Processes delta time, camera input, scene update, and renders the active scene |
| `engine.EndFrame()` | Renders the UI stat panel, polls window events, and swaps the back buffer |

> **Never call these outside the main loop.** Both calls are required every frame for the engine to function correctly.

---

## 2. Render Modes & Switching Renderers

Pengu Engine uses a **pipeline-based renderer**. Each render mode is a separate `RenderPipeline` that you swap in at runtime by calling `setPipeline()` on the `Renderer`. The old pipeline is automatically cleaned up before the new one is initialized.

### Available Pipelines

| Mode | Class | Description |
|---|---|---|
| **Unlit** | `UnlitPipeline` | Flat colours and textures, no lighting |
| **Forward** | `ForwardPipeline` | Standard lighting pass |
| **ForwardShadows** | `ForwardShadPipeline` | Full lighting with shadow mapping |

---

### How to Switch Pipeline at Runtime

You can use the Scene tab to select what Pipeline you want to use or you can change it in code using the example below.

Call `setPipeline()` on the renderer at any point — for example inside your scene's `update()` on a key press:

```cpp
#include "Pengu_Engine/Graphics/Rendering/Techniques/ForwardShad.hpp"
#include "Pengu_Engine/Graphics/Rendering/Techniques/Forward.hpp"
#include "Pengu_Engine/Graphics/Rendering/Techniques/Unlit.hpp"

void MyScene::update(float dt)
{
    auto& inp      = m_engine->GetInput();
    auto& renderer = m_engine->GetRenderer();

    if (inp.isPressed(Action::F1)) {
        renderer.setPipeline(std::make_unique<UnlitPipeline>());
    }
    if (inp.isPressed(Action::F2)) {
        renderer.setPipeline(std::make_unique<ForwardPipeline>());
    }
    if (inp.isPressed(Action::F3)) {
        renderer.setPipeline(std::make_unique<ForwardShadPipeline>());
    }
}
```

> **Note:** `setPipeline()` automatically cleans up the current pipeline, initializes the new one, and restores the correct viewport size — you don't need to do anything else.

---

### Checking the Current Pipeline

You can check whether a pipeline is active before doing pipeline-specific work:

```cpp
auto& renderer = m_engine->GetRenderer();

if (renderer.hasPipeline()) {
    auto* pipeline = renderer.getPipeline();
    // use pipeline...
}
```

---

### Pipeline Lifecycle (What Happens Internally)

When you call `setPipeline()`, the engine:

1. Calls `cleanup()` on the old pipeline and destroys it
2. Calls `init()` on the new pipeline with the current `ResourceManager`
3. Calls `onResize()` to restore the correct viewport dimensions

You never need to manage this manually — just call `setPipeline()` with a new instance.

---

## 3. Creating a Scene

Scenes in Pengu Engine are C++ classes that inherit from `SceneBase`. Each scene has three lifecycle methods you must implement: `onLoad`, `update`, and `onUnload`.

### Step 1: Create the Header File

Create a new `.hpp` file for your scene. The class must inherit from `SceneBase` and pass the engine pointer to its constructor:

```cpp
#ifndef MYSCENE_HPP
#define MYSCENE_HPP 1
#include "Pengu_Engine/Scene/SceneBase.hpp"
#include "Pengu_Engine/PenguEngine.hpp"

namespace Pengu::Resources { class ResourceManager; }

namespace Pengu::Scene {
    class MyScene : public SceneBase {
    public:
        MyScene(Pengu::Core::PenguEngine* engine) : SceneBase(engine) { m_name = "My_Scene"; }
        void update(float dt) override;
        void onLoad(Pengu::Resources::ResourceManager& rm) override;
        void onUnload() override;
    };
}
#endif
```

> **Important:** Set `m_name` in the constructor — this is how the engine identifies your scene.

---

### Step 2: Implement the Scene

Create the matching `.cpp` file. A scene has three methods to fill in:

| Method | When it runs | What to do here |
|---|---|---|
| `onLoad` | Once when the scene is loaded | Register components, create entities, load assets |
| `update` | Every frame | Handle per-frame logic, input, entity updates |
| `onUnload` | When the scene is unloaded | Clean up any resources if needed |

---

### Step 3: Register Components and Systems

At the start of `onLoad`, register the components your scene will use:

```cpp
void MyScene::onLoad(Pengu::Resources::ResourceManager& rm)
{
    auto& world = getWorld();

    // Register components your scene uses
    world.RegisterComponent<DrawableComponent>();
    world.RegisterComponent<TransformComponent>();
    world.RegisterComponent<Lights>();
    world.RegisterComponent<TagComponent>();

    // Register and configure the light system
    auto lightsys = world.RegisterSystem<LightSystem>();
    {
        Signature signature;
        signature.set(world.GetComponentType<Lights>());
        signature.set(world.GetComponentType<TransformComponent>());
        world.SetSystemSignature<LightSystem>(signature);
    }
    setLightSystem(lightsys);
}
```

---

### Step 4: Load Assets and Create Entities

Use the `ResourceManager` to load meshes and materials, then create entities and attach components:

```cpp
    // Create and configure a material
    auto myMat = rm.getMaterial("My_Material");
    myMat->albedoMap = nullptr;
    myMat->albedoColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // red

    // Create built-in primitives
    auto cube   = rm.CreateCube24v("MyCube", 2.5f, myMat);
    auto plane  = rm.CreatePlane("MyPlane", 100, 100, 1, myMat);
    auto sphere = rm.CreateSphere("MySphere", 1.0f, 50.0f, 50.0f, myMat);
    auto quad   = rm.CreateQuad("MyQuad", 1.0f, myMat);

    // Load an external 3D model
    auto model = rm.loadObject("../resource/object/mymodel/mymodel.obj");

    // Create an entity and attach components
    auto entity = world.CreateEntity();
    world.AddComponent(entity, DrawableComponent(cube));
    world.AddComponent(entity, TransformComponent(
        glm::vec3(0.0f, 0.0f, 0.0f),  // position
        glm::vec3(1.0f),               // scale
        glm::vec3(0.0f)                // rotation
    ));

    // Don't forget to call this at the end of onLoad!
    refreshRenderables();
```

> **Always call `refreshRenderables()` at the end of `onLoad`** — without it, your entities won't be drawn.

---

### Step 5: Add a Light

To add a spotlight to your scene:

```cpp
    auto& camera = m_engine->GetCamera();
    auto& world = getWorld();

    auto light = world.CreateEntity();
    world.AddComponent(light, Lights(LightType::E_Spot));
    world.AddComponent(light, TransformComponent(
        camera.position_,
        glm::vec3(1.0f),
        glm::normalize(camera.front_)
    ));
    world.AddComponent(light, TagComponent("MyLight")); // optional tag to find it later

    world.GetComponent<Lights>(light).color_         = glm::vec3(0.5f, 0.5f, 0.5f);
    world.GetComponent<Lights>(light).radius_        = 50.0f;
    world.GetComponent<Lights>(light).innerCutoff_   = glm::cos(glm::radians(12.0f));
    world.GetComponent<Lights>(light).outerCutoff_   = glm::cos(glm::radians(17.0f));
    world.GetComponent<Lights>(light).shadowSoftness_ = 0.5f;
```

---

### Step 6: Per-Frame Update Logic

Use `update(float dt)` for anything that needs to run every frame, such as moving a light with the camera or spawning entities on input:

```cpp
void MyScene::update(float dt)
{
    auto& world = getWorld();
    auto& inp   = m_engine->GetInput();
    auto& camera = m_engine->GetCamera();

    // Move a tagged light to follow the camera
    auto light = world.GetEntityByTag("MyLight");
    world.GetComponent<TransformComponent>(light).position_ = camera.position_;
    world.GetComponent<TransformComponent>(light).rotation_ = glm::normalize(camera.front_);

    // Spawn a new spotlight on key press
    if (inp.isPressed(Action::C)) {
        auto newLight = world.CreateEntity();
        world.AddComponent(newLight, Lights(LightType::E_Spot));
        world.AddComponent(newLight, TransformComponent(
            camera.position_, glm::vec3(1.0f), glm::normalize(camera.front_)
        ));
        world.GetComponent<Lights>(newLight).radius_ = 50.0f;
    }
}
```

---

### Available Primitives

| Function | Description |
|---|---|
| `rm.CreatePlane(name, width, height, subdivisions, material)` | Flat plane mesh |
| `rm.CreateCube8v(name, size, material)` | Cube with 8 vertices (no smooth normals) |
| `rm.CreateCube24v(name, size, material)` | Cube with 24 vertices (correct normals per face) |
| `rm.CreateQuad(name, size, material)` | Single flat quad |
| `rm.CreateSphere(name, radius, segmentsW, segmentsH, material)` | UV sphere |
| `rm.loadObject(path)` | Load an external `.obj` model |

---

### Full Minimal Scene Example

```cpp
// MyScene.hpp
#ifndef MYSCENE_HPP
#define MYSCENE_HPP 1
#include "Pengu_Engine/Scene/SceneBase.hpp"
#include "Pengu_Engine/PenguEngine.hpp"
namespace Pengu::Resources { class ResourceManager; }
namespace Pengu::Scene {
    class MyScene : public SceneBase {
    public:
        MyScene(Pengu::Core::PenguEngine* engine) : SceneBase(engine) { m_name = "My_Scene"; }
        void update(float dt) override;
        void onLoad(Pengu::Resources::ResourceManager& rm) override;
        void onUnload() override;
    };
}
#endif
```

```cpp
// MyScene.cpp
#include "MyScene.hpp"

namespace Pengu::Scene {

    void MyScene::onLoad(Pengu::Resources::ResourceManager& rm)
    {
        auto& world = getWorld();

        world.RegisterComponent<DrawableComponent>();
        world.RegisterComponent<TransformComponent>();
        world.RegisterComponent<Lights>();
        world.RegisterComponent<TagComponent>();

        auto lightsys = world.RegisterSystem<LightSystem>();
        {
            Signature signature;
            signature.set(world.GetComponentType<Lights>());
            signature.set(world.GetComponentType<TransformComponent>());
            world.SetSystemSignature<LightSystem>(signature);
        }
        setLightSystem(lightsys);

        auto mat = rm.getMaterial("My_Material");
        mat->albedoColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        auto cube = rm.CreateCube24v("Cube", 2.5f, mat);

        auto entity = world.CreateEntity();
        world.AddComponent(entity, DrawableComponent(cube));
        world.AddComponent(entity, TransformComponent(glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f)));

        auto& camera = m_engine->GetCamera();
        auto light = world.CreateEntity();
        world.AddComponent(light, Lights(LightType::E_Spot));
        world.AddComponent(light, TransformComponent(camera.position_, glm::vec3(1.0f), glm::normalize(camera.front_)));
        world.AddComponent(light, TagComponent("Light"));
        world.GetComponent<Lights>(light).radius_ = 50.0f;
        world.GetComponent<Lights>(light).innerCutoff_ = glm::cos(glm::radians(12.0f));
        world.GetComponent<Lights>(light).outerCutoff_ = glm::cos(glm::radians(17.0f));

        refreshRenderables();
    }

    void MyScene::update(float dt)
    {
        auto& world  = getWorld();
        auto& camera = m_engine->GetCamera();

        auto light = world.GetEntityByTag("Light");
        world.GetComponent<TransformComponent>(light).position_ = camera.position_;
        world.GetComponent<TransformComponent>(light).rotation_ = glm::normalize(camera.front_);
    }

    void MyScene::onUnload() {}
}
```

---
## 4. Lua Scripting

Pengu Engine uses **Lua** for entity scripting. Scripts are hot-reloaded automatically — you can edit a script while the engine is running and see changes instantly without restarting.

### Attaching a Script to an Entity

In C++, a script is attached to an entity by adding a `ScriptingComponent` with the path to your Lua file:

```cpp
ecs->AddComponent(entity, ScriptingComponent("../resource/scripts/my_script.lua"));
```

As a user of the engine, you just need to place your `.lua` file in the `resource/scripts/` folder and make sure it is referenced correctly.

### Script Structure

Every Lua script **must return a table**. The engine looks for three optional callback functions inside that table:

```lua
local MyScript = {}

-- Called once when the scene starts
function MyScript:onStart(entity)

end

-- Called every frame
function MyScript:onUpdate(entity, dt)

end

-- Called when the scene ends or the entity is destroyed
function MyScript:onFinish(entity)

end

return MyScript
```

| Callback | When it runs | Parameters |
|---|---|---|
| `onStart` | Once at scene start | `entity`, |
| `onUpdate` | Every frame | `entity`, `dt` (delta time in seconds) |
| `onFinish` | On scene end / entity destroyed | `entity` |

> All three callbacks are optional — only define the ones you need.

---

### Moving an Entity

Use `entity:setPosition()` to move an object:

```lua
local MyScript = {}

function MyScript:onStart(entity)
    entity:setPosition(vec3(0, 0, 0))
end

function MyScript:onUpdate(entity, dt)
    local pos = entity:getPosition()
    pos.x = pos.x + 1.0 * dt  -- move right over time
    entity:setPosition(pos)
end

return MyScript
```

---

### Handling Input

Use `IsKeyDown` to check if a key is held, or `IsKeyPressed` for a single press.

```lua
local PlayerScript = {}

function PlayerScript:onUpdate(entity, dt)
    local pos = entity:getPosition()
    local speed = 5.0

    if IsKeyDown(Action.MoveForward) then
        pos.z = pos.z - speed * dt
    end
    if IsKeyDown(Action.MoveBackward) then
        pos.z = pos.z + speed * dt
    end
    if IsKeyDown(Action.MoveLeft) then
        pos.x = pos.x - speed * dt
    end
    if IsKeyDown(Action.MoveRight) then
        pos.x = pos.x + speed * dt
    end

    entity:setPosition(pos)
end

return PlayerScript
```

> **Tip:** `IsKeyDown` returns `true` every frame the key is held. `IsKeyPressed` returns `true` only on the first frame the key is pressed.

---

### Rotating an Entity

```lua
local SpinScript = {}

function SpinScript:onUpdate(entity, dt)
    local rot = entity:getRotation()
    rot.y = rot.y + 90.0 * dt  -- rotate 90 degrees per second
    entity:setRotation(rot)
end

return SpinScript
```

---

### Storing State Between Frames

You can store variables directly in the script table — they persist across frames:

```lua
local CounterScript = {}

CounterScript.timer = 0.0

function CounterScript:onUpdate(entity, dt)
    self.timer = self.timer + dt
    if self.timer >= 1.0 then
        print("One second has passed!")
        self.timer = 0.0
    end
end

return CounterScript
```

> **Note:** When a script is hot-reloaded (because you saved changes), primitive values like numbers, strings, and booleans are preserved. Tables and functions are reset.

---

## 5. Scripting API Reference

### `vec3`

A 3D vector used for positions, rotations, and scale.

```lua
local v = vec3(1.0, 2.0, 3.0)
v.x  -- access x component
v.y  -- access y component
v.z  -- access z component
```

---

### Entity Methods

| Method | Returns | Description |
|---|---|---|
| `entity:getPosition()` | `vec3` | Get the entity's world position |
| `entity:setPosition(vec3)` | — | Set the entity's world position |
| `entity:getRotation()` | `vec3` | Get the entity's rotation (euler angles) |
| `entity:setRotation(vec3)` | — | Set the entity's rotation |
| `entity:getScale()` | `vec3` | Get the entity's scale |
| `entity:setScale(vec3)` | — | Set the entity's scale |
| `entity:getTransform()` | `Transform` | Get the full Transform component |
| `entity:setTransform(Transform)` | — | Set the full Transform component |

---

### Input Functions

| Function | Returns | Description |
|---|---|---|
| `IsKeyDown(Action.X)` | `bool` | `true` every frame the key is held down |
| `IsKeyPressed(Action.X)` | `bool` | `true` only on the first frame the key is pressed |

Replace `X` with your action name, e.g. `Action.MoveForward`.

---

### Available Lua Libraries

The engine exposes the following standard Lua libraries:

- `base` — print, type, pairs, ipairs, etc.
- `math` — math.sin, math.cos, math.floor, math.random, etc.

---

## Tips & Common Mistakes

**Script must return a table** — if your script doesn't end with `return MyScript`, the engine will throw an error.

**Use `self` in callbacks** — callbacks are defined with `:` so `self` refers to your script table. Use `self.myVar` to access stored state.

**Delta time is in seconds** — multiply speeds by `dt` in `onUpdate` so movement is frame-rate independent.

**Hot reload preserves numbers, strings, and booleans** — but not tables or functions. Don't rely on storing complex state that needs to survive a hot reload.

---

*Pengu Engine — PMG 2025/26*