# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Unreal Engine **5.7** C++ project (`ProjectShooting.uproject`, single runtime module `ProjectShooting`). It is a wave-based FPS/TPS shooter. Engine installed at `C:\Program Files\Epic Games\UE_5.7`. Rider is the primary IDE (`.idea/` present).

**Critical distinction — two code lineages in `Source/ProjectShooting/`:**
- `SX`-prefixed classes = the actual game. All new work goes here.
- `ProjectShooting*` (root: `ProjectShootingCharacter`, `...GameMode`, etc.) and `Variant_Horror/`, `Variant_Shooter/` = untouched Epic "First Person" template scaffolding. Treat as dead reference code; don't extend it. (`Config/DefaultGame.ini` still says "First Person Template" — vestigial.)

## Build & Run

No test framework is configured. Building = compiling the C++ module against the engine.

```bash
# Build the editor target (most common — needed before opening the editor after code changes)
"C:/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" \
  ProjectShootingEditor Win64 Development \
  -Project="C:/Users/dnjs4475/Documents/UnrealProjects/Project-SX/ProjectShooting.uproject" -WaitMutex

# Build the standalone game target
"C:/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" \
  ProjectShooting Win64 Development -Project="...ProjectShooting.uproject" -WaitMutex

# Regenerate IDE/project files after adding or moving source files
"C:/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/GenerateProjectFiles.bat" \
  -Project="...ProjectShooting.uproject" -Game -Rocket
```

Or build the `ProjectShooting.sln` from Rider/Visual Studio. Play in the editor from `UnrealEditor.exe` with the `.uproject`; the game boots into `MainMenuLevel` (`Config/DefaultEngine.ini`, `GlobalDefaultGameMode = BP_SXGameMode`).

`.uproject` plugins: `StateTree`, `GameplayStateTree`, `ModelingToolsEditorMode`. Module deps: `EnhancedInput`, `AIModule`, `StateTreeModule`, `GameplayStateTreeModule`, `UMG`, `Slate`.

## Conventions

- **`SX` prefix + `PROJECTSHOOTING_API`** on every game class; blueprint categories are namespaced `"SX|Area|Sub"`.
- **Blueprints subclass C++.** C++ classes are marked `Blueprintable`/`BlueprintSpawnableComponent` and their `BP_*` subclasses in `Content/ProjectShooting/` are what actually get spawned/placed. Config data lives in `UDataAsset`s (`USXInputConfig`, `USXStageWaveDataAsset`, `USXShopDataAsset`, `USXSkillData`) and `USTRUCT` tables — tune gameplay by editing assets, not hardcoding. C++ exposes `BlueprintImplementableEvent` hooks (e.g. `BP_OnDamaged`, `BP_OnDied`) for designers.
- **Dynamic multicast delegates are the primary decoupling mechanism.** Actors/components broadcast (`On...Changed`, `OnDeath`, `OnWaveCleared`, `OnStageStarted`, ...); controllers and UMG widgets subscribe. When adding state, follow this pattern rather than polling.

## Architecture

**Characters** (`Character/`): `ASXCharacterBase` (abstract) owns a `USXStatusComponent` and the current-weapon logic; routes engine `TakeDamage` → `ReceiveDamage` → component → `Die`. Two branches:
- `ASXPlayerCharacter` — Enhanced Input handlers, multiple view modes (`EViewMode`: first/third/back), iron-sight/FOV, full-auto selector, owns `USXSkillComponent`.
- `ASXEnemyCharacterBase` — attack range/interval + loot (`FSXDropItemData`); concrete `SXEnemyMelee`, `SXEnemyCharger`, `SXEnemyRanged`. Enemies are driven by `ASXEnemyAIController` via a simple **timer-based `UpdateAI` loop** (`AIUpdateInterval`) that calls `UpdateAIBehavior` — not a Behavior Tree/StateTree, despite the plugins being enabled.

**Stats** (`Components/SXStatusComponent.h`): the single source of truth for health, gold, experience, walk/sprint speed, and fire rate. Damage/heal/rewards flow through it and broadcast change delegates. (`SXHealthComponent` also exists but `SXStatusComponent` is the one wired into `ASXCharacterBase`.)

**Weapons** (`Item/SXWeapon.h`): `ASXWeapon` actor picked up via `USXPickupComponent`. Data-driven by `FSXAmmoData` per `ESXAmmoType` (Normal/Explosive/Piercing) × `ESXAmmoFireMethod` (LineTrace/Projectile), with per-type runtime magazine/reserve state. Firing fans out to `FireNormal/Piercing/Explosive` + `FireLineTrace/Projectile`. Projectiles: `SXAmmoProjectile` (player), `SXEnemyProjectile` (enemy).

**Stage / wave flow** (`Stage/`): `ASXStageFlowManager` is the orchestrator — finds `ASXWaveSpawner` + `ASXStageDoor`s, opens the entrance, runs waves sequentially, and fires `OnStageStarted/WaveStarted/Cleared/Failed`. Wave content comes from `USXStageWaveDataAsset` (or an inline `FSXStageWaveData` array; toggle `bUseStageWaveDataAsset`). `ASXWaveSpawner` spawns per-wave enemies, tracks alive count, and broadcasts clear. Triggers/doors: `SXStageStartTrigger`, `SXStageDoor`, `SXWaveClearDoor`.

**Skills** (`Skill/`): `USXSkillComponent` holds three slots (`ESXSkillSlot`: Movement/Skill1/Skill2), each configured by a `USXSkillData` asset instantiating a `USXSkillBase` subclass (`SXAttackSkill`, `SXDashSkill`, `SXDefenseSkill`, `SXUtilitySkill`), with cooldown/duration timers. Some skills spawn `SXSkillAreaActor` / `SXSkillBarrierActor`.

**GameModes / Controllers / State** (`GameMode/`, `Controller/`, `GameState/`, `PlayerState/`, `GameInstance/`): `ASXGameMode` → `ASXStageGameMode`, `ASXMenuGameMode`. `ASXPlayerController` centralizes UMG (HUD, main/pause/options/game-over menus), input-mode switching, and Enhanced Input mapping-context management; `SXGamePlayerController`/`SXMenuPlayerController` specialize it.

**Input** (`Input/SXInputConfig.h`): `USXInputConfig` is a `UDataAsset` holding all `UInputAction` references (move, look, fire, reload, skills, iron-sight, selector, ...). The player character binds these; the controller applies the `UInputMappingContext`s (`FSXInputMappingContextEntry` list, priority-ordered).

**Shop** (`Shop/`): `ASXShopActor` + `USXShopDataAsset`/`SXShopItemData`, integrating with `SXStatusComponent` gold. **Interaction**: `Interaction/SXInteractableInterface.h` — the player tracks an interaction candidate and calls into it.
