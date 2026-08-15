# Prompt de Continuación — Neon District Sandbox (próxima sesión)

_Copiar y pegar completo en la próxima sesión de Hermes._

---

**Objetivo:** Continuar y cerrar el benchmark Neon District Sandbox. El criterio de aceptación del usuario es **"tiene que poder ejecutarse y yo usarlo"** — todavía NO está cerrado.

**Proyecto:**
- Repo: `C:\Users\fjmn2\Dev\neon-district-sandbox`
- GitHub público: https://github.com/Nando2392/neon-district-sandbox
- Unreal: `C:\Program Files\Epic Games\UE_5.8`
- Wiki Obsidian: `C:\Users\fjmn2\Dev\Alien AI Studio\Neon District Sandbox\` (00-06 actualizados)
- NotebookLM: cuaderno `5c5bfbb7-a66b-456f-8c38-3cdc4c405f82` (5 fuentes + nota "ESTADO 2026-08-15")

**Contexto obligatorio — leer antes de actuar:**
1. `docs/process.md`
2. `docs/scorecard.md`
3. `docs/packaging.md`
4. `docs/packaging/gameinstance-cook-fix.md`
5. Wiki Obsidian: 01 Estado Actual.md y 03 Gates y Scorecard.md
6. NotebookLM vía `nlm` si está disponible

**Estado real al cierre de la sesión anterior (2026-08-15):**
- ✅ Compile gate PASSED — UE 5.8, `Build.bat NeonDistrictEditor Win64 Development` → `Result: Succeeded` (16 fixes de migración 5.6→5.8)
- ✅ Packaging PASSED — exe Windows en `dist/Windows/NeonDistrictSandbox.exe`, generado con BuildCookRun y ejecutado con éxito (exit 0)
- ✅ Benchmark automatizado **19/21 PASS** en el exe empaquetado: gameplay, misión "Entrega a Nova", AI (12 civiles + 2 policías), vehículo Chaos, wanted 3 niveles, pausa
- ❌ **save.write + save.load FAIL** — `GameInstanceClass=/Script/NeonDistrict.UNDGameInstance` (DefaultEngine.ini:6) **no se incluye en el pak cookeado** por UE 5.8 (clase C++ sin referencia de asset). Log: `Unable to load GameInstance Class '/Script/NeonDistrict.UNDGameInstance'. Falling back to generic UGameInstance.` → `Cast<UNDGameInstance>()` = nullptr. **Funciona en PIE/editor.**
- ❌ Audio gate FAIL (sin assets → silencio, documentado como límite de content)
- ⏸ Visual: ciudad procedural runtime OK; screenshots pendientes de captura manual

**Tareas de la próxima sesión (en orden):**

1. **Validación manual en editor (PIE)** — paso crítico para el criterio del usuario:
   ```
   "C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "C:/Users/fjmn2/Dev/neon-district-sandbox/NeonDistrictSandbox.uproject" -game -windowed -ResX=1280 -ResY=720
   ```
   - Verificar que `ND_MainMenu` y `ND_City` abren (0 actores estáticos; mundo 100% procedural runtime)
   - Recorrer en orden: mover/sprint → pausa (Esc) → hablar con Mei (E) → misión → recoger paquete → entregar a Nova → entrar/salir vehículo (F) → conducir → provocar wanted → evadir hasta bajar wanted → F5/F9 save/load (en PIE funciona)

2. **Screenshots visuales** → `docs/screenshots/` (menú, calle, jugador, NPC, vehículo, persecución, misión). Asignar skeletal mesh humano en editor si se desea (opcional).

3. **Resolver save/load en build empaquetado** (ticket `docs/packaging/gameinstance-cook-fix.md`):
   - **Opción A (recomendada):** crear Blueprint `BP_GameInstance` derivado de `UNDGameInstance` en el editor → `GameInstanceClass=/Script/NeonDistrict.BP_GameInstance` en DefaultEngine.ini → re-cocinar → re-ejecutar benchmark (esperado 21/21)
   - **Opción B:** aceptar PIE-only para validación de save/load y documentarlo

4. **Audio gate** (opcional): asignar SFX/música por soft-path o dejar documentado como silencio intencional.

5. **Cierre de gates:** actualizar `docs/scorecard.md` + wiki Obsidian + NotebookLM con evidencia real de cada gate.

6. **Higiene repo:** eliminar `docs/logs/` artifacts de debug (fix_rsp.py, game_link*.json) si se decide; commit final limpio + push.

**Regla de verdad:** NO declarar PASS sin evidencia real. Si no hay evidencia → FAIL/BLOCKED con causa. El benchmark runner (`Source/NeonDistrict/Benchmark/NDBenchmarkRunner.cpp`) es infraestructura de CI, no gameplay — no interferir.

**Comandos clave:**
```bash
# Build editor
"/c/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" NeonDistrictEditor Win64 Development -project="C:/Users/fjmn2/Dev/neon-district-sandbox/NeonDistrictSandbox.uproject" -waitmutex

# Empaquetar
"/c/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun -project="C:/Users/fjmn2/Dev/neon-district-sandbox/NeonDistrictSandbox.uproject" -noP4 -platform=Win64 -clientconfig=Development -serverconfig=Development -cook -map=/Game/Maps/ND_City -stage -pak -archive -archivedirectory="C:/Users/fjmn2/Dev/neon-district-sandbox/dist/Windows"

# NotebookLM
nlm login --check
nlm list sources 5c5bfbb7-a66b-456f-8c38-3cdc4c405f82
nlm note create 5c5bfbb7-a66b-456f-8c38-3cdc4c405f82 --content "<texto>" --title "..."
```

**Criterio de aceptación final:**
- Compile gate PASS
- ND_City abre en PIE con jugador controlable
- Evidencia real para cada gate (screenshots/logs)
- `docs/scorecard.md` refleja PASS/FAIL/BLOCKED con pruebas
- README actualizado
- Wiki Obsidian actualizada
- NotebookLM actualizado
- Commit + push completados
