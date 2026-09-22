# Pengu Engine — Namespace & Forward Declaration Refactor Guide

## Overview

This guide walks you through refactoring `Pengu_Engine` from global classes to a clean `Pengu::` namespace hierarchy with forward declarations. Work through each phase in order — each one builds on the last.

---

## Target Namespace Structure

```
Pengu::
  ├── Core::           Application, Window, Timer, Logger, EventBus
  ├── Graphics::       Camera, Shader, Texture, Mesh, Material
  │     └── Rendering::  RenderPipeline, UnlitPipeline, LitPipeline
  ├── ECS::            Entity, Registry, System, SystemManager
  ├── Components::     TransformComponent, DrawableComponent, CameraComponent
  ├── Scene::          SceneBase, SceneManager, SceneGraph
  ├── Physics::        PhysicsWorld, RigidBody, Collider
  ├── Resources::      ResourceManager, AssetLoader, ShaderLibrary
  ├── Audio::          AudioEngine, AudioSource, AudioClip
  └── Input::          InputManager, Key, MouseButton
```

---

## Phase 1 — Wrap Base Classes First

These are the most critical because everything inherits from them. Get them right first.

**Files to change:** `RenderPipeline.hpp`, `SceneBase.hpp`, `System.hpp`

```cpp
// Before
class RenderPipeline { ... };

// After
namespace Pengu::Graphics::Rendering {
    class RenderPipeline { ... };
} // namespace Pengu::Graphics::Rendering
```

> **Why first?** If you wrap a derived class before its base, you'll get cascading compiler errors. Bases must be wrapped before their children.

---

## Phase 2 — Wrap Component & Data Types

Components are referenced almost everywhere as pointers or references — making them forward-declarable immediately cuts down includes across the whole engine.

**Files to change:** `TransformComponent.hpp`, `DrawableComponent.hpp`, `CameraComponent.hpp`

```cpp
// Before
struct TransformComponent { glm::vec3 position; ... };

// After
namespace Pengu::Components {
    struct TransformComponent { glm::vec3 position; ... };
    struct DrawableComponent  { ... };
} // namespace Pengu::Components
```

After this phase you can replace many `#include "Components/TransformComponent.hpp"` in headers with a forward declaration:

```cpp
namespace Pengu::Components {
    struct TransformComponent;  // no #include needed
}
```

---

## Phase 3 — Wrap Managers & Resources

`ResourceManager` is commonly passed by reference to `init()` methods — a perfect forward declaration candidate once wrapped.

**Files to change:** `ResourceManager.hpp`, `AssetLoader.hpp`, `ShaderLibrary.hpp`

```cpp
// Before
class ResourceManager { ... };

// After
namespace Pengu::Resources {
    class ResourceManager { ... };
} // namespace Pengu::Resources
```

In any header that only takes a `ResourceManager&`, replace the include:

```cpp
// header — forward declare only
namespace Pengu::Resources { class ResourceManager; }

// .cpp — full include here
#include "Pengu_Engine/Managers/Resource/ResourceManager.hpp"
```

---

## Phase 4 — Audit All Headers for Unnecessary Includes

Go through every `.hpp` file and ask: *"Do I need the full type here, or just a pointer/reference?"*

Use this checklist per header:

| Usage in header | Action |
|---|---|
| `T*` or `T&` parameter | Replace `#include` with forward declaration |
| `std::shared_ptr<T>` or `std::unique_ptr<T>` member | Replace with forward declaration |
| `T` by value as a member | Keep `#include` — compiler needs the size |
| Inheriting from `T` | Keep `#include` — compiler needs full definition |
| Calling `t.method()` | Move the call to `.cpp` — forward declare in header |

Add forward declarations at the top of the header after `#pragma once`:

```cpp
#pragma once
#include <memory>
#include <string>

// Forward declarations
namespace Pengu::Resources   { class ResourceManager; }
namespace Pengu::Components  { struct DrawableComponent; struct TransformComponent; }
namespace Pengu::Scene       { class SceneBase; }
namespace Pengu::Graphics    { class Camera; class Shader; }

// Now the actual class
namespace Pengu::Graphics::Rendering {
    class UnlitPipeline : public RenderPipeline { ... };
}
```

---

## Phase 5 — Apply to Pipelines (Concrete Example)

This is the `UnlitPipeline` from before, fully refactored:

### Header (`UnlitPipeline.hpp`)

```cpp
#pragma once
#include "Pengu_Engine/Graphics/Rendering/RenderPipeline.hpp" // must include — inheriting
#include <memory>
#include <string>
#include <glm/glm.hpp>

// Forward declarations — replaces 3 #includes
namespace Pengu::Resources   { class ResourceManager;    }
namespace Pengu::Components  { struct DrawableComponent; struct TransformComponent; }
namespace Pengu::Scene       { class SceneBase;          }
namespace Pengu::Graphics    { class Camera; class Shader; }

namespace Pengu::Graphics::Rendering {

    class UnlitPipeline : public RenderPipeline {
    public:
        void init(Pengu::Resources::ResourceManager& rm) override;
        void cleanup() override;
        void render(Pengu::Scene::SceneBase& scene, Pengu::Graphics::Camera& camera) override;
        void onResize(int width, int height) override;

        const std::string& getName() const override {
            static const std::string name = "Unlit";
            return name;
        }

    private:
        glm::mat4 buildModelMatrix(const Pengu::Components::TransformComponent& transform);
        void drawSubMeshes(const Pengu::Components::DrawableComponent& draw);
        void resetBlendState();

        std::shared_ptr<Pengu::Graphics::Shader> m_shader;
        int m_width  = 0;
        int m_height = 0;
    };

} // namespace Pengu::Graphics::Rendering
```

### Implementation (`UnlitPipeline.cpp`)

```cpp
#include "UnlitPipeline.hpp"

// Full includes go here — NOT in the header
#include "Pengu_Engine/Managers/Resource/ResourceManager.hpp"
#include "Pengu_Engine/Components/DrawableComponent.hpp"
#include "Pengu_Engine/Components/TransformComponent.hpp"
#include "Pengu_Engine/Graphics/Camera.hpp"
#include "Pengu_Engine/Graphics/Shader.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace Pengu::Graphics::Rendering {

    void UnlitPipeline::init(Pengu::Resources::ResourceManager& rm) { ... }
    void UnlitPipeline::render(Pengu::Scene::SceneBase& scene, Pengu::Graphics::Camera& camera) { ... }
    void UnlitPipeline::cleanup() { }
    void UnlitPipeline::onResize(int w, int h) { m_width = w; m_height = h; }

    glm::mat4 UnlitPipeline::buildModelMatrix(const Pengu::Components::TransformComponent& t) { ... }
    void UnlitPipeline::drawSubMeshes(const Pengu::Components::DrawableComponent& draw) { ... }
    void UnlitPipeline::resetBlendState() { }

} // namespace Pengu::Graphics::Rendering
```

---

## Phase 6 — Verify & Clean Up

Work through this list once all phases are done:

**1. Build incrementally** — after each file you refactor, compile immediately. Don't batch everything at once.

**2. Watch for `incomplete type` errors** — these mean you forward-declared something but then tried to use its size or call a method in a header. Move that code to the `.cpp`.

**3. Never put `using namespace` in a header** — this leaks into every file that includes yours. It's fine inside `.cpp` files.

```cpp
// ❌ Never in a header
using namespace Pengu::Graphics;

// ✅ Fine in a .cpp, or use a single-name import
using Pengu::Graphics::Shader;
```

**4. Use anonymous namespaces for internal helpers in `.cpp` files:**

```cpp
// UnlitPipeline.cpp
namespace {
    // Only visible inside this translation unit
    bool isValidShader(const Pengu::Graphics::Shader* s) { return s != nullptr; }
}

namespace Pengu::Graphics::Rendering {
    void UnlitPipeline::init(...) {
        if (isValidShader(m_shader.get())) { ... }
    }
}
```

**5. Commit each phase separately** — this makes rollback trivial if something breaks.

---

## Quick Reference Card

```cpp
// ── In a HEADER ───────────────────────────────────────────────
#pragma once
#include "base.hpp"                        // must include if inheriting

namespace Pengu::Foo { class Bar; }        // forward declare references/pointers

namespace Pengu::MySystem {
    class MyClass : public Base {
        void doThing(Pengu::Foo::Bar& b);  // ✅ reference — forward decl enough
        std::shared_ptr<Pengu::Foo::Bar> m_bar; // ✅ smart pointer — forward decl enough
        // Pengu::Foo::Bar m_bar;          // ❌ by value — would need full include
    };
}

// ── In a .CPP ─────────────────────────────────────────────────
#include "MyClass.hpp"
#include "Pengu_Engine/Foo/Bar.hpp"        // full include here

namespace Pengu::MySystem {
    void MyClass::doThing(Pengu::Foo::Bar& b) {
        b.method(); // ✅ can call methods now — full type available
    }
}
```
