# TODO

### - [X] Step 1 — Low risk, high value (do this first)

```cpp
// Just add EngineConfig, keep create() working
static std::optional<PenguEngine> create(const EngineConfig& config);
static std::optional<PenguEngine> create(int w, int h, std::string t); // keep old one too
```

### - [X] Step 2 — Fix ECS ownership (medium risk)

```cpp
// Change GetECS() return type only
ECSManager& GetECS() { return *ecs_; }
// Then fix call sites one by one
```

### - [ ] Step 3 — Add Scene system (zero breakage)

```cpp
// Purely additive — add LoadScene() and Run()
// without removing anything yet
```
### - [ ] Step 4 — Clean up main last

Once scenes work, main shrinks naturally and the old accessors can be removed safely.

## After doing these changes then implement these changes

### - [ ] Priority 1 — Scene & Game Loop (biggest impact)

The Scene base class + Run() loop. This is the change that makes everything else easier because it gives you a clear place to put game logic that isn't main.
```
PenguEngine
└── Run()
    └── activeScene_->OnLoad()
    └── loop:
        ├── OnUpdate(dt)
        └── OnRender()
```
### - [ ] Priority 2 — Camera Decoupling

Right now camera lives in the engine globally. Problems:

Can't have multiple cameras
Can't switch cameras per scene
Projection matrix leaks into TransformComponent

Fix: Move camera into the scene, or make it a proper CameraComponent on an entity. Projection matrix moves to the camera, not the transform.

### - [ ] Priority 3 — Event System

Right now systems probably call each other directly or you check input inline everywhere. An EventBus fixes this:

```cpp
// Instead of this scattered in the loop:
if (inp.isPressed(Action::C)) { /* spawn light */ }

// You get this:
eventBus.Subscribe<KeyPressEvent>([](auto& e) {
    if (e.action == Action::C) SpawnFlashlight();
});
```
Completely decouples input from gameplay logic.

### - [ ] Priority 4 — Resource Handle System

Right now ResourceManager probably returns raw shared_ptr. The problem is anything can hold a resource alive forever accidentally.
Fix: Handle-based resources:
```cpp
// Instead of:
std::shared_ptr<Shader> s = rm.getShader(...);

// You get:
ShaderHandle s = rm.getShader(...);  // just an ID + weak ref
```
Safer, cacheable, and enables async loading later.

---

### - [ ] Priority 5 — Renderer Abstraction

`RenderSystem` directly calls OpenGL right now I assume. This makes it impossible to swap backends later. A thin abstraction layer:
```
RenderSystem
└── submits DrawCalls to →
    Renderer
    └── GraphicsAPI (OpenGL / Vulkan / etc.)
```

---

### - [ ] Priority 6 — Layer Stack

Right now UI, gameplay, and debug rendering are all mixed. A `LayerStack` lets you separate them cleanly:
```
LayerStack
├── GameLayer       — ECS update + render
├── DebugLayer      — debug overlays, gizmos
└── UILayer         — ImGui / StatPanel
```

Each layer has its own `OnUpdate` / `OnRender`. Easy to enable/disable.

---

### Suggested Order
```
Scene system          ← do first, unlocks everything
Camera decoupling     ← fixes the projection matrix issue
EventBus              ← cleans up input handling
Resource handles      ← prevents memory leaks at scale
Renderer abstraction  ← future-proofs graphics backend
LayerStack            ← polish, do last
```
## Change renderer lights
Your ECS doesn't have a GetAllEntities() method, but you already have HasComponent<T>() which is perfect. The cleanest solution for your specific ECS is to register a separate LightSystem with its own signature, so lights get their own entities_ set automatically:
```cpp
// A minimal system just to track light entities
class LightSystem : public System {
public:
    // entities_ will automatically contain only light entities
    // thanks to the signature
};
```
Register it alongside your Renderer:
```cpp
// Renderer signature — only drawables
auto rendersys = ecs->RegisterSystem<Renderer>(*ecs, rendShader, shadShader);
{
    Signature signature;
    signature.set(ecs->GetComponentType<DrawableComponent>());
    ecs->SetSystemSignature<Renderer>(signature);
}

// LightSystem signature — only lights
auto lightsys = ecs->RegisterSystem<LightSystem>();
{
    Signature signature;
    signature.set(ecs->GetComponentType<Lights>());
    ecs->SetSystemSignature<LightSystem>(signature);
}
```
Then pass it to your Renderer:
```cpp
class Renderer : public System {
public:
    Renderer(ECSManager& ecs, 
             std::shared_ptr<LightSystem> lightSystem,
             std::shared_ptr<Shader> renderShader, 
             std::shared_ptr<Shader> shadowShader)
        : ecs_(ecs)
        , m_lightSystem(lightSystem)
        , m_defaultShader(renderShader)
        , m_shadowShader(shadowShader)
    {}

private:
    ECSManager&                  ecs_;
    std::shared_ptr<LightSystem> m_lightSystem; // borrows light entities
    std::shared_ptr<Shader>      m_defaultShader;
    std::shared_ptr<Shader>      m_shadowShader;
    ShadowMap                    m_shadowMap;
};
```
Then your render methods become really clean:
```cpp
void Renderer::render(Camera& camera, Window& window)
{
    if (m_lightSystem->entities_.empty())
        renderNoLights(camera, window);
    else
        renderWithLights(camera, window);
}

void Renderer::renderWithLights(Camera& camera, Window& window)
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    bool firstPass = true;

    // Clean iteration — only actual light entities
    for (auto lightId : m_lightSystem->entities_) {
        auto& light          = ecs_.GetComponent<Lights>(lightId);
        auto& lightTransform = ecs_.GetComponent<TransformComponent>(lightId);

        glm::mat4 lightSpaceMatrix = buildLightSpaceMatrix(lightTransform);

        shadowPass(lightSpaceMatrix);
        lightingPass(light, lightTransform, lightSpaceMatrix, camera, window, firstPass);

        firstPass = false;
    }

    resetBlendState();
}
```
This way:

```
entities_ in Renderer → only drawables, managed automatically by ECS signature
m_lightSystem->entities_ → only lights, managed automatically by ECS signature
LightEntities_ → gone entirely, no manual bookkeeping
```
Your ECS does all the filtering for you, which is exactly what it's designed for

And adding a light is now just:
```cpp
auto sun = ecs->CreateEntity();
ecs->AddComponent(sun, Lights(LightType::E_Directional));
ecs->AddComponent(sun, TransformComponent(...));
// automatically appears in m_lightSystem->entities_ — no manual insert needed
```