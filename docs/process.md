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

1. Login en Epic Launcher + instalar **UE 5.6**.
2. Abrir `NeonDistrictSandbox.uproject`, generar VS project files, compilar.
3. Crear `Content/Maps/ND_MainMenu.umap` y `ND_City.umap` (vacíos).
4. PIE → recorrer gates: gameplay → visual (screenshots) → AI → vehicle →
   audio → packaging.
5. Registrar cada fallo aquí con captura + fix.
