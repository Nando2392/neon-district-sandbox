# Process Log — Neon District Sandbox

Registro cronológico de research, skills, arquitectura, decisiones, fallos,
fixes, escalado y performance. Cada fallo documenta: fecha/hora, gate fallido,
captura/log, hipótesis, research, skill usada, fix y resultado.

---

## 2026-08-15 — Sesión inicial (benchmark kickoff)

### Gate: Setup (motor + MCP + VibeUE) — **FAIL**

**Fallo**: Unreal Engine no instalado, Epic Games Launcher ausente, Unreal MCP no
responde, VibeUE no presente.

**Log/evidencia**:
- `ls C:\Program Files\Epic Games` → vacío / no existe.
- `where UnrealEditor` → no encontrado.
- Sin procesos `UnrealEditor` / `EpicGamesLauncher` en ejecución.
- Hardware apto: RTX 4070 Laptop 8GB, 32GB RAM, i7-13650HX, 555GB libres,
  Visual Studio 2022 + Build Tools C++ instalados (requisito UE C++).

**Hipótesis**: máquina nueva para UE; el benchmark asume motor presente. El
bloqueo no es de capacidad, es de instalación.

**Research breve**:
- UE 5.6 requiere VS2022 17.x + Windows SDK y ~40-90 GB de instalación.
- La instalación del motor exige login en el Epic Games Launcher (paso humano:
  cuenta Epic) — no automatizable sin credenciales.
- Unreal MCP (plugin `unreal-mcp` / VibeUE) solo existe dentro de un editor en
  marcha; sin motor no hay MCP que descubrir.

**Skill usada**: `unreal-cpp-gameplay`, `unreal-blueprints`, `unreal-packaging`
(leídas; documentan patrón del código), `reference-images` (paleta/ambiente).

**Fix aplicado**:
1. Instalado el **Epic Games Launcher** vía winget (automatizable, sin login):
   `winget install --id EpicGames.EpicGamesLauncher` → exit 0, "Successfully installed".
2. El código del juego se escribió **engine-ready** en este repo (todo el C++),
   de modo que con motor + 2 niveles vacíos creados a mano, la slice entera se
   construye sola (world builder procedural + spawner).

**Resultado nuevo**: Setup gate sigue FAIL (motor no instalado), pero el camino
queda reducido a: login Epic → instalar UE 5.6 → abrir proyecto → crear
`ND_MainMenu` y `ND_City` vacíos → PIE. Paso humano mínimo y documentado.

**Escalado**: no hizo falta escalar a otros modelos para el bloqueo: es un paso
de instalación humana, no un fallo de razonamiento. Se documenta por si el
siguiente intento necesita auto-research del motor.

---

## 2026-08-15 — Sesión de motor: UE 5.8 instalado + compile gate PASSED

### Gate: Setup (motor) — **PASS** (parcial: MCP/VibeUE siguen ausentes)

- Epic Games Launcher ya instalado (winget) y **UE 5.8 instalado** en
  `C:\Program Files\Epic Games\UE_5.8` (login Epic manual; `UnrealEditor.exe`
  presente; `ToolComplete.txt` confirma instalación).
- Decisión de motor: **5.8 en vez de 5.6** (5.6 no estaba disponible; 5.8 es la
  versión instalada). `EngineAssociation` actualizado a `"5.8"`.
- Toolchain: MSVC 14.44 (VS2022 BuildTools) + **.NET Framework 4.8.1 SDK**
  instalado vía `winget install --id Microsoft.DotNet.Framework.DeveloperPack_4
  --silent` (lo exige SwarmInterface de UBT).

### Gate: Compile (editor, Development) — **PASS**

```text
Result: Succeeded
Output binary: ...\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe
```

### Migración del código a UE 5.8 (fallos reales + fixes)

| # | Error (UBT/MSVC) | Causa raíz | Fix |
|---|---|---|---|
| 1 | `NeonDistrictEditor modifies the values of properties ... not allowed` | Build environment compartido con `UnrealEditor`; `BuildEnvironment = Unique` no permitido con motor instalado | `bOverrideBuildEnvironment = true` en ambos `*Target.cs` |
| 2 | `Plugin 'ChaosVehicles' not found` | En 5.8 el plugin se llama `ChaosVehiclesPlugin` | Renombrado en `.uproject` (módulo C++ sigue `ChaosVehicles`) |
| 3 | `Could not locate the .NET Framework SDK` (SwarmInterface) | Falta developer pack | winget `Microsoft.DotNet.Framework.DeveloperPack_4` + verificación reg |
| 4 | UHT: clase UObject `NDMissionSystem` sin prefijo U | UHT exige `U` en UObject | Renombrada a `UNDMissionSystem` (+ usos) |
| 5 | UHT: interfaz `UNDInteractable` ↔ `INDIInteractable` (nombre UObject no coincide) | Naming | Renombrada a `UNDIInteractable` |
| 6 | `Instigator`/`Pawn`/`Character`/`bSprinting`/`bInVehicle` shadowing (C4458 = error) | Warnings como errores | Variables renombradas (`PlayerController`, `P`, `NPC`, `bNewSprinting`, `bVehicleActive`) |
| 7 | Includes `"Player/X.h"` no encontrados | UBT solo añade `Source/` al include path | `PrivateIncludePaths.Add(ModuleDirectory)` en `NeonDistrict.Build.cs` |
| 8 | Chaos API: `FChaosWheelSetup::LocalLocation`, `EngineSetup.MOI`, clase base `UChaosVehicleMovementComponent` | API 5.8 | `AdditionalOffset`, `EngineRevUpMOI`, `UChaosWheeledVehicleMovementComponent` |
| 9 | `UWorld::SetPaused` no existe | API | `UGameplayStatics::SetGamePaused(World, b)` |
| 10 | `FSlateFontInfo(nullptr, N)` C2440 (9 sitios) | Constructor con nullptr ambigüo en 5.8 | `FCoreStyle::GetDefaultFontStyle("Regular"/"Bold", N)` + include `Styling/CoreStyle.h` |
| 11 | `FEditorFileUtils::NewBlankMap` no existe | API 5.8 | `UEditorLoadingAndSavingUtils::NewBlankMap/SaveMap` (commandlet `NDCreateMapsCommandlet`) |
| 12 | `Engine/SkyAtmosphere.h` no existe | API | Solo `Components/SkyAtmosphereComponent.h` (la clase `ASkyAtmosphere` está ahí) |
| 13 | `GetRandomReachablePointInRadius` pide `FNavLocation&` | API | `FNavLocation` en vez de `FVector` (2 sitios) |
| 14 | `AExponentialHeightFog::GetFogComponent()` / `USkyAtmosphereComponent::SetHeight()` no existen | API 5.8 | `GetComponent()` y `SetAtmosphereHeight()` |
| 15 | `UMaterial::BaseColor/EmissiveColor/ShadingModel` inaccesibles | En 5.8 el grafo del material es editor-only | `UMaterialEditingLibrary::ConnectMaterialProperty` + `SetShadingModel`, bajo `#if WITH_EDITOR` con fallback runtime a `/Engine/BasicShapes/BasicShapeMaterial` (documentado) |
| 16 | `UButton::SetMinDesiredWidth` no existe | API UMG 5.8 | Wrapper `USizeBox::SetMinDesiredWidth` |

**Resultado**: compile limpio. Quedan warnings de Upgrade (BuildSettings V5→V7,
IncludeOrderVersion) — no bloqueantes.

---

## Skills cargadas / disponibles (obligatorias)

Leídas desde `C:\Users\fjmn2\Dev\aaabench-src\.claude\skills` (20 skills):
unreal-cpp-gameplay, unreal-blueprints, unreal-behavior-trees,
unreal-enhanced-input, unreal-niagara, unreal-packaging, game-ai, game-feel,
level-design, camera-systems, performance-optimization, physics-tuning,
procedural-gen, audio-design, game-ui-ux, shader-programming, input-systems,
dialogue-systems, save-systems, reference-images.

Red-team-adventures: mismo set (sin skills UE adicionales distintivas).

Hermes personal skills: `game-architecture` no disponible (podada), no se usó.

**VibeUE**: no instalado (no existe `AgentCity/Plugins/VibeUE`). Documentado;
no se inventó su API. Si aparece, el siguiente paso es `ListSkills`/`GetSkills`
por Unreal MCP antes de usarlo.

**Unreal MCP**: no disponible (sin motor). Documentado como bloqueo del gate de
setup; no se finge edición visual.

---

## Arquitectura y decisiones

- **Módulo único C++ `NeonDistrict`** (Runtime), plugins EnhancedInput,
  ChaosVehicles, Niagara habilitados en `.uproject`.
- **Todo runtime-created**: HUD, menú, pausa, materiales neón, ciudad — cero
  dependencia de assets binarios. Los `.umap` que faltan se crean vacíos y el
  `NDWorldSubsystem` construye el distrito al arrancar cualquier nivel no-menú.
- **Sistemas como subsistemas**: `UNDWantedSystem` y `NDMissionSystem` viven en
  GameInstance → sobreviven a transiciones de nivel y se serializan en save.
- **Event-driven**: HUD/audio se suscriben a delegates; nada spawnea desde Tick;
  caps explícitos en `NDPerfConstants.h` (NPCs ≤14, policía ≤3, tráfico ≤4,
  manejables ≤3, FX ≤24 pool).
- **NPCs humanos**: `ACharacter` + skeletal mesh asignable por soft-path en
  editor (MetaHuman/Manny/Quinn/Mixamo). Sin mesh asignado, el NPC no crashea
  (gate visual marcaría falta de mesh hasta que se asigne — documentado).
- **Vehículo Chaos** con ruedas creadas en código (`EnsureWheels`) si el BP no
  las trae; entrada/salida posee/desposee y oculta al personaje.
- **Cámara**: spring arm con colisión + lag + límites de pitch; cámara propia en
  vehículo.
- **Misión corta**: "Entrega el paquete a Nova" — Mei (misión giver) → paquete →
  Nova. 4 stages, marcador por subsistema, HUD suscrito.
- **Audio**: `UNDAudioManager` con volúmenes por categoría (Master/Music/SFX/
  Ambience/UI/Vehicles/Dialogue), assets opcionales por soft-path, wanted≥2
  activa sirena y tensión.
- **FX Niagara** con pooling round-robin y cooldown por tipo.

---

## Fallos y fixes (solo C++ — antes de compilación)

| # | Fecha | Gate | Log/captura | Hipótesis | Fix | Resultado |
|---|---|---|---|---|---|---|
| 1 | 2026-08-15 | Build (pre) | `NDGameInstance` sin `GetVFXManager()`, `NDVehicle::NotifyHit` lo llama | Referencia cruzada incompleta al escribir por tandas | Añadido `VFXManager` + getter + Init/Shutdown | Símbolo consistente (grep) |
| 2 | 2026-08-15 | Build (pre) | `NDPauseWidget` llama `HandlePauseFromWidget()` no declarado | Mismo patrón | Declarado en header + implementación que reusa `HandlePause()` | Consistente |
| 3 | 2026-08-15 | Build (pre) | Patch de `HandlePause()` rompió if/else | Patch mal aplicado | Re-lectura y restauración de la estructura + `HandlePauseFromWidget` | Consistente |
| 4 | 2026-08-15 | Build (pre) | `AddBox` attach invertido (`GetRootComponent()->AttachToComponent(Comp)`) | Error de orden en build procedural | `SetupAttachment(SceneRoot)` + `RegisterComponent` + relative transforms | Corregido |
| 5 | 2026-08-15 | Build (pre) | `AddDynamic` con puntero a función genérico (macro exige literal) | API UMG | Botones bindeados con `AddDynamic(this, &Clase::Metodo)` literal; `MakeButton` solo construye | Corregido |
| 6 | 2026-08-15 | Build (pre) | Menú necesita nivel `ND_MainMenu` sin pawn | Flujo de mapa | `NDGameMode::BeginPlay` → `DefaultPawnClass=nullptr` si nivel es menú; `ANDPlayerController::BeginPlay` muestra menú procedural en vez de HUD | Corregido |

---

## Performance (objetivo 60 FPS, mínimo 30 estables)

Medición pendiente (requiere motor). Diseño para el objetivo:
- Caps explícitos (ver `NDPerfConstants.h`) — nunca spawn ilimitado.
- FX con pooling (24 actores máx., reciclado round-robin).
- Ciudad estática construida una vez en `BeginPlay`; sin trabajo por frame en el
  builder.
- NPCs con tick condicional (persecución solo cuando relevante).
- Materiales neón con emisivo + bloom controlado (BloomThreshold 1.1) — sin
  overdraw de iluminación dinámica masiva (las luces de farola/baliza son
  contadas).
- `stat fps` / `stat unit` a registrar en la sesión de motor.

---

## Siguiente paso

1. Abrir `NeonDistrictSandbox.uproject` con UE 5.8 (doble clic / `-project`).
   Si el editor pide regenerar, aceptar.
2. Confirmar `Content/Maps/ND_MainMenu.umap` y `ND_City.umap` vacíos (creados
   por el commandlet; si no existen, correr el commandlet o crearlos vacíos).
3. PIE en `ND_City` → recorrer gates: gameplay → visual (screenshots en
   `docs/screenshots/`) → AI → vehicle → audio → packaging (ver `docs/scorecard.md`).
4. Registrar cada fallo aquí con captura + fix.
5. (Opcional) Asignar skeletal mesh humano al NPC en el editor para el gate
   visual; asignar assets de audio si se quiere el gate de audio completo.
