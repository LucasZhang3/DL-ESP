# Development guide

[← README](../README.md)

## Navigating the project

### Recommended reading order

1. [execution-flow.md](execution-flow.md) - when code runs
2. [architecture.md](architecture.md) - how modules connect
3. [esp.md](esp.md) or [gui.md](gui.md) - depending on your task
4. [sdk-and-game-data.md](sdk-and-game-data.md) - game-facing changes

### Open in IDE

- Solution: `Deadlock.sln`
- Set configuration: **Release | x64** (matches `REL_BUILD` solution mapping)
- Output: `Deadlock/x64/Release/Deadlock.dll` (path may vary by VS settings)

### Find feature code quickly

| Task | Start here |
|------|------------|
| New menu toggle | `Settings.hpp` → `CSettingsJson.cpp` → `CDeadlockMenu.cpp` |
| New ESP element | `CVisual.cpp` + `CRenderStackSystem` |
| New hook | `DeadLock/Hook/Hook_*.cpp` + `CHook_Loader.cpp` |
| Game function | `CFunctionList.hpp` + `FunctionListSDK.hpp` |
| Crash on entity | `CEntityData.hpp`, `CEntityCache.cpp` |

---

## Building

- **Toolset:** v143 (VS 2022)
- **Standard:** C++20
- **Character set:** MultiByte (x64 Release)
- **Preprocessor:** `RELEASE_BUILD`, `DEADLOCK_EXPORTS`, `NDEBUG`

Do not rely on this doc for exact VS version - see `Deadlock.vcxproj`.

**Libraries** (project directory): `freetype.lib`, `FW1FontWrapperRel.lib`, `libprotobuf.lib`, `steam_api64.lib`, `VMProtectSDK64.lib`.

---

## Adding a new visual feature

### Safe path

1. Add `Settings::Visual::YourFeature` default in `Settings.hpp`
2. Serialize in `CSettingsJson::LoadConfig` / `SaveConfig`
3. Add checkbox in `CDeadlockMenu::RenderVisualTabContent`
4. In `CVisual::OnClientOutput` or `OnRender`:
   - Read setting
   - Resolve entities (cache or `FOR_EACH_ENTITY`)
   - Call `GetRenderStackSystem()->Draw*` - **do not** call ImGui draw directly from engine hook
5. Test with menu closed and open

**Reference implementation:** health ESP - `ShowHealth` / `ShowHealthBar` in `Settings.hpp`, JSON in `CSettingsJson.cpp`, menu in `CDeadlockMenu.cpp`, drawing in `CVisual::OnRenderPlayerEsp()` using `m_PlayerDataGlobal().m_iHealth()` / `m_iHealthMax()`.

### Render stack contract

- **Enqueue** only from `OnClientOutput` (or functions it calls)
- **Never** enqueue from random threads without locking (mutex exists but ImGui is not thread-safe)
- Call `EnsureUpdateCapacity(n)` if you expect many primitives per frame

---

## Adding a new menu tab

1. Add `ImGui::BeginTabItem` in `RenderRightChild()`
2. Implement `RenderYourTabContent()` mirroring scroll child pattern
3. Persist settings via `CSettingsJson` if needed

---

## Adding a hook

1. Create `Hook_YourFeature.hpp/.cpp` with detour + `*_o` original
2. Add pattern entry to `CHook_Loader::InstallSecondHook` vector
3. Document DLL name and failure mode (`m_bSkipIfNotFound` if optional)
4. Keep detour minimal - delegate to singleton

**Danger:** Installing hooks after game threads call targets requires ordering - all hooks install once in init thread before return.

---

## Dangerous systems

| System | Why |
|--------|-----|
| `CHook_Loader` pattern table | Wrong pattern = crash on hook enable |
| `Hook_ParseMessage` offsets | Protobuf layout change = invalid memory reads |
| `WndProc` hook | Breaks input if not forwarded correctly |
| `OnDestroy` / unload | Race with game render thread |
| Direct `CEntityData` field access | Wrong offset = subtle corruption |
| Calling game virtuals without init check | Null interface crash |

---

## Rendering cautions

- Use **background draw list** via `CRender` for ESP (world-aligned 2D overlay)
- `WorldToScreen` requires valid ImGui frame (`DisplaySize`) - fails outside Present path
- `DrawCircle3D` still projects through game's view; extreme angles may clip oddly
- FW1 `DrawString` uses D3D11 device from GUI - fonts init on first `OnRender`

---

## Hook cautions

- Overlay Present may differ from borderless/fullscreen swap chains - test both
- `ResizeBuffers` destroys GUI - expect full re-init on display mode change
- `MH_EnableHook(MH_ALL_HOOKS)` enables everything at once - partial enable not used
- `CreateMove` runs in prediction context - heavy work causes frame time spikes

---

## Config cautions

- Always clamp user-facing ints in `GetIntJson`
- Stamp file must stay basename-only (security check in `ReadLastLoadedConfigStamp`)
- `SaveConfig` overwrites entire JSON - no merge with unknown keys from older files (keys not in save path are dropped on round-trip)

---

## Testing checklist (manual)

- [ ] DLL loads; `debug.log` shows init success (if logging on)
- [ ] Insert opens menu; game input suppressed appropriately
- [ ] Load/Save/Create/Delete config
- [ ] Restart DLL - `last_loaded_config.txt` restores settings
- [ ] ESP toggles with local player never boxed
- [ ] Health text/bar update when damage is dealt (enemy + teammate)
- [ ] Enemy-only footsteps do not show teammates
- [ ] Bones respect team/enemy toggles
- [ ] Game update: verify patterns still scan (function list + hooks)

---

## Code style (observed)

- `auto` return types with trailing `-> void` / `-> bool`
- Singleton `GetX()` accessors
- `XorStr()` for string literals in many paths (may be no-op if disabled)
- Hungarian-ish member prefixes: `m_p`, `m_b`, `g_` for globals
- Feature settings as `inline` namespace variables

Match surrounding files when contributing.

---

## Git / hygiene

- Do not commit `x64/Release/`, `.vs/`, `debug.log`, `gui.ini`, personal `*.json` if they contain secrets
- Protobuf `.pb.cc` files are bulky - avoid drive-by formatting
- `FontAwesomeIcon.hpp` is generated-scale - do not hand-edit icon constants

---

## When game updates break the build

Typical fix order:

1. Re-scan hook patterns (overlay + client + engine2)
2. Re-scan `CFunctionList` patterns
3. Update `Offsets.hpp` from reversing `ParseMessage` / usercmd if network code changed
4. Refresh schema / `CEntityData` (or run dump flags locally)
5. Update `CHeroIdLookup` table for new heroes (cosmetic)

No automated pipeline exists in-repo for steps 1–4.
