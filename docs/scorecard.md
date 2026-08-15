# Scorecard — Neon District Sandbox

Criterios del benchmark con estado PASS/FAIL/NOT RUN y evidencia. Estado a
2026-08-15: **sin motor instalado** — los gates ejecutables están NOT RUN hasta
que haya UE 5.6; el código del repo es la base engine-ready sobre la que se
ejecutan.

Leyenda: ✅ PASS · ❌ FAIL · ⏸ NOT RUN (requiere motor) · 🔶 PARCIAL

---

## Setup gates

| Criterio | Estado | Evidencia |
|---|---|---|
| 1. Unreal Editor disponible | ❌ FAIL | No instalado (ni launcher) al inicio; launcher instalado vía winget en sesión; motor pendiente de login+descarga |
| 2. Proyecto abre | ⏸ NOT RUN | Requiere motor; `.uproject` + targets + configs listos |
| 3. Unreal MCP responde | ❌ FAIL | Sin editor no hay plugin/MCP; documentado en `docs/process.md` |
| 4. Toolsets listados | ❌ FAIL | Idem |
| 5. VibeUE instalado | ❌ FAIL | No existe `AgentCity/Plugins/VibeUE`; se continuó con tooling estándar |

## Build/compile gate

| Criterio | Estado | Evidencia |
|---|---|---|
| Proyecto abre | ⏸ | — |
| Compile sin errores | ⏸ | C++ escrito (23 archivos); verificación estática de símbolos hecha (grep cruzado); compilación real pendiente |
| Assets referenciados existen | 🔶 | Soft-paths: skeletal mesh NPC y sonidos opcionales; meshes del motor (`/Engine/BasicShapes/*`) estándar |
| Sin warnings críticos ignorados | ⏸ | — |

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
| Screenshots (menú, calle, jugador, NPCs, vehículo, persecución, misión) | ⏸ | `docs/screenshots/` vacío hasta tener motor |
| Escena no-default-template | ⏸ | Diseño: world builder procedural neón (paleta, niebla, bloom) — a validar en motor |
| Sin predominio de cubos/cápsulas | ⏸ | Edificios con fachadas neón + ventanas + antenas; props variados |
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

Ningún gate ejecutable se declara pasado sin motor. Cuando el motor esté
instalado, se ejecutará el checklist en orden y cada fila cambiará a PASS/FAIL
con captura (screenshots en `docs/screenshots/`, logs y fixes en
`docs/process.md`).
