# ESP / visual features

[← README](../README.md)

Implementation: `DeadlockClient/Features/CVisual/CVisual.cpp`  
Settings: `DeadlockClient/Settings/Settings.hpp`

---

## Master toggles

| Setting | Default | Role |
|---------|---------|------|
| `Settings::Visual::Active` | `false` | Enables player box ESP path in `OnRender()` |
| `Settings::Visual::EnemyEsp` | `false` | Draw non-teammates |
| `Settings::Visual::TeamEsp` | `false` | Draw teammates |
| `Settings::Visual::ShowHeroName` | `false` | Text above box |
| `Settings::Visual::ShowHealth` | `false` | `current/max` HP text below box |
| `Settings::Visual::ShowHealthBar` | `false` | HP bar below box (team-colored fill) |
| `Settings::Visual::SoundStepEsp` | `false` | Footstep circles |
| `Settings::Visual::BonesEsp` | `false` | Skeleton lines |

**Interaction matrix:**

| Feature | Requires `Active`? | Requires team/enemy flags? |
|---------|-------------------|---------------------------|
| Box ESP | **Yes** | Per-player team check |
| Hero name | Yes (via box path) | Same as box |
| Health text / bar | Yes (via box path) | Same as box; toggles independent |
| Bones ESP | **No** (separate branch) | Same team/enemy checks |
| Footstep ESP | **No** | `EnemyEsp` only in sound handler + render |

---

## Execution order (per engine output)

**Entry:** `Hook_OnClientOutput` → `CVisual::OnClientOutput()`

```
1. CRenderStackSystem::EnsureUpdateCapacity(768)
2. CVisual::OnRender()          // boxes + footstep render
3. if BonesEsp → FOR_EACH_ENTITY skeleton pass
4. (later, same hook) CRenderStackSystem::OnClientOutput() publishes buffer
```

**Draw:** `Hook_Present` → `CRenderStackSystem::OnRenderStack()` executes queued commands.

---

## Entity iteration (box ESP)

**Source:** `CEntityCache::GetCachedEntity()` — vector populated by add/remove entity hooks.

**Loop** (`CVisual::OnRender`):

1. Lock cache mutex
2. `GetCL_CitadelPlayerController()->GetLocal()` for team filtering
3. For each `CachedEntity_t`:
   - Resolve `m_Handle.Get()` → `C_BaseEntity*`
   - Verify handle still matches entity identity
   - Only process `CITADEL_PLAYER_CONTROLLER`

**Per controller:**

1. Get pawn: `m_hHeroPawn().Get<C_CitadelPlayerPawn>()`
2. **Skip local player** if controller == local
3. **Team filter:** skip if same team && !`TeamEsp`; skip if !same team && !`EnemyEsp`
4. World positions:
   - Origin: `m_vOldOrigin()` on pawn
   - Head: bone `"head"` via `GetBonePosition`
5. `Math::WorldToScreen` both points
6. Build `Rect_t` from head/feet screen Y and width = height/2
7. `OnRenderPlayerEsp(controller, rect, local)`

**Not used for boxes:** live entity index loop (only cache).

---

## Local player exclusion

Explicit checks:

```cpp
if (pCachedLocalCtrl && pCCitadelPlayerController == pCachedLocalCtrl)
    continue;
```

Also in `OnRenderPlayerEsp` (defensive duplicate).

---

## Team / enemy filtering

**Color helper** `GetCitadelPlayerEspColor`:

| Condition | Color (RGB) |
|-----------|-------------|
| No local | White |
| Local player | White |
| Same team | Blue `(0,0,255)` |
| Enemy | Red `(255,0,0)` |

Filtering uses `m_iTeamNum()` on controller vs local.

---

## Box rendering

**`OnRenderPlayerEsp`:**

- Skip if not `m_PlayerDataGlobal().m_bAlive()`
- `GetRenderStackSystem()->DrawOutlineCoalBox(min, max, PlayerColor)`
- Coal box = corner-style outline (see `CRender::DrawOutlineCoalBox`)

**Hero label** (if `ShowHeroName`):

- `m_nHeroID().m_Value` → `HeroIdLookup::FormatEnemyHeroEspLabel`
- `DrawString` centered above box (`FW1_CENTER`), Verdana font

**Health** (if `ShowHealth` and/or `ShowHealthBar`):

Data source (schema, already defined in `CEntityData.hpp`):

```cpp
pCCitadelPlayerController->m_PlayerDataGlobal().m_iHealth();     // int32
pCCitadelPlayerController->m_PlayerDataGlobal().m_iHealthMax(); // int32
```

Behavior in `OnRenderPlayerEsp()`:

- Negative health clamped to `0`; current health clamped to max if overshoot
- **Bar** (`ShowHealthBar`): only if `m_iHealthMax > 0`; 3px tall, full box width; dark gray background; fill width ∝ `current/max`, color = `PlayerColor` (team blue / enemy red)
- **Text** (`ShowHealth`): centered below bar (or at `box bottom + 2` if bar off); format `"%d/%d"` or `"HP: ?"` when max is 0
- Independent toggles: text-only, bar-only, or both

---

## Bones ESP

**When:** `Settings::Visual::BonesEsp` in `OnClientOutput` (not gated by `Active`).

**Iteration:** `FOR_EACH_ENTITY(idx)` macro in `CGameEntitySystem.hpp` — scans all entity indices.

**Filters:**

- Entity is citadel player controller
- Not local controller
- Team/enemy flags (same as boxes)
- `m_bAlive()`
- Valid citadel player pawn

**Skeleton:** `g_AllSkeletonPairBones` in `CL_Bones.cpp` — 24 bone name pairs (spine, arms, legs).

For each pair:

- `GetCL_Bones()->GetBonePositionByName` → world positions
- `WorldToScreen` both ends
- `DrawLine` with 2px thickness and team color

**Bone update:** `CalcWorldSpaceBones(FLAG_ALL_BONE_FLAGS)` inside bone helper.

---

## Footstep ESP (sound)

### Collection — `OnStartSound`

**Trigger:** `Hook_ParseMessage` for `GE_SosStartSoundEvent`.

**Filters:**

1. `Settings::Visual::EnemyEsp` must be true (checked at start — **not** `SoundStepEsp`)
2. Sound name substring `"Footstep"`
3. Source entity not local pawn
4. Source team != local team
5. Entity is citadel player pawn

Stores `SoundData_t { GetTickCount64(), Pos }` in `m_SoundList`.

### Rendering — `OnRenderSound`

Called from `OnRender()` when `SoundStepEsp` true.

- Remove entries older than `g_SoundShowTime` (1000 ms)
- If `!EnemyEsp`, clear list and return
- For each sound: `WorldToScreen`, `DrawCircle3D` with:
  - Color: **fixed** `ImColor(1,1,0,Alpha)` — **not** `Settings::Colors::Visual::SoundStepEsp`
  - Radius lerps 20 → 0 over lifetime

---

## World-to-screen

**File:** `DeadLock/SDK/Math/Math.cpp`

Uses game function **`ScreenTransform`** (pattern-scanned in `CFunctionList`) to project world → clip, then maps to ImGui display size:

```cpp
vOut.x = ((Out.m_x + 1.0f) * 0.5f) * DisplaySize.x;
vOut.y = DisplaySize.y - (((Out.m_y + 1.0f) * 0.5f) * DisplaySize.y);
```

Depends on ImGui `DisplaySize` being valid (Present frame).

---

## Render stack commands used

| ESP feature | Stack API |
|-------------|-----------|
| Player box | `DrawOutlineCoalBox` |
| Hero name | `DrawString` |
| Health text | `DrawString` |
| Health bar | `DrawFillBox` (background + fill) |
| Bones | `DrawLine` |
| Footsteps | `DrawCircle3D` |

All funnel to `CRender` → ImGui `GetBackgroundDrawList()`.

---

## Render ordering

Within one frame:

1. **Enqueue** (OnClientOutput): all ESP commands pushed to `m_vecUpdateBuffer`
2. **Publish:** buffer swapped to shared ptr
3. **Present / OnRender:** menu ImGui widgets drawn first in `NewFrame`…`EndFrame`
4. **OnRenderStack:** ESP draw lists executed (background draw list — typically on top of game, under ImGui windows depending on order)

Watermark (`CHEAT_NAME`) drawn via FW1 in `OnRender` **before** `OnRenderStack`.

Exact Z-order vs game world is determined by overlay RTV binding, not depth-tested world geometry.

---

## Hero ID handling

**File:** `DeadlockClient/Data/CHeroIdLookup.hpp`

- Static sorted table of 38 hero IDs → names
- Binary search `TryLookupHeroName`
- `FormatEnemyHeroEspLabel` — unknown IDs formatted as numeric string (see header comment)

**Limitation:** Table is manually maintained; new heroes show numbers until table update.

---

## Feature checklist for developers

| To add… | Extend… |
|---------|---------|
| New ESP type | `CVisual::OnRender` or `OnClientOutput`, enqueue via `CRenderStackSystem` |
| New filter | Entity cache type or `FOR_EACH_ENTITY` branch |
| New color rule | `GetCitadelPlayerEspColor` or per-feature color |
| Persisted toggle | `Settings::Visual`, `CSettingsJson` load/save, `CDeadlockMenu::RenderVisualTabContent` |

---

## Screenshots (placeholders)

| File | Content |
|------|---------|
| `docs/images/esp-boxes.png` | Enemy + team colors |
| `docs/images/esp-bones.png` | Skeleton overlay |
| `docs/images/esp-footsteps.png` | Yellow 3D circles |
| `docs/images/esp-hero-names.png` | Labels above boxes |
| `docs/images/esp-health.png` | HP text and/or bar below boxes |

---

## Uncertainties / gaps

- `CachedEntity_t::m_bVisible` is never set in code — no visibility check for ESP
- Commented-out hitbox/bone dump blocks in `CVisual.cpp` are debug-only, not features
- Pawn-type cache entries are stored but **not** rendered in `OnRender` switch (only controller case)
- **Enemy/teammate HP replication** not verified statically — confirm in-game that values update on damage/heal
- Health display requires **Player ESP** box path (`Active` + team/enemy flags); not shown in bones-only mode without boxes
