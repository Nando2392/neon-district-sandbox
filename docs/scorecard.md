# Scorecard — Neon District Sandbox

Criterios del benchmark con estado PASS/FAIL/NOT RUN y evidencia. Estado a
2026-08-15: **UE 5.8 instalado y compile gate PASSED** — los gates de PIE,
visual, AI, vehicle, audio y packaging siguen NOT RUN hasta abrir el proyecto
en el editor.

Leyenda: ✅ PASS · ❌ FAIL · ⏸ NOT RUN (requiere motor/editor) · 🔶 PARCIAL

---

## Setup gates

| Criterio | Estado | Evidencia |
|---|---|---|
| 1. Unreal Editor disponible | ✅ PASS | `C:\Program Files\Epic Games\UE_5.8` + `UnrealEditor.exe` presente (2026-08-15) |
| 2. Proyecto abre | 🔶 PARCIAL | `.uproject` asociado a 5.8 (`EngineAssociation: "5.8"`); apertura en editor pendiente |
| 3. Unreal MCP responde | ❌ FAIL | Sin plugin/MCP; documentado en `docs/process.md` — se trabaja con tooling estándar |
| 4. Toolsets listados | ❌ FAIL | Idem |
| 5. VibeUE instalado | ❌ FAIL | No existe; se continuó con tooling estándar |

## Build/compile gate

| Criterio | Estado | Evidencia |
|---|---|---|
| Proyecto abre | 🔶 | Asociado a 5.8; apertura pendiente |
| Compile sin errores | ✅ PASS | `Build.bat NeonDistrictEditor Win64 Development` → `Result: Succeeded` (2026-08-15). Migración completa a APIs de 5.8 (ver `docs/process.md` §Migración) |
| Assets referenciados existen | 🔶 | Soft-paths: skeletal mesh NPC y sonidos opcionales; meshes del motor (`/Engine/BasicShapes/*`) estándar |
| Sin warnings críticos ignorados | 🔶 | Warnings de Upgrade (BuildSettings V5→V7, IncludeOrder) presentes pero no bloqueantes; `bOverrideBuildEnvironment` evita la infracción de entorno compartido |

## PIE gameplay gate

| Acción | Estado |
|---|---|
| Iniciar PIE | ⏸ |
| Mover personaje | ⏸ |
| Pausa/reanudar | ⏸ |
| Hablar/interactuar con NPC | ⏸ |
| Iniciar misión | ⏸ |
| Entrar vehículo | ⏸ |
| Conducir | ⏸ |
| Salir vehículo | ⏸ |
| Provocar wanted | ⏸ |
| Policía persigue | ⏸ |
| Evadir hasta bajar wanted | ⏸ |
| Completar misión | ⏸ |

## Visual gate

| Criterio | Estado | Evidencia |
|---|---|---|
| Screenshots (menú, calle, jugador, NPCs, vehículo, persecución, misión) | ⏸ | `docs/screenshots/` pendiente de PIE |
| Escena no-default-template | 🔶 | Diseño: world builder procedural neón — a validar en PIE |
| Sin predominio de cubos/cápsulas | 🔶 | Edificios con fachadas neón + ventanas + antenas (código); validación visual pendiente |
| Humanos reconocibles | 🔶 | Requiere asignar skeletal mesh humano en editor (soft-path); sin él, mesh default del motor |
| Ambiente urbano claro | ⏸ | — |
| UI legible / cámara encuadrada / lighting intencional | ⏸ | — |

## AI gate

| Criterio | Estado |
|---|---|
| ≥5 NPCs con comportamiento activo | ⏸ (12 civiles + 2 policías por diseño, caps en `NDPerfConstants.h`) |
| Policía detecta jugador | ⏸ |
| Persecución funciona | ⏸ |
| Pérdida reduce wanted | ⏸ |
| No se quedan atascados | ⏸ |

## Vehicle gate

| Criterio | Estado |
|---|---|
| Entrar/salir | ⏸ |
| Acelerar/frenar/girar | ⏸ |
| Cámara de vehículo | ⏸ |
| Colisión básica | ⏸ |
| No explota al primer contacto | ⏸ (impacto gated 0.6s, sin física rota) |

## Audio gate

| Criterio | Estado |
|---|---|
| Ambiente urbano audible | 🔶 — assets de sonido opcionales; sin ellos, silencio (documentado) |
| Pasos con feedback | 🔶 |
| Vehículo con audio | 🔶 |
| Wanted cambia audio/sirena | 🔶 |
| Mute/pause funcionan | ⏸ |

## Packaging gate

| Criterio | Estado |
|---|---|
| Ejecutable Windows local | ⏸ — flujo en `docs/packaging.md` |
| Instrucciones de ejecución | ✅ — README + packaging.md |
| Log + corrección si falla | ⏸ |

## Human approval simulation

| Pregunta | Estado |
|---|---|
| ¿Parece un juego, no una escena técnica? | ⏸ |
| ¿Los humanos parecen humanos? | ⏸ |
| ¿La ciudad tiene identidad? | ⏸ |
| ¿El vehículo se siente conducible? | ⏸ |
| ¿La persecución genera situación jugable? | ⏸ |
| ¿Hay placeholders? | ⏸ |
| ¿El jugador entiende qué hacer? | ⏸ |

---

## Cierre

Ningún gate ejecutable se declara pasado sin evidencia en el editor. Con el
compile PASSED (5.8), el siguiente paso es abrir el proyecto en el editor,
confirmar los maps vacíos y ejecutar el checklist en orden; cada fila cambiará
a PASS/FAIL con captura (screenshots en `docs/screenshots/`, logs y fixes en
`docs/process.md`).
