# Asset/Render Pass — Research y Decisiones (Fase 2)

**Repo:** C:\Users\fjmn2\Dev\neon-district-sandbox
**Engine:** Unreal Engine 5.8 (`C:\Program Files\Epic Games\UE_5.8`)
**Fecha:** 2026-08-15/16
**Objetivo:** Que la maqueta deje de percibirse como greybox y se vea como un juego urbano
moderno de mundo abierto (intención GTA 5), **sin copiar IP, marcas, mapas, personajes,
vehículos, armas ni assets propietarios**, y sin assets externos que rompan el repo público
(no hay login Fab, no hay Git LFS configurado).

---

## 1. Decisión estratégica

**No usar assets descargados de Fab/Quixel/Megascans/City Sample como base de repo.**

Motivos (verificado en campo):
- Requieren login de Epic (Fab/Megascans Plugin/Bridge).
- Los paquetes son grandes (GBs) y no aptos para un repo público sin Git LFS.
- Términos de Epic/Fab: no redistribuir "source assets" en repositorios públicos.
- MetaHuman completo requiere assets descargados + es inviable en repo público.

**Alternativa elegida:** generación propia/procedural + assets Engine incluidos + candidatos CC0 auditables
cuando aporten una mejora clara y puedan redistribuirse en GitHub público.

## 2. Diagnóstico de por qué se ve greybox (evidencia en código)

| Síntoma | Causa raíz |
|---|---|
| Humanos = stick figures | Cubos/esferas BasicShapes con tintes planos; cabeza esfera sin cara |
| Sin sombras en personajes | Históricamente `bCastDynamicShadow = false` en partes del personaje |
| Sin luz ambiente | BuildAtmosphere dependía de moon/fog/postprocess, faltaba balance nocturno |
| Materiales planos | BasicShapeMaterial con color plano; grafo procedural editor-only no sirve en packaged |
| Mundo monótono | AddBox usa Cube mesh + MID; falta variación de materiales/props |

## 3. Recursos disponibles verificados (sin login, runtime-safe)

| Recurso | Ruta | Licencia | Uso |
|---|---|---|---|
| `EmissiveMeshMaterial` | `/Engine/EngineMaterials/EmissiveMeshMaterial` | Engine EULA | Neón emisivo real en packaged builds |
| `BasicShapeMaterial` | `/Engine/BasicShapes/BasicShapeMaterial` | Engine EULA | Material base con parámetros de color/emissive |
| Cubo/Cilindro/Esfera | `/Engine/BasicShapes/*` | Engine EULA | Geometría base para props/vehículos/ciudad/armas temporales |
| TutorialCharacter | `Engine/Content/Tutorial/SubEditors/TutorialAssets/Character/` | Engine EULA | Humanoide tutorial referenciable/cookable |
| Kenney Blaster Kit | `https://kenney.nl/assets/blaster-kit` | Creative Commons CC0 | Candidato seguro para reemplazar el mesh procedural de arma; 3D, 40 files, sin marcas reales |

## 4. Tabla placeholder → replacement

| Placeholder actual | Replacement | Fuente | Ruta | Evidencia |
|---|---|---|---|---|
| Humano cubos/esferas stick | Humanoide con proporciones + TutorialTPP/proxy mejorado | Engine/tutorial + C++ | `Player/NDCharacter.cpp`, `AI/NDNPCCharacter.cpp` | `player_visible.png`, `npc_interaction_mei.png` |
| Sin sombras personaje | dynamic/contact shadows | C++ | ídem | Sombra visible en packaged |
| Neón no-op en packaged | `EmissiveMeshMaterial` MID | Engine Materials | `Systems/NDWorldBuilder.cpp` | `city_street.png` |
| Fachadas planas | Ventanas físicas/mullions/spandrels | C++ procedural | `Systems/NDWorldBuilder.cpp` | fachadas con luz en screenshot |
| Supercar caja | Mesh visual importado/A-B con material slots | Blender/Khronos review CC-BY | `Vehicle/NDVehicle.cpp` | `vehicle_driving.png` |
| Asfalto plano | Materiales generated hard-referenced/cooked | Generado local | `M_NDAsphalt`, `M_NDSidewalk` | `city_street.png` |
| Sin armas | Blaster urbano usable: pickup, equip, disparo, daño, proyectil físico | C++ procedural ahora; Kenney CC0 como reemplazo visual recomendado | `Source/NeonDistrict/Combat/*`, `Player/*`, `Benchmark/*` | benchmark packaged `weapon.*` gates |

## 5. Armas usables tipo mundo abierto — decisión 2026-08-16

El objetivo no es meter props decorativos: el arma debe poder **usarse** y tener comportamiento físico/creíble.

### Implementado en esta iteración

- `ANDWeaponPickup`: pickup interactuable en la calle (`E`) que equipa el arma.
- `ANDPlayerController`: mouse izquierdo dispara; HUD muestra `Blaster urbano: ammo`.
- `ANDCharacter`: arma visible en la mano cuando está equipada.
- `ANDWeaponProjectile`: proyectil físico con colisión, velocidad, impulso y daño.
- `ANDNPCCharacter`: salud básica y `TakeDamage()` para validar impacto.
- Benchmark packaged:
  - `weapon.pickup`
  - `weapon.equip`
  - `weapon.fire`
  - `weapon.npc_damage`

### Física/gameplay

- El disparo usa primero trace de `Pawn` para respuesta inmediata contra NPCs visibles.
- Si no golpea pawn, hace fallback a geometría `ECC_Visibility`.
- Además spawnea un proyectil físico visible (`ANDWeaponProjectile`) con colisión e impulso.
- El benchmark final verificó daño inmediato `health 100.0 -> 66.0`; el log también mostró segundo impacto físico del proyectil `health=32.0`.

### Assets candidatos

| Fuente | Licencia | Riesgo | Decisión |
|---|---|---|---|
| Kenney Blaster Kit | CC0 | Bajo; no requiere atribución, estilo ficticio/no realista | Recomendado para reemplazar el mesh procedural cuando se descargue/audite/importe |
| Fab/Marketplace armas | Variable/Epic terms | Alto para repo público; login y redistribución dudosa | No integrar sin decisión explícita |
| Modelos de armas reales/marcadas | Variable + trade dress | Alto | Evitar; usar armas ficticias/originales |

## 6. Riesgos y mitigaciones

| Riesgo | Mitigación |
|---|---|
| Assets externos no redistribuibles | Preferir C++/Blender propio o CC0 explícito |
| Armas parecidas a modelos/marcas reales | Mantener estética ficticia/blaster, sin logos ni nombres reales |
| Disparo no determinista en benchmark | Trace de Pawn + benchmark `FireWeaponFrom()` con origen/dirección explícitos |
| Proyectil físico tarda frames en impactar | Hitscan valida gameplay inmediato; proyectil valida feedback físico/log |
| Repo público sin LFS | No meter packs grandes sin tamaño/licencia auditados |

---

## 7. Investigación: cómo se produce un coche para un juego en tiempo real

Los vehículos de producción separan el **arte** del sistema de conducción. Unreal documenta
un vehículo como mesh de arte, physics asset, animación/Blueprint y wheel blueprints; aquí el
equivalente seguro es conservar `BodyMesh` como collider/controlador Chaos y montar el arte en
`AuthoredBodyMesh` con `NoCollision`. Esto evita que un replacement visual cambie enter/drive/
exit o las pruebas de ruedas.

| Etapa | Práctica de producción | Decisión Neon District |
|---|---|---|
| Diseño | Proporciones y superficies originales; no copiar planos, badges ni trade dress | Supercoupé original o candidato revisado sin logos/placa |
| High poly | Paneles, gaps, ópticas, pasos de rueda, interior y detalles de freno | Priorizar luces separadas, ruedas completas, cristales y pasos de rueda |
| Low/game mesh | Retopología limpia y UVs; detalles a normal/AO | Validar por lectura packaged; no aumentar geometría sin A/B |
| Bake | UV activa + objeto low activo para bake | Usar en activos propios futuros |
| Materiales | Paint, glass, tire, rim, brake, trim, lamps | Conservar material slots del asset visual |
| Ruedas/rig | Ruedas/dirección pertenecen a sistema de vehículo | `BodyMesh` Chaos sigue como root/collider; arte visual `NoCollision` |
| LOD/cook | Importar como review asset; probar escala, cooker, logs y capturas reales | BuildCookRun + benchmark packaged obligatorios |

## 8. Fuentes primarias/seguras consultadas

- Epic Games, Vehicle Art Setup / Chaos Vehicles setup / Automotive Configurator sample.
- Blender Manual 5.2, Render Baking.
- Kenney, Blaster Kit: página fuente reporta **Creative Commons CC0**, categoría 3D, 40 files.
- Poly Haven / ambientCG / OpenGameArt / Quaternius / Freesound quedaron como candidatos para texturas/props/audio, revisando licencia por asset antes de integrar.

---

## 9. Asset/Render Pass 2 — AAA 2026 research/application note (2026-08-17)

**Objetivo corregido por el usuario:** no basta con “presentable”; el criterio de aceptación sube a **vertical slice urbana AAA-caliber diseñada en 2026**, sin copiar GTA/Rockstar ni assets/marcas/trade dress licenciados.

### Research técnico incorporado

Ver documento dedicado: `docs/aaa-2026-asset-render-pass-research.md`.

Conclusiones aplicables a esta iteración:

- **Muscle simulation / Chaos Flesh / ML Deformer:** válido como research de dirección AAA para personajes hero futuros, pero no se integra en Pass 2 porque exige asset humanoide, dataset/deformers y cook/perf gates específicos. Para el estado actual, el camino seguro es mejorar silueta/ropa/material/luz de humanoides primero.
- **Ciudad:** el mayor retorno visual inmediato era romper el vacío/plataforma: más foreground props, storefronts, bollards, power boxes, awnings, árboles/planters y backdrop towers.
- **Vehículo:** mantener la regla técnica: `BodyMesh` sigue siendo root/collider Chaos; mesh visual como `AuthoredBodyMesh`/arte sin colisión. No se reemplazó aún `SM_CarConceptReview`, por tanto la atribución CC-BY-4.0 sigue vigente.
- **Licencias:** no se agregaron terceros nuevos en Pass 2. Todo el nuevo dressing urbano es C++ procedural/original con BasicShapes/Engine materials.

### Cambios aplicados

- `ANDWorldBuilder::BuildHeroStreetClutter()` añade dressing localizado en la ruta de screenshots packaged: power boxes, bollards, awnings, cables altos y señalética ficticia.
- `ANDWorldBuilder::BuildUrbanBackdrop()` añade torres de fondo y guardrail/rim para reducir el look de “plataforma flotante”.
- Se corrigió un fallo de unity build renombrando símbolos genéricos repetidos (`CubeMeshPath`, `SphereMeshPath`, `BasicShapeMatPath`, etc.) con prefijos por archivo.

### Evidencia packaged

```text
BuildCookRun: BUILD SUCCESSFUL
Runtime command: dist/Windows/NeonDistrictSandbox.exe -game -benchmark -log -unattended -nosplash
Benchmark: === RESULT: 29 passed, 0 failed ===
```

Archivos revisados:

- `dist/Windows/NeonDistrictSandbox/Saved/Benchmark/NDBenchmarkResult.txt`
- `dist/Windows/NeonDistrictSandbox/Saved/Logs/NeonDistrictSandbox.log`
- `dist/Windows/NeonDistrictSandbox/Saved/Screenshots/Windows/city_street.png`
- `dist/Windows/NeonDistrictSandbox/Saved/Screenshots/Windows/player_visible.png`
- `dist/Windows/NeonDistrictSandbox/Saved/Screenshots/Windows/npc_interaction_mei.png`
- `dist/Windows/NeonDistrictSandbox/Saved/Screenshots/Windows/vehicle_driving.png`

### Visual gate result

**PASS parcial:** las capturas ya muestran más densidad urbana real: storefronts/neon, carteles, bollards, planters, árboles, props pequeños, más depth en fachadas y más actividad alrededor del vehículo/jugador.

**WARN todavía abierto:** no declarar AAA final. Persisten:

- horizonte/road edge aún lee como plataforma en `vehicle_driving.png`;
- personajes siguen mannequin naranja, no ropa/roles/facial final;
- vehículo sigue usando `SM_CarConceptReview` CC-BY review asset;
- warnings Chaos vehicle por `Bone name for wheel 0 is not set` siguen en log;
- audio todavía no tuvo pass final.
