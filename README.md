# Neon District Sandbox

Vertical slice de **sandbox urbano third-person original** en Unreal Engine 5.8.

## Estado real — 2026-08-15

**✅ Maqueta 3D jugable empaquetada.**
**✅ Benchmark automatizado en exe standalone: 25/25 PASS.**
**⚠️ Visual/content todavía en fase greybox/placeholder.**

| Gate | Estado | Evidencia |
|---|---:|---|
| UE 5.8 instalado | ✅ | `C:\Program Files\Epic Games\UE_5.8` |
| BuildCookRun | ✅ | `BUILD SUCCESSFUL`, ExitCode=0 |
| Ejecutable Windows | ✅ | `dist/Windows/NeonDistrictSandbox.exe` |
| Runtime map | ✅ | Log: `LoadMap: /Game/Maps/ND_City` |
| GameMode | ✅ | Log: `Game class is 'NDGameMode'` |
| Benchmark gameplay | ✅ | `=== RESULT: 25 passed, 0 failed ===` |
| Screenshots | ✅ técnico | 7 PNG generados en `Saved/Screenshots/Windows/` |
| Visual final | ⚠️ | Ciudad procedural visible, pero aún greybox/cubos/materiales básicos |
| Asset pass | ❌ pendiente | Personajes, vehículos, props, UI y audio necesitan assets reales |

## Cómo ejecutar el juego

1. Ir a:
   ```text
   C:\Users\fjmn2\Dev\neon-district-sandbox\dist\Windows
   ```
2. Ejecutar:
   ```text
   NeonDistrictSandbox.exe
   ```
3. No hace falta instalar ni abrir Unreal para jugar el build empaquetado.

## Controles

| Entrada | Acción |
|---|---|
| WASD / flechas | Mover / acelerar-frenar y girar en vehículo |
| Ratón | Cámara third-person |
| Shift | Sprint |
| Espacio | Saltar / freno de mano |
| E | Interactuar |
| F | Entrar / salir de vehículo |
| ESC | Pausa / reanudar |
| F5 | Guardar rápido |
| F9 | Cargar rápido |

## Sistemas implementados

- Personaje third-person con cámara.
- Ciudad procedural `ANDWorldBuilder` en `ND_City`.
- NPCs civiles y policías.
- Misión corta: “Entrega a Nova”.
- Wanted/heat con policía.
- Vehículos manejables.
- Pausa.
- Save/load.
- Benchmark runner empaquetado con reporte y screenshots.

## Arquitectura

```text
Source/NeonDistrict/
├── Core/        NDGameInstance, NDGameMode, NDSaveGame, NDPerfConstants
├── Player/      NDPlayerController, NDCharacter, NDInteractable
├── Vehicle/     NDVehicle, NDTrafficVehicle
├── AI/          NDNPCCharacter, NDNPCAIController, NDCitySpawner
├── Systems/     NDWantedSystem, NDMissionSystem, NDWorldBuilder
├── Audio/       NDAudioManager
├── FX/          NDVFXManager
└── UI/          NDHUDWidget, NDPauseWidget, NDMainMenuWidget
```

Descriptor: `NeonDistrictSandbox.uproject`
Módulo C++: `NeonDistrict`
Targets: `NeonDistrictSandbox.Target.cs`, `NeonDistrictSandboxEditor.Target.cs`

## Benchmark final

Resultado guardado en:

```text
dist/Windows/NeonDistrictSandbox/Saved/Benchmark/NDBenchmarkResult.txt
```

Resumen:

```text
Map: /Game/Maps/ND_City
[PASS] mission.accept
[PASS] mission.complete
[PASS] wanted.level2
[PASS] vehicle.enter
[PASS] vehicle.drive_input
[PASS] vehicle.exit
[PASS] controls.pause
[PASS] save.write
[PASS] save.load
[PASS] gameplay.player_move — moved 50.4 cm
=== RESULT: 25 passed, 0 failed ===
```

## Próximo hito

**Asset/Render Pass.** La maqueta ya existe y funciona; ahora hay que renderizarla como juego:

- Materiales runtime coloreados/emisivos.
- Meshes/props urbanos.
- Humanos reconocibles.
- Vehículos reconocibles.
- HUD legible y capturas que muestren el estado real.
- Audio assets o synth verificable.

Ver `docs/scorecard.md`, `docs/packaging.md` y la wiki Obsidian del proyecto para el prompt de continuación.
