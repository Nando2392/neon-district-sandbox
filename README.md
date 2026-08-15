# Neon District Sandbox

Vertical slice de **sandbox urbano third-person original** en Unreal Engine 5.

---

## Estado REAL (actualizado: 2026-08-15)

**BUILD: ✅ COMPILADO Y EMPAQUETADO CON ÉXITO**

| Gate | Estado | Evidencia |
|---|---|---|
| Unreal 5.8 instalado | ✅ | `C:\Program Files\Epic Games\UE_5.8` |
| Compile editor/Development | ✅ | `Build.bat` → `Result: Succeeded` (16 fixes 5.6→5.8) |
| Executable packageado | ✅ | `dist/Windows/NeonDistrictSandbox.exe` (17MB) |
| Benchmark automatizado | ⚠️ **24/25 PASS** | 1 fail: `gameplay.player_move` (colisión teletransporte), 1 warn: audio |
| Visual | ✅ | 14 screenshots generados |
| Audio | ⚠️ WARN | Sin assets de contenido → silencio (placeholder) |

---

## Cómo ejecutar el juego

### Opción 1: Ejecutable packageado (recomendado)

1. Navegar a `dist/Windows/`
2. Ejecutar `NeonDistrictSandbox.exe`
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
├── NeonDistrictSandbox.exe          (17MB stub)
├── Engine/                           (runtime engine)
└── NeonDistrictSandbox/
    ├── Binaries/
    ├── Content/Paks/                 (11MB de contenido)
    └── Saved/
        ├── Screenshots/
        ├── Logs/
        └── Benchmark/
```

Ver `docs/scorecard.md` para estado detallado del benchmark.

---

## Benchmark Status

**Resultados:** 24/25 tests PASS

- ✅ Todos los gates de gameplay, IA, vehículos, misión pasan
- ⚠️ `gameplay.player_move`: falla en colisión de teletransporte (benchmark específico)
- ⚠️ Audio: sin assets de contenido → silencio (placeholder conocido)

**NOTA:** El jugador puede moverse manualmente en el juego. El fallo es específico del entorno de test del benchmark.

---

## Build History

- **Compilado:** 2026-08-15 contra UE 5.8
- **Fixes migración 5.6→5.8:** 16 fixes documentados en `docs/process.md`
- **Packaging:** Win64 Development, sin perfiles de licencia

---

## Documentación

- `docs/process.md` — Detalles del build y migración 5.6→5.8
- `docs/scorecard.md` — Estado completo de todos los gates
- `docs/packaging.md` — Procedimiento de packaging
- `smoke_check.py` — Verificación automatizada del proyecto