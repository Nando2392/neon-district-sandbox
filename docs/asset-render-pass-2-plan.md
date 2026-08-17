# Asset/Render Pass 2 Plan — AAA 2026 Direction

**Proyecto:** Neon District Sandbox  
**Fecha:** 2026-08-17  
**Research base:** `docs/aaa-2026-asset-render-pass-research.md`  
**Gate mínimo:** packaged EXE + benchmark `29/29` o superior + screenshots revisados.  

---

## Principio

El nuevo criterio no es “que compile” ni “que se vea mejor que greybox”. El criterio es una **vertical slice urbana con dirección AAA 2026**, dentro de las restricciones del repo público:

- original/no GTA/IP/trade dress;
- sin assets grandes/no redistribuibles;
- cambios cook-safe;
- evidencia en packaged EXE.

---

## Scope recomendado para esta sesión

### En scope

1. **Research documentado**
   - ML Deformer / Chaos Flesh / MetaHuman Crowd / Mass / Lumen / VSM / MetaSounds.
   - Estado local de plugins UE 5.8.
   - Diagnóstico visual de screenshots actuales.

2. **Densidad urbana localizada**
   - Mejorar los puntos que las cámaras benchmark ya ven.
   - Añadir profundidad y ocultar vacío/horizonte: skyline/backdrop, más bloques/fachadas, parapetos, carteles, postes, cableado simple, curb detail.
   - Más storefronts ficticias y señalética urbana.

3. **Personajes/roles sin bomba de dependencia**
   - Mantener TutorialTPP ya integrado o probar MoverExamples Manny sólo si cook no se rompe.
   - Mejorar material/rol/accesorios: policía, Mei, Nova, civiles.
   - Evitar Chaos Flesh en esta fase; documentarlo como roadmap hero-character.

4. **Arma/VFX/audio feedback**
   - Mejorar blaster propio o integrar Kenney sólo tras auditar descarga/licencia/tamaño.
   - Añadir muzzle flash/impact cue procedural si viable.
   - Mejorar audio procedural: disparo, impacto, sirena/ambiente.

5. **Vehículo con cautela**
   - No mover `BodyMesh` root/collider.
   - No romper `AuthoredBodyMesh` `NoCollision`.
   - Si se toca: material readability/wheel warning docs; reemplazo final original/CC0 es hito mayor.

### Out of scope para esta sesión corta

- Implementar ML Deformer/Chaos Flesh completo.
- Descargar MetaHumans/Fab/Marketplace/Quixel/City Sample.
- Migrar todo a Mass/StateTree/SmartObjects.
- Hacer “AAA final” sin packaged visual gates.

---

## Orden de implementación propuesto

### Step 1 — Urban hero corners

**Objetivo:** que `city_street.png`, `player_visible.png`, `npc_interaction_mei.png` y `vehicle_driving.png` no muestren vacío/plataforma.

Cambios probables en `NDWorldBuilder`:

- Añadir `BuildUrbanBackdrop()` o ampliar `BuildDistrict()` con edificios de fondo no interactivos.
- Añadir `BuildStorefrontCluster()` cerca de las cámaras.
- Añadir props ligeros repetidos:
  - bollards,
  - power boxes,
  - cable posts,
  - street bins,
  - bus stop / shelter,
  - lane arrows / crosswalk wear,
  - fake ads/signs with original names.

Gate visual:

- skyline/fondo urbano visible,
- menos vacío/horizonte azul,
- más escala humana en foreground.

### Step 2 — Human role readability

Cambios probables en `NDNPCCharacter`:

- Añadir accesorios visuales adicionales sobre el mannequin/proxy:
  - police cap/vest light,
  - Mei jacket/accent,
  - Nova courier bag,
  - civilian backpack/hat variants.
- Si el material de TutorialTPP no acepta tint bien, usar overlay props en BasicShapes/Engine materials.

Gate visual:

- Mei/Nova/policía distinguibles en screenshots sin leer HUD.

### Step 3 — Weapon readability + procedural feedback

Cambios probables:

- Aumentar escala/forma del blaster visible.
- Añadir muzzle flash breve / emissive barrel component al disparar.
- Log/audio gate: disparo e impacto.

Gate visual:

- `player_visible.png` muestra arma clara.
- Log confirma weapon fire + projectile/impact.

### Step 4 — Audio procedural pass

Cambios probables:

- Extender `NDSynthAudioComponent`/`NDAudioManager`.
- Sin assets externos si se puede.

Gate:

- No necesariamente screenshot, pero sí log + packaged execution.

### Step 5 — Validate and document

Comandos:

```bash
cd C:/Users/fjmn2/Dev/neon-district-sandbox
cmd.exe /c "C:/Users/fjmn2/AppData/Local/Temp/run_build.bat"

cd C:/Users/fjmn2/Dev/neon-district-sandbox/dist/Windows
rm -f NeonDistrictSandbox.uproject
rm -rf NeonDistrictSandbox/Config \
       NeonDistrictSandbox/Saved/Logs \
       NeonDistrictSandbox/Saved/Benchmark \
       NeonDistrictSandbox/Saved/Screenshots/Windows
./NeonDistrictSandbox.exe -game -benchmark -log -unattended -nosplash
```

Expected:

```text
BUILD SUCCESSFUL
=== RESULT: 29 passed, 0 failed ===
```

---

## Research conclusion for muscle simulation

ML Deformer + Chaos Flesh is **valid AAA 2026 tech**, and UE_5.8 local has the plugin stack. But it is not the best immediate pass because it requires a stable skeletal mesh, target mesh topology, training/simulation data and extra packaging risk.

For this project, Pass 2 should first make humans read as humans/roles in packaged screenshots. Then a future **hero-character deformation spike** can test:

- `MLDeformerFramework`,
- `ChaosFleshGenerator`,
- `ChaosFlesh`,
- base/target mesh workflow,
- packaged runtime cost.

---

## Acceptance for Pass 2

Pass 2 can be called accepted only if:

- BuildCookRun succeeds.
- Packaged benchmark is `29/29` or higher with zero failures.
- Screenshots show visible improvement over current baseline in:
  - urban density/background,
  - player/NPC role readability,
  - weapon/vehicle/context readability.
- License manifest/docs updated.
- No new external asset without source/license/redistribution/cook proof.
- No regressions in weapon, vehicle, mission, wanted, save/load, movement.
