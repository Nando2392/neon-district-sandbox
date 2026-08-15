# Scorecard — Neon District Sandbox

Actualizado: 2026-08-15 20:07 local. Fuente de verdad: exe empaquetado `dist/Windows/NeonDistrictSandbox.exe`, log runtime y `Saved/Benchmark/NDBenchmarkResult.txt`.

Leyenda: ✅ PASS · ⚠️ WARN · ❌ FAIL · ⏸ PENDIENTE

## Resumen ejecutivo

**Estado actual: ✅ MAQUETA 3D JUGABLE EMPAQUETADA — 25/25 tests automatizados PASS.**

Esto NO significa “juego final visualmente terminado”. Significa que el vertical slice ya corre sin Unreal y que los sistemas principales funcionan en el ejecutable Windows. La siguiente sesión debe convertir la maqueta/greybox en una presentación renderizada con assets reales.

## Packaging / runtime

| Criterio | Estado | Evidencia |
|---|---:|---|
| Unreal Engine | ✅ PASS | UE 5.8 instalado en `C:\Program Files\Epic Games\UE_5.8` |
| BuildCookRun | ✅ PASS | `BUILD SUCCESSFUL`, ExitCode=0, 2026-08-15 |
| Ejecutable standalone | ✅ PASS | `dist/Windows/NeonDistrictSandbox.exe` ejecutado, exit 0 |
| Mapa correcto | ✅ PASS | Log: `Browse: /Game/Maps/ND_City?Name=Player`, `LoadMap: /Game/Maps/ND_City` |
| GameMode correcto | ✅ PASS | Log: `Game class is 'NDGameMode'` |
| Config suelto corrupto removido | ✅ PASS | `dist/Windows/NeonDistrictSandbox/Config` eliminado antes del benchmark final |

## Benchmark automatizado empaquetado — 25/25 PASS

```text
Map: /Game/Maps/ND_City
[PASS] world.builder — ANDWorldBuilder actors: 1
[PASS] world.spawner — ANDCitySpawner actors: 1
[PASS] gameplay.player_controller — GetPlayerController(0) exists
[PASS] gameplay.player_pawn — possessed NDCharacter
[PASS] systems.game_instance — UGameInstance present
[PASS] systems.mission — UNDMissionSystem present
[PASS] systems.wanted — UNDWantedSystem present
[PASS] ai.civilians — civilians: 15 (>=10)
[PASS] ai.police — police: 2 (>=1)
[PASS] ai.total_npcs — total NPCs: 17 (>=12)
[PASS] vehicle.count — ANDVehicle actors: 3 (>=1)
[PASS] mission.accept — stage 0 -> 1
[PASS] mission.complete — stage after complete: 3
[PASS] wanted.heat — wanted 0 -> 1 after ReportDetection
[PASS] wanted.level2 — SetWantedLevel(2) -> 2
[PASS] wanted.clear — ClearWanted -> 0
[PASS] vehicle.enter — IsDriving after EnterVehicle: true
[PASS] vehicle.drive_input — ApplyDriveInput(0.3, 0.5) + SetHandbrake(false) applied
[PASS] vehicle.exit — IsDriving after ExitVehicle: false
[PASS] controls.pause — HandlePause -> paused=true; resume -> running=true
[PASS] save.write — UNDGameInstance present
[PASS] save.load — same cause as save.write
[PASS] save.write — SaveGame() -> true
[PASS] save.load — LoadGame() -> true
[PASS] gameplay.player_move — moved 50.4 cm over 6 input ticks (>=10)

=== RESULT: 25 passed, 0 failed ===
```

## Visual / assets

| Criterio | Estado | Evidencia |
|---|---:|---|
| Capturas generadas por exe | ✅ PASS técnico | 7 PNG en `dist/Windows/NeonDistrictSandbox/Saved/Screenshots/Windows/` |
| ND_City visible, no OpenWorld | ✅ PASS | Capturas actuales muestran ciudad procedural con calles/edificios |
| Presentación final / estética | ⚠️ WARN | Sigue siendo greybox/maqueta: muchos cubos, materiales básicos, poca identidad visual |
| HUD en capturas | ⚠️ WARN | Benchmark funcional, pero capturas no demuestran HUD suficientemente |
| Vehículo/wanted/pausa visualmente claros | ⚠️ WARN | Gates lógicos pasan; capturas actuales no venden bien esos estados |
| Assets reales de personajes/vehículos/props/UI/audio | ❌ FAIL content | Content folders casi vacíos; próximo hito es asset pass completo |

Hoja de contacto actual: `docs/screenshots/current_benchmark_contact_sheet.png`.

## Riesgos abiertos

1. **Visual quality:** la maqueta funciona, pero aún no parece juego final. Necesita material/runtime asset pass.
2. **Assets:** reemplazar cubos por meshes/materiales reconocibles: humanos, vehículos, props urbanos, UI, audio.
3. **Vehicle warnings:** Chaos emite warnings de bone names; el gate lógico de enter/drive/exit pasa, pero el setup visual/físico debe endurecerse con assets reales.
4. **Enhanced Input warning:** build log muestra `UEnhancedInputLocalPlayerSubsystem ... no valid PlayerInput object`; fallback clásico está configurado y el benchmark pasa, pero conviene resolver Enhanced Input en el asset pass.

## Criterio para próxima aprobación humana

El usuario debe abrir `dist/Windows/NeonDistrictSandbox.exe` y validar manualmente:

- Menú principal.
- Carga de `ND_City`.
- Movimiento + sprint.
- Interacción.
- Pausa/reanudación.
- Misión “Entrega a Nova”.
- Entrar/salir/conducir vehículo.
- Wanted/policías.
- F5/F9 save/load.

Para considerar el juego “presentable”, además debe pasar un **Asset/Render Gate**: humanos reconocibles, vehículos reconocibles, materiales no grises, HUD legible y ambiente urbano con identidad.
