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
| Setup (motor + MCP) | **PASS (parcial)** | UE 5.8 instalado en `C:\Program Files\Epic Games\UE_5.8`. Unreal MCP / VibeUE siguen ausentes (no se usan). |
| Build/compile | **PASS** | `Build.bat NeonDistrictEditor Win64 Development` → `Result: Succeeded` (2026-08-15). Migración completa a APIs de 5.8. |
| PIE / Visual / AI / Vehicle / Audio / Packaging | **NOT RUN** | Requieren abrir el proyecto en el editor. Flujo de verificación en `docs/scorecard.md`. |
| Repo + docs | **PASS** | Este repo, público. |

**Camino desbloqueado**: UE 5.8 instalado y el módulo compila limpio. Falta:
abrir `NeonDistrictSandbox.uproject` en el editor → confirmar los maps vacíos →
PIE → seguir `docs/process.md`.

---

## Setup (Windows)

1. **Unreal Engine 5.8** (ya instalado — `C:\Program Files\Epic Games\UE_5.8`).
   - Requiere VS2022 + C++ workload (presente) y .NET Framework 4.8.1 SDK
     (instalado vía `winget install --id Microsoft.DotNet.Framework.DeveloperPack_4`).
2. Abrir `NeonDistrictSandbox.uproject` (botón derecho → "Generate Visual Studio
   project files" la primera vez).
3. **Los dos niveles** `Content/Maps/ND_MainMenu.umap` y `ND_City.umap` ya
   existen (creados por el commandlet `NDCreateMapsCommandlet`; vacíos, 0
   actores). Si no existen: `File > New Level > Empty Level` → guardar con esos
   nombres.
   - *No hace falta colocar nada*: el `NDWorldSubsystem` construye el distrito
     completo al entrar en cualquier nivel que no sea el menú.
4. `Ctrl+Shift+P` en el editor (o menú Tools) → *Compile* para compilar el módulo C++.
5. `Play` (PIE).

> El HUD, el menú, la pausa, los materiales neón y la ciudad son 100%
> procedurales en código: el proyecto funciona con cero assets creados a mano.
> Los únicos assets opcionales a asignar en el editor son: skeletal mesh/anim
> humano para los NPCs (`NDNPCCharacter` → `NPCVisual`) y sonidos
> (`NDAudioManager`), ambos por soft-path, sin los cuales el juego corre en
> silencio/placeholder visual pero no crashea.

---

## Controles

| Entrada | Acción |
|---|---|
| WASD / flechas | Mover / acelerar-frenar y girar (vehículo) |
| Ratón | Cámara third-person (pitch/yaw, colisión de spring arm) |
| Shift | Correr / sprint |
| Espacio | Saltar (a pie) / freno de mano (vehículo) |
| E | Interactuar (hablar, misión, pickup) |
| F | Entrar / salir de vehículo |
| ESC | Pausa / reanudar (M en pausa → menú principal) |
| F5 / F9 | Guardar / cargar rápido |

---

## Features

- **Personaje third-person**: caminar/correr/sprint/saltar, cámara follow con
  collision avoidance y lag, pasos con feedback de audio.
- **Ciudad procedural** (`ANDWorldBuilder`): 2×2 manzanas, calles con carriles
  emisivos, aceras, edificios con fachadas neón + ventanas cálidas + antenas con
  baliza, farolas, señales, kiosko, basura, hidrantes, niebla púrpura, luz de
  luna, sky atmosphere, bloom controlado. Identidad synthwave/noche, nada de
  cajas grises por defecto.
- **NPCs humanos** (`ANDNPCCharacter` + `ANDNPCAIController`): 12 civiles + 2
  policías (caps en `NDPerf`), 3+ variaciones de outfit, patrulla por puntos,
  los civiles huyen al detectar amenaza, los policías detectan → persiguen →
  pierden → bajan heat.
- **Vehículos Chaos** (`ANDVehicle`): 3 manejables (malla propia en código +
  ruedas), acelerar/frenar/girar, cámara de vehículo, entrar/salir (F), audio de
  motor por velocidad, impacto con chispas + alerta. Tráfico por spline
  (`ANDTrafficVehicle`).
- **Wanted/heat 3 niveles** (`UNDWantedSystem`): nivel 1 persecución ligera,
  nivel 2 refuerzos + sirena, nivel 3 huida total; decae con timer tras perder
  visión/distancia. Event-driven (HUD + audio suscritos).
- **Misión corta** (`NDMissionSystem`): "Entrega el paquete a Nova" — hablar con
  Mei → recoger → entregar; marcador en HUD, diálogo breve, notificaciones.
- **UI** (`UNDHUDWidget`): objetivo, wanted, prompt de interacción, estado de
  vehículo, notificaciones; pausa y menú principal procedurales (UMG en código).
- **Audio** (`UNDAudioManager`): buses Master/Music/SFX/Ambience/UI/Vehicles/
  Dialogue con volúmenes por categoría, pasos, motor, sirena, impacto, alerta;
  la intensidad musical reacciona al wanted level. Assets opcionales por soft-path.
- **FX Niagara** (`UNDVFXManager`): humo/chispas/polvo/luces con pooling y cap
  explícito (`MaxFXActors`), spawn solo por eventos.
- **Save/load** (`UNDGameInstance` + `UNDSaveGame`): ubicación + volumen + estado
  de misión/wanted en slot único; pausa/reanudar/salir a menú.

---

## Arquitectura

```
Source/NeonDistrict/
├── Core/        NDGameInstance (save/load, audio+FX managers), NDGameMode,
│                NDSaveGame, NDPerfConstants (caps y tuning)
├── Player/      NDPlayerController (Enhanced Input runtime, interacción,
│                vehículo, pausa, menú), NDCharacter (cámara+movimiento+pasos),
│                NDInteractable (interfaz)
├── Vehicle/     NDVehicle (Chaos manejable), NDTrafficVehicle (splines)
├── AI/          NDNPCCharacter (civiles/policía/misión), NDNPCAIController
│                (detectar/perseguir/pierde), NDCitySpawner (caps)
├── Systems/     NDWantedSystem, NDMissionSystem, NDWorldBuilder (ciudad
│                procedural), NDWorldSubsystem (auto-build)
├── Audio/       NDAudioManager (buses, sirenas, motor, pasos)
├── FX/          NDVFXManager (Niagara pooling)
└── UI/          NDHUDWidget, NDPauseWidget, NDMainMenuWidget
```

Principios: **event-driven** (nada spawnea desde Tick), **caps explícitos** en
`NDPerfConstants.h`, **cero dependencia de assets** (todo runtime-created con
soft paths opcionales), sistemas desacoplados por subsistemas de
GameInstance/World.

---

## Build / Packaging

Ver `docs/packaging.md`. Resumen: en el editor `File > Package Project >
Windows > Win64`, o CLI:
`RunUAT.bat BuildCookRun -project=... -platform=Win64 -stage -pak -cook`
El ejecutable queda en `dist/Windows/NeonDistrictSandbox.exe` (fuera de git).

---

## Documentación del proceso

- `docs/process.md` — research, skills usadas, decisiones, fallos, escalado.
- `docs/scorecard.md` — tabla de criterios PASS/FAIL con evidencia.
- `docs/screenshots/` — capturas de gates visuales (pendientes de motor).
- `docs/packaging.md` — cómo se genera el ejecutable.

---

## ¿Qué NO está hecho todavía?

Nada se declara terminado: los gates mecánicos requieren motor. El código es la
entrega actual; la verificación ejecutable (compile + PIE + visual + AI +
vehicle + audio + packaging + screenshots + human approval) es el siguiente
paso con motor instalado, y cada fallo se registrará en `docs/process.md`.
