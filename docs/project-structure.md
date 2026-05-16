# Project structure

[← README](../README.md)

Root layout:

```
DL Source/
├── Deadlock.sln
├── Deadlock/
│   ├── Deadlock.vcxproj
│   ├── Deadlock.vcxproj.filters
│   └── DeadlockMod/          ← all C++ source
├── README.md
├── docs/
└── schema_dump.hpp           ← optional schema artifact (repo root)
```

---

## `DeadlockMod/` — source root

| Path | Purpose |
|------|---------|
| `DllMain.cpp` / `DllMain.hpp` | DLL entry point |
| `DllLauncher.cpp` / `.hpp` | Init thread, paths, teardown |
| `Common/` | Shared utilities and third-party headers |
| `DeadLock/` | SDK, hooks, protobuf, patterns |
| `GameClient/` | Entity cache, player/bone helpers |
| `DeadlockClient/` | Menu, ESP, render stack, settings |

---

## `Common/`

| Path | Purpose |
|------|---------|
| `Common.hpp` | Shared includes / macros |
| `Config.hpp` | **Global defines:** `CHEAT_NAME`, file names, build flags |
| `DevLog.*` | File/console logging (`DEV_LOG`) |
| `CrashLog.*` | Vectored exception handler |
| `MemoryEngine.*` | Pattern scan helpers |
| `Base64.*` | Encoding utility |
| `CDelay.hpp` | Timing helper |
| `Helpers/ModuleLoaderHelper.*` | `IsModuleLoad` etc. |
| `Helpers/StringHelper.*` | UTF-8 / wide conversions |
| `Include/ImGui/` | Dear ImGui + DX11/Win32 backends + FreeType |
| `Include/MinHook/` | MinHook library sources |
| `Include/rapidjson/` | JSON for configs |
| `Include/FW1FontWrapper/` | DirectX font drawing for ESP text |
| `Include/SteamAPI/` | Steam API headers (not full feature set used) |
| `Include/XorStr/` | Compile-time string obfuscation (optional) |
| `Include/stb/`, `lodepng/`, `google/protobuf/`, etc. | Vendored deps |

**Key file:** `Config.hpp` — changing `CHEAT_NAME`, `CONFIG_FILE`, or log flags affects entire project.

---

## `DeadLock/`

### `DeadLock/Hook/`

One pair `.hpp`/`.cpp` per detour. Each cpp includes feature headers and calls singletons.

| File | Role |
|------|------|
| `Hook_Present.cpp` | DXGI Present → GUI |
| `Hook_ResizeBuffers.cpp` | Teardown GUI on resize |
| `Hook_CreateSwapChain.cpp` | Clear RTV |
| `Hook_OnClientOutput.cpp` | ESP tick + render stack publish |
| `Hook_OnAddEntity.cpp` / `OnRemoveEntity.cpp` | Entity cache |
| `Hook_CreateMove.cpp` | UserCmd hook (stub client handler) |
| `Hook_ParseMessage.cpp` | Sound events → footsteps |
| `Hook_FireEventClientSide.cpp` | Game events (stub) |
| `Hook_MouseInputEnabled.cpp` | Menu blocks game mouse |
| `Hook_IsRelativeMouseMode.cpp` | Relative mouse override |
| `Hook_GetMatricesForView.cpp` | Passthrough only |

### `DeadLock/SDK/`

| Area | Files | Role |
|------|-------|------|
| Core | `SDK.hpp`, `SDK.cpp` | Interface accessors, DLL name constants |
| Patterns | `CFunctionList.*`, `FunctionListSDK.hpp` | Scanned game functions |
| Schema | `CSchemaOffset.*`, `CShemaSystemSDK.hpp` | Runtime schema walk |
| Types | `Types/CEntityData.hpp`, `CHandle.hpp`, … | Entity definitions |
| Math | `Math/*` | Vectors, `WorldToScreen` via `ScreenTransform` |
| Interface | `CGameEntitySystem.hpp`, `IEngineToClient.hpp`, … | Engine API wrappers |
| Update | `Offsets.hpp`, `CUserCmd.hpp`, `CCitadelInput.hpp` | Hardcoded offsets |
| SDL3 | `SDL3_Functions.*` | Dynamic SDL3 exports for mouse warp |
| Network | `CNetworkMessages.hpp` | Message IDs / serializers |

### `DeadLock/Protobuf/`

Generated `.pb.h` / `.pb.cc` from game network protos. Used by `Hook_ParseMessage` and related. Large; do not edit by hand.

### Top-level `DeadLock/`

| File | Role |
|------|------|
| `CHook_Loader.*` | Hook table install/destroy |
| `CSDK_Loader.*` | Interface init gate |
| `CBasePattern.*` | Single pattern search helper |

---

## `GameClient/`

| Path | Role |
|------|------|
| `CEntityCache/` | Vector of `CachedEntity_t` + mutex |
| `CL_CitadelPlayerController.*` | `GetLocal()` → `CGameEntitySystem::GetLocalCitadelPlayerController` |
| `CL_CitadelPlayerPawn.*` | Local pawn accessor |
| `CL_Bones.*` | `g_AllSkeletonPairBones` + `GetBonePositionByName` |

---

## `DeadlockClient/`

| Path | Role |
|------|------|
| `CDeadlockClient.*` | Hook callback multiplexer |
| `CDeadlockGUI.*` | D3D11 + ImGui + WndProc + themes |
| `GUI/CDeadlockMenu.*` | Menu layout and widgets |
| `Features/CVisual/` | ESP implementation |
| `Render/CRender.*` | Immediate ImGui draw primitives |
| `Render/CRenderStackSystem.*` | Deferred command queue |
| `Fonts/` | FW1 fonts + Font Awesome icon font |
| `Settings/Settings.hpp` | Runtime toggles |
| `Settings/CSettingsJson.*` | JSON persistence |
| `Data/CHeroIdLookup.hpp` | HeroID → name table (header-only) |
| `Resources/UI_Sounds/` | Embedded UI sound bytes (headers) |

---

## Important file reference

### `CDeadlockClient.hpp`

Declares `IDeadlockClient` / `CDeadlockClient` with virtual hooks:

- `OnInit`, `OnFireEventClientSide`, `OnAddEntity`, `OnRemoveEntity`
- `OnStartSound`, `OnClientOutput`, `OnRender`, `OnCreateMove`

### `CDeadlockGUI.hpp`

- `EDeadlockGuiStyle`: INDIGO, VERMILLION, CLASSIC_STEAM, CHARCOAL
- `FreeTypeBuild` for optional FreeType atlas rebuild
- Public `m_pFontAwesomeIcons` for menu icons

### `CEntityCache.hpp`

- `CachedEntity_t::Type`: UNKNOWN, CITADEL_PLAYER_CONTROLLER, CITADEL_PLAYER_PAWN
- `m_bVisible` field exists but is **not updated** in current `CEntityCache.cpp`

### `CRenderStackSystem.hpp`

Defines `IRenderObject` hierarchy: line, box, coal box, circle 3D, string, etc.

---

## Visual Studio filters

`Deadlock.vcxproj.filters` mirrors folder structure under virtual filters `DeadlockMod\...` for Solution Explorer — no impact on compile.

---

## What is intentionally not in source tree

- Injector executable
- Automated offset updater
- Unit tests
- CI scripts
