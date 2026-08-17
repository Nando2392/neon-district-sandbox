# AAA 2026 Asset/Render Pass Research — Neon District Sandbox

**Fecha:** 2026-08-17 02:47 RDT  
**Proyecto:** `C:/Users/fjmn2/Dev/neon-district-sandbox`  
**Engine objetivo:** Unreal Engine 5.8  
**Criterio actualizado:** vertical slice urbana third-person con aspiración **AAA 2026**, no sólo benchmark técnico.  
**Restricción IP/licencia:** original, sin GTA/Rockstar/trade dress, sin Marketplace/Fab/Quixel/MetaHuman descargado si no hay licencia, tamaño, redistribución y cook proof.

---

## 1. Fuentes y evidencia consultada

### Fuentes web / documentación

1. Epic Developer Community — **How to Use the Machine Learning Deformer in Unreal Engine 5.8**  
   URL: https://dev.epicgames.com/documentation/en-us/unreal-engine/how-to-use-the-machine-learning-deformer-in-unreal-engine  
   Evidencia vía search result: ML Deformer usa `Base Mesh` ligado a skeleton y `Target Mesh` con misma topología/vertices; el target puede provenir de volume preservation y **muscle simulation** para deformación realista.

2. Epic Developer Community — **ML Deformer Framework in Unreal Engine 5.8**  
   URL: https://dev.epicgames.com/documentation/unreal-engine/ml-deformer-framework-in-unreal-engine  
   Evidencia vía search result: framework para entrenar modelos ML Deformer que producen deformaciones de alta calidad en runtime.

3. Epic Developer Community — **Chaos Flesh Overview in Unreal Engine 5.8**  
   URL: https://dev.epicgames.com/documentation/unreal-engine/chaos-flesh-overview  
   Evidencia vía search result: Chaos Flesh proporciona simulación real-time de cuerpos deformables/soft bodies donde la forma puede cambiar durante simulación.

4. Epic Developer Community — **ML Deformer Sample in Unreal Engine 5.8**  
   URL: https://dev.epicgames.com/documentation/en-us/unreal-engine/ml-deformer-sample-in-unreal-engine  
   Evidencia vía search result: sample muestra personaje de alta fidelidad real-time con deformaciones aprendidas desde simulaciones offline de muscle, flesh y cloth.

5. Epic Developer Community — **Unreal Engine 5.8 Release Notes**  
   URL: https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes  
   Evidencia vía search result: MegaLights llega a Production Ready en 5.8, con reducción de ruido y mejoras de rendimiento para 60fps; MetaHuman Crowd experimental usa transiciones entre actores high-fidelity e Instanced Skinned Meshes por distancia.

6. Unreal Engine News — **Unreal Engine 5.8 is here**  
   URL: https://www.unrealengine.com/news/unreal-engine-5-8-is-now-available  
   Evidencia vía search result: MetaHuman Animator puede capturar performance de cuerpo/cara desde una cámara off-actor/webcam.

7. MetaHuman — **MetaHuman 5.8 is now available / release notes**  
   URL: https://www.metahuman.com/news/metahuman-5-8-is-now-available  
   Evidencia vía search result: MetaHuman Crowd experimental permite combinar close-ups realistas con footprint optimizado vía transición a Instanced Skinned Meshes, composición modular de personajes y Mass para crowd orchestration.

8. Epic Developer Community — **Mass Entity / StateTree / Smart Objects / City Sample**  
   URLs:
   - https://dev.epicgames.com/documentation/en-us/unreal-engine/mass-entity-in-unreal-engine
   - https://dev.epicgames.com/documentation/unreal-engine/state-tree-in-unreal-engine
   - https://dev.epicgames.com/documentation/unreal-engine/overview-of-state-tree-in-unreal-engine
   - https://dev.epicgames.com/documentation/en-us/unreal-engine/city-sample-project-unreal-engine-demonstration  
   Evidencia vía search result: MassGameplay es sistema ECS para muchas entidades; StateTree combina selectores de behavior trees con estados/transiciones; Smart Objects permiten interacción configurable de AI/players; City Sample usa Smart Objects para interactividad urbana.

9. Epic Developer Community — **Chaos Vehicles / How to Set Up Vehicles**  
   URLs:
   - https://dev.epicgames.com/documentation/unreal-engine/chaos-vehicles
   - https://dev.epicgames.com/documentation/unreal-engine/how-to-set-up-vehicles-in-unreal-engine  
   Evidencia vía search result: Chaos Vehicles es sistema lightweight de física de vehículos; torque curve típica tiene forma de U invertida, pico en rango medio de RPM y caída a ambos lados.

10. Epic Developer Community — **Lumen / Virtual Shadow Maps / rendering**  
   URLs:
   - https://dev.epicgames.com/documentation/unreal-engine/lumen-performance-guide-for-unreal-engine
   - https://dev.epicgames.com/documentation/unreal-engine/virtual-shadow-maps-in-unreal-engine
   - https://dev.epicgames.com/documentation/unreal-engine/lumen-technical-details-in-unreal-engine
   Evidencia vía search result: Lumen depende de TSR para output 4K desde resolución interna menor; VSM está pensado para assets film-quality y mundos grandes dinámicamente iluminados.

11. Epic Developer Community — **MetaSounds**  
   URLs:
   - https://dev.epicgames.com/documentation/unreal-engine/metasounds-in-unreal-engine
   - https://dev.epicgames.com/documentation/unreal-engine/metasounds-the-next-generation-sound-sources-in-unreal-engine  
   Evidencia vía search result: MetaSounds usa grafos DSP, timing sample-accurate y control a nivel de audio buffer para sistemas procedurales.

12. Kenney — **Blaster Kit**  
   URL: https://kenney.nl/assets/blaster-kit  
   Evidencia vía search result: pack gratuito, **CC0**, categoría 3D, 40 assets.

13. Kenney itch.io — **Blaster Kit by Kenney**  
   URL: https://kenney-assets.itch.io/blaster-kit  
   Evidencia vía search result: License CC0 1.0 Universal; uso comercial permitido; atribución no requerida aunque apreciada.

14. Poly Haven — **License**  
   URL: https://polyhaven.com/license  
   Evidencia vía search result: assets bajo **CC0**, public domain incluso en jurisdicciones que no soportan dominio público.

15. Poly Haven Wiki — **FAQ**  
   URL: https://docs.polyhaven.com/en/faq  
   Evidencia vía search result: todos sus assets CC0, uso libre incluyendo AI training.

### Evidencia local verificada en esta máquina

Comando ejecutado contra `C:/Program Files/Epic Games/UE_5.8/Engine/Plugins` confirmó plugins locales:

```text
AI/MassAI
AI/MassCrowd
Animation/DeformerGraph
Animation/MLDeformer
Animation/MLDeformer/ChaosFleshGenerator
Animation/MLDeformer/MLDeformerFramework
Experimental/ChaosFlesh
Experimental/ChaosVehiclesPlugin
Experimental/MetaHuman
Experimental/MetaHuman/MetaHumanRuntime
Experimental/Mover
Experimental/MoverExamples
MetaHuman/MetaHumanAnimator
MetaHuman/MetaHumanCharacter
MetaHuman/MetaHumanCrowd
Runtime/GameplayBehaviorSmartObjects
Runtime/GameplayStateTree
Runtime/MassGameplay
Runtime/SmartObjects
Runtime/StateTree
Runtime/AudioMotorSim
Runtime/AudioSynesthesia
```

Estado actual del proyecto:

- `.uproject` sólo habilita `EnhancedInput`, `ChaosVehiclesPlugin`, `ProceduralMeshComponent`, `Niagara` y plugins editor-only de modeling/python.
- `DefaultEngine.ini` ya tiene `r.AntiAliasingMethod=4`, `r.Shadow.Virtual.Enable=1`, `r.Lumen.DiffuseIndirect.Allow=1`, `r.Lumen.Reflections.Allow=1`, `r.VolumetricFog=1`, DX12 default.
- `NDWorldBuilder` ya tiene props básicos: semáforos, señales, bancos, hidrantes, kiosko, basura, crosswalks, árboles, luces y post-process.
- Audio actual usa `NDSynthAudioComponent` y `NDAudioManager`; no hay MetaSounds integrado.
- NPCs siguen C++/procedurales, no skeletal/MetaHuman/Manny.

---

## 2. Lo que significa “AAA diseñado en 2026” para este repo

No significa meter assets enormes o copiar GTA. Significa que cada captura packaged debe mostrar intención de producción en estas capas:

| Capa | AAA 2026 esperado | Camino seguro para este repo público |
|---|---|---|
| Personajes | Skeletal humans, cloth/hair/muscle deformation, crowd LOD | Fase corta: MoverExamples Manny/Tutorial humanoide o procedural humanoide mejorado; fase larga: ML Deformer/Chaos Flesh sólo para hero character experimental |
| Animación/deformación | ML Deformer/Chaos Flesh/cloth simulation para close-ups | Investigar y documentar; no bloquear Pass 2 si no hay DCC/training dataset |
| Ciudad | Densidad urbana con props, fachadas, storefronts, iluminación y vida | Procedural C++ + assets CC0 ligeros; más capas, composición por screenshots |
| Vehículo | Hero mesh original, materiales separados, wheel setup correcto, torque curve | Mantener BodyMesh Chaos; mejorar art visual original/CC0; resolver warnings o documentarlos |
| Rendering | Lumen/VSM/TSR, contacto, niebla, exposición controlada, material hierarchy | Ajustes empaquetados medidos; screenshots EXE como gate primario |
| Audio | Procedural/MetaSounds, motores, sirenas, ambiente, UI feedback | Usar synth procedural primero; MetaSounds o CC0 sólo si cook proof |
| AI urbano | Crowds/tráfico con Mass/StateTree/SmartObjects | No integrar grande ahora; usar como roadmap. Para Pass 2, mejorar densidad visible y comportamiento actual sin romper gates |
| Licencias | Assets auditados, redistribuibles, tamaño controlado | Kenney/Poly Haven/ambientCG/propios; documentar por asset |

---

## 3. Muscle simulation / deformation en Unreal 5.8

### Opciones reales

1. **ML Deformer Framework**
   - Sirve para aprender deformaciones complejas desde simulaciones offline.
   - Requiere `Base Mesh` skeletal y `Target Mesh` con misma topología/vertices.
   - Para músculo realista, el target suele venir de simulación offline con preservación de volumen / músculo / flesh / cloth.
   - Buen fit: hero character close-up, no crowds masivos.

2. **Chaos Flesh**
   - Soft body/deformable simulation real-time.
   - Puede generar datos para muscle/flesh workflows.
   - En UE 5.8 está disponible localmente como `Experimental/ChaosFlesh` y `MLDeformer/ChaosFleshGenerator`.
   - Riesgo: experimental, requiere pipeline más complejo y posible integración DCC/training.

3. **MetaHuman 5.8 / MetaHuman Animator / MetaHuman Crowd**
   - Excelente para estándar AAA humano, pero implica assets grandes, pipeline Epic, y potencialmente contenido no redistribuible o no apto para repo público.
   - MetaHuman Crowd experimental usa ISKM y Mass para crowds; útil como benchmark/roadmap, no como primer cambio en repo público sin revisar tamaño/licencia.

4. **MoverExamples Manny / TutorialTPP**
   - Mejor opción local-first para reemplazar stick figures sin descarga: activos ya incluidos en UE 5.8.
   - MoverExamples requiere habilitar plugin y validar cook; TutorialTPP es más simple.
   - Para Pass 2, esto probablemente da más mejora visible/riesgo menor que montar Chaos Flesh.

### Decisión recomendada

**No implementar muscle simulation como primer paso de Pass 2.**  
Hacer primero:

1. Reemplazar/prototipar NPC/player visual con un humanoide UE-local cookable (Manny/TutorialTPP) o mejorar procedural humanoide.
2. Documentar ML Deformer/Chaos Flesh como **roadmap de hero character**.
3. Sólo activar MLDeformer/ChaosFlesh si hay un caso aislado con:
   - skeletal mesh estable,
   - target mesh/topología consistente,
   - training data disponible,
   - build packaged verde,
   - tamaño aceptable.

Motivo: el usuario pide AAA 2026, pero el repo público y el benchmark actual premian mejoras visibles packaged. ML muscle es AAA, pero no es el primer cuello de botella visible si los NPCs aún leen como placeholders.

---

## 4. Research de áreas necesarias para Pass 2

### 4.1 Personajes/NPCs

**Gap actual:** humanos simplificados.  
**AAA 2026:** silueta humana, ropa/rol, animación/idle/walk, close-up aceptable, deformación en hero si aplica.  
**Plan seguro:**

- Auditar assets locales UE 5.8:
  - `MoverExamples` Manny: mejor candidato si cook pasa.
  - TutorialTPP: fallback sin plugin pesado.
- Mantener `ANDCharacter`/`ANDNPCCharacter` y cápsula/AI; sólo cambiar visual.
- Añadir roles por material/prop:
  - policía: chaleco/placa/luz azul-roja,
  - Mei: chaqueta/cabello/acento cálido,
  - Nova: courier bag/visor,
  - civiles: 3–5 variantes.
- Gate: `player_visible.png`, `npc_interaction_mei.png`, `mission_delivery_nova.png` deben mostrar humano reconocible.

### 4.2 Vehículo

**Gap actual:** `SM_CarConceptReview` mejora visual pero CC-BY review; Chaos warnings por wheel bones/torque curve.  
**AAA 2026:** hero vehicle original, material slots correctos, ruedas/vidrio/luces/paint, torque curve y wheel setup sin warnings.  
**Plan seguro:**

- No tocar `BodyMesh` root/collider.
- `AuthoredBodyMesh` sigue `NoCollision`.
- Mejorar o reemplazar por asset original/CC0.
- Resolver o documentar:
  - wheel bone names,
  - torque curve,
  - physics warnings.
- Gate: `vehicle_driving.png` y `wanted_police_chase.png` deben mostrar carro legible y chase creíble.

### 4.3 Ciudad/urban density

**Gap actual:** hay props, pero no densidad AAA.  
**AAA 2026:** layered streets: curb, lane markings, traffic lights, storefronts, trash, poles, benches, kiosks, signs, billboards, lit interiors, skyline, composition.  
**Plan seguro:**

- Aumentar composición de los 7 screenshot fixtures, no todo el mundo.
- Añadir variación procedural:
  - fachadas laterales, depth, awnings, window mullions,
  - storefront brands ficticias,
  - más señales/farolas/semáforos por intersección,
  - parked props y street clutter.
- Añadir “district hero corners” cerca de cameras benchmark.

### 4.4 Rendering / lighting

**Estado actual:** Lumen, VSM, volumetric fog, DX12 habilitados; postprocess existe.  
**AAA 2026:** material hierarchy y sombras/contacto, no bloom excesivo.  
**Plan seguro:**

- No cambiar 20 settings globales a la vez.
- Primero screenshot A/B de:
  - contact shadows en props/personajes,
  - exposure/night grade,
  - fog density,
  - emissive intensity hierarchy.
- Gate: screenshots EXE, no editor/Movie Render Queue.

### 4.5 Audio

**Gap actual:** synth/audio manager básico; no ambiente/motor/sirena/arma rico.  
**AAA 2026:** procedural/MetaSounds, motor por RPM, sirena, city bed, UI blips, weapon transient/impact.  
**Plan seguro:**

- Fase 1 sin assets externos: mejorar `NDSynthAudioComponent`/`NDAudioManager`:
  - ambient drone,
  - footsteps blips,
  - siren/police chase oscillator,
  - weapon shot/impact envelope.
- Fase 2: MetaSounds o Freesound CC0 sólo con cook proof y license file.

### 4.6 Assets/licencias

**Candidatos seguros:**

- Kenney Blaster Kit: CC0, 3D, 40 assets, no real-world gun branding.
- Poly Haven: CC0 textures/HDRI/props.
- ambientCG: candidato para PBR textures, requiere verificar por asset.
- Propio Blender/C++ procedural: más seguro para repo público.

**No usar sin decisión explícita:** Fab/Marketplace/Quixel/Megascans/MetaHuman downloads/City Sample content, por login, tamaño, EULA/redistribución y cook proof.

---

## 5. Plan de ejecución recomendado para Asset/Render Pass 2

### Orden recomendado por impacto/riesgo

1. **NPC/humanoides visibles** — mayor impacto en `player_visible`, `npc_interaction_mei`, `mission_delivery_nova`.
2. **Composición urbana de screenshots** — densidad localizada alrededor de cámaras benchmark.
3. **Arma visual** — Kenney CC0 auditado o blaster propio mejorado; gameplay ya existe.
4. **Audio procedural** — ambiente/sirena/weapon/impact sin assets externos.
5. **Vehículo final/warnings** — si se toca, cuidar Chaos root/collider y validar enter/drive/exit.
6. **ML Deformer/Chaos Flesh** — research/roadmap o spike aislado, no gate principal del pase corto.

### Acceptance gates nuevos sugeridos

Además del benchmark `29/29`:

- `visual.npc_humanlike`: screenshot muestra humanoides no-stick figure.
- `visual.street_density`: `city_street.png` tiene mínimo 6 categorías de props urbanos visibles.
- `visual.weapon_readable`: `player_visible.png` muestra arma/blaster ficticio legible.
- `audio.runtime_feedback`: log confirma disparo/impacto/sirena/ambiente o componente synth activo.
- `vehicle.no_new_chaos_regression`: enter/drive/exit PASS y warnings no empeoran.
- `license.asset_manifest`: todo asset externo tiene fuente/licencia/redistribución/cook proof.

---

## 6. Decisión para este repo ahora

**Hacer research-first y luego implementar un pase incremental.**  
No activar todo el stack AAA de golpe. Para esta base procedural pública, el camino correcto es:

1. Crear una vertical slice visualmente más densa y humana en packaged EXE.
2. Usar UE-local assets/procedural/CC0 auditado.
3. Tratar ML Deformer + Chaos Flesh como una línea de producción para hero character futuro, no como requisito de cierre de Pass 2.
4. Elevar criterios visuales con screenshots + vision review + benchmark, no con claims.

---

## 7. Checklist antes de implementación

- [ ] Elegir asset tier para humanoides: MoverExamples Manny vs TutorialTPP vs procedural mejorado.
- [ ] Verificar si habilitar plugin adicional rompe cook/package.
- [ ] Auditar Kenney Blaster Kit descarga/formato/tamaño si se integra.
- [ ] Decidir si vehículo se toca en este pase o sólo se documenta.
- [ ] Definir 2–3 screenshot cameras prioritarias para densidad urbana.
- [ ] Mantener `BUILD SUCCESSFUL` + `=== RESULT: 29 passed, 0 failed ===` como gate mínimo.
- [ ] No commit/push hasta docs/wiki/benchmark/screenshots verdes.

---

## 8. Diagnóstico visual packaged actual contra AAA 2026

Fuente: screenshots reales del EXE en
`dist/Windows/NeonDistrictSandbox/Saved/Screenshots/Windows/`.

### `city_street.png`

- La escena ya tiene calles, árboles, neón, carteles, jardineras y coche; ya no es greybox puro.
- Falla AAA por **geometría de edificios todavía prismática**: fachadas planas con ventanas como rectángulos emisivos sin interiores, marcos finos, balcones, zócalos, HVAC, cables, puertas, toldos ni detalle lateral.
- La calle se percibe como plataforma sobre vacío/océano azul en el horizonte; falta fondo urbano, skyline o bloqueo visual.
- Árboles y foliage tienen volumen, pero hoja/tronco se leen low-poly y repetidos.
- Props existen pero no forman lenguaje urbano denso: faltan postes repetidos, marcas viales secundarias, basura pequeña, cajas eléctricas, señalética de tránsito, curb detail, shopfronts y variación.
- Materiales siguen planos: asfalto/acera no muestran charcos, manchas, roughness contrast, desgaste de bordillos o microdetail convincente.

### `player_visible.png`

- El humanoide ya lee como mannequin humano; mejora fuerte frente a cubos.
- Falla AAA porque el personaje está **amarillo uniforme**, sin ropa real, piel/cara/cabello, textiles, holsters, zapatos ni material separation.
- La pose de arma existe pero es rígida y sin aim-offset/animación de disparo; no hay recoil, muzzle flash visible ni VFX de proyectil en la captura.
- El arma/blaster es demasiado pequeña/poco distinguible en pantalla; no se lee como prop diseñado.
- Entorno alrededor del jugador aún es amplio/vacío, con líneas de calle muy limpias y pocos elementos de escala humana.

### `npc_interaction_mei.png`

- El NPC cercano se parece al player/mannequin; roles se distinguen poco salvo color/posición.
- No hay rostro/expresión/ropa/accesorio legible de “Mei” o personaje de misión.
- UI/interacción no se ve claramente en el frame; la captura no vende conversación o misión AAA.
- Señales/carteles ayudan, pero se ven como cajas emisivas con rectángulo negro, no storefronts o cartelería diseñada.

### `vehicle_driving.png`

- El vehículo se lee como supercoupé y está mucho mejor que el collider/cubo.
- Falla AAA por materiales: pintura roja demasiado uniforme/plástica, techo blanco plano, vidrio/lámparas sin shader convincente, ruedas con detalle limitado.
- La composición deja mucho vacío/océano al horizonte; rompe ilusión de ciudad densa.
- El coche no tiene entorno de calle rico alrededor: faltan parked cars, reflejos, marcas viales realistas, tráfico, farolas, señales y obstáculos.
- El asset sigue siendo `SM_CarConceptReview` CC-BY review/prototype salvo reemplazo posterior; no es final original.

### Prioridad visual derivada

1. **Densidad y composición urbana alrededor de cameras**: más impacto inmediato en todas las capturas.
2. **Humanos/roles**: ropa/colores/accesorios y/o MoverExamples Manny; no saltar todavía a Chaos Flesh.
3. **Arma visual y VFX**: blaster legible, muzzle/impact feedback.
4. **Vehículo/materiales**: material separation y background urbano; reemplazo final original/CC0 queda como hito mayor.
5. **Rendering**: ajustar contacto/sombras/fog/exposure sólo con A/B packaged.
