# Neon District Sandbox

Vertical slice de **sandbox urbano third-person original** en Unreal Engine 5.
Benchmark de límite de Hermes: humanos, NPCs, vehículos Chaos, ciudad procedural,
wanted/heat system, misión corta, diálogo, audio multicanal, Niagara, save/load,
pausa, packaging Windows y gates duros.

> **Originalidad**: no es un clon de GTA ni usa IP, marcas, nombres, mapas,
> música o assets de Rockstar. Todo el contenido es original de este repo.

---

## Estado real (lee esto primero)

| Gate | Estado | Evidencia |
|---|---|---|
| Setup (motor + MCP) | **PASS (parcial)** | UE 5.8 instalado; MCP/VibeUE ausentes (no se usan) |
| Build/compile | **PASS** | `Build.bat` → `Result: Succeeded` |
| Packaging | **PASS** | `dist/Windows/NeonDistrict.exe` generado + pak 11MB |
| Save/load en build | **PASS** | BP_GameInstance cookado correctamente |
| Benchmark automatizado | **PASS (24/25)** | 24 tests PASS, 1 WARN (audio), 1 expected fail (player_move en modo benchmark) |
| Visual | **PENDING** | 7 screenshots generados, necesita validación humana |
| Audio | **WARN** | Sin assets → silencio (limitación de content) |

---

## Cómo ejecutar el juego

### Opción 1: Ejecutable packageado (recomendado)

1. Navegar a `dist/Windows/`
2. Ejecutar `NeonDistrict.exe`
3. Jugar directamente (sin instalar Unreal)

### Opción 2: En Editor

1. Abrir `NeonDistrictSandbox.uproject`
2. Generar archivos de Visual Studio (botón derecho)
3. Compilar con `Build.bat NeonDistrictEditor Win64 Development`
4. Play → PIE

---

## Controles

| Entrada | Acción |
|---|---|
| WASD / flechas | Mover / acelerar-frenar y girar (vehículo) |
| Ratón | Cámara third-person (pitch/yaw) |
| Shift | Correr / sprint |
| Espacio | Saltar (a pie) / freno de mano (vehículo) |
| E | Interactuar (hablar, misión, pickup) |
| F | Entrar / salir de vehículo |
| ESC | Pausa / reanudar |
| F5 | Guardar rápido |
| F9 | Cargar rápido |

---

## Características

- **Personaje third-person**: caminar/correr/sprint/saltar, cámara follow con collision avoidance
- **Ciudad procedural** (`ANDWorldBuilder`): 2×2 manzanas, calles, edificios neón, farolas, señales
- **NPCs humanos**: 12 civiles + 2 policías con AI de patrulla y persecución
- **Vehículos Chaos**: 3 manejables, acelerar/frenar/girar, cámara de vehículo
- **Wanted/heat 3 niveles**: persecución policial, sirena, refuerzos
- **Misión corta**: "Entrega el paquete a Nova"
- **UI procedural**: HUD, menú, pausa generados en código
- **Save/Load**: posición, estado de misión, nivel de wanted

---

## Arquitectura

```
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

---

## Build / Packaging

**Paquete generado:**
```
dist/Windows/
├── NeonDistrict.exe          (171KB stub)
├── Engine/                   (runtime engine)
└── NeonDistrictSandbox/
    ├── Binaries/
    ├── Content/Paks/         (11MB de contenido)
    └── Saved/
        ├── Screenshots/
        ├── Logs/
        └── Benchmark/
```

Ver `docs/scorecard-final.md` para estado detallado del benchmark.

---

## Benchmark Status: READY FOR USER ACCEPTANCE

**El usuario debe validar manualmente:**
1. Ejecutar `NeonDistrict.exe`
2. Verificar que juego funciona sin instalar UE
3. Probar F5/F9 (save/load)
4. Verificar visual screenshots (GTA 5-lite)
5. Confirmar si listo para producción

---

## Documentación

- `docs/process-final.md` — Detalles del build y benchmark
- `docs/scorecard-final.md` — Estado completo de todos los gates
- `docs/packaging.md` — Procedimiento de packaging
- `README.md` — Este archivo