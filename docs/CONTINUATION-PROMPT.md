# Prompt de Continuación — Neon District Sandbox

Copiar y pegar completo en la próxima sesión.

---

Tenemos una maqueta 3D jugable de **Neon District Sandbox** en Unreal Engine 5.8. No retomar bugs viejos de 24/25, OpenWorld, save/load o movimiento: ya están resueltos. El objetivo ahora es **renderizar y assetear la maqueta 3D** hasta que el usuario la perciba visualmente como un juego urbano moderno con estándar tipo **GTA 5** — sin copiar IP, marcas, mapas ni assets de GTA.

## Contexto fijo

- Repo: `C:\Users\fjmn2\Dev\neon-district-sandbox`
- GitHub público: `https://github.com/Nando2392/neon-district-sandbox`
- Unreal: `C:\Program Files\Epic Games\UE_5.8`
- Descriptor: `NeonDistrictSandbox.uproject`
- Módulo C++: `NeonDistrict` (no renombrar)
- Exe usuario: `dist/Windows/NeonDistrictSandbox.exe`
- Wiki: `C:\Users\fjmn2\Dev\Alien AI Studio\Neon District Sandbox\`
- NotebookLM: cuaderno `5c5bfbb7-a66b-456f-8c38-3cdc4c405f82`

## Estado real actual

```text
BuildCookRun: BUILD SUCCESSFUL
Runtime: dist/Windows/NeonDistrictSandbox.exe -game -benchmark -log -unattended -nosplash
Map: /Game/Maps/ND_City
Game class: NDGameMode
=== RESULT: 25 passed, 0 failed ===
```

- ✅ La maqueta 3D es jugable y está empaquetada.
- ✅ Movimiento, misión, wanted, vehículos, pausa y save/load pasan.
- ⚠️ Visual sigue como greybox: cubos/materiales básicos/placeholders.
- ❌ Próximo hito: **Asset/Render Gate tipo GTA 5**.

## Skills obligatorias a cargar

- `unreal-gta5-visual-target`
- `unreal-humanlike-characters`
- `unreal-stylized-supercars`
- `unreal-engine-2026`
- `asset-pipeline-2026`
- `game-assets`
- `unreal-packaging-runtime`
- `unreal-troubleshooting-debugging`
- skills especializadas nuevas descargadas por el usuario para Unreal/assets/rendering.

## Research obligatorio antes de implementar

Haz todo el research necesario antes de assetear. No improvises rutas ni assets:

1. Investigar fuentes actuales UE5/Fab/Quixel/Megascans/Epic Samples/City Sample compatibles para:
   - ciudad urbana moderna;
   - materiales PBR/asfalto/acera/cristal/metal/neón;
   - humanos human-like;
   - vehículos estilizados tipo supercar;
   - props urbanos;
   - UI/HUD/audio.
2. Verificar licencia, tamaño, formato, import route y si requiere Git LFS.
3. Si assets externos bloquean por licencia/tamaño/login, crear alternativas stylized low-poly/runtime-safe en el repo.
4. Documentar una tabla: placeholder actual → asset/replacement elegido → fuente/licencia → evidencia cook/packaged.

## Tarea principal

Hacer un **Asset/Render Pass completo** con aceptación visual tipo GTA 5:

1. Materiales runtime coloreados/emisivos que funcionen en packaged build.
2. Personajes/NPCs human-like: player, civiles, policías, Mei y Nova deben parecer humanos, no cubos.
3. Coches estilizados como **supercars**: bajos, anchos, agresivos, con ruedas, cristales, faros, color, spoiler/lightbar para policía.
4. Props urbanos reconocibles: semáforos, señales, bancos, farolas, anuncios, basura, hidrantes, barreras, mobiliario urbano.
5. Fachadas y calles: asfalto, aceras, líneas de carril, ventanas emisivas, letreros, variación visual, iluminación nocturna, niebla/bloom.
6. HUD/menú/pausa: objetivo, wanted, interacción, save/load, misión visible en screenshots.
7. Audio: ambiente, pasos, motor, sirena, UI blips o synth runtime verificable.
8. Rehacer screenshots del exe y validar con visión: menú, calle, jugador, NPC/Mei, entrega a Nova, supercar, wanted/policía, pausa.

## Criterio de aceptación visual

No basta con que compile. Debe verse como un juego urbano moderno tipo GTA 5 en intención visual:

- ciudad densa y reconocible;
- escala humana correcta;
- humanos human-like reconocibles;
- supercars reconocibles;
- policía/wanted visualmente claro;
- HUD legible;
- materiales/iluminación no grises;
- screenshots vendibles, no greybox.

## Revalidación obligatoria

Después de cambios:

```bash
cd C:/Users/fjmn2/Dev/neon-district-sandbox
cmd.exe /c "C:/Users/fjmn2/AppData/Local/Temp/run_build.bat"
rm -f dist/Windows/NeonDistrictSandbox.uproject
rm -rf dist/Windows/NeonDistrictSandbox/Config
rm -f dist/Windows/NeonDistrictSandbox/Saved/Logs/NeonDistrictSandbox.log dist/Windows/NeonDistrictSandbox/Saved/Benchmark/NDBenchmarkResult.txt
cd dist/Windows
./NeonDistrictSandbox.exe -game -benchmark -log -unattended -nosplash
```

Criterio técnico mínimo:

```text
Map: /Game/Maps/ND_City
=== RESULT: 25 passed, 0 failed ===
```

Criterio visual mínimo:

- Capturas no son azul sólido ni greybox genérico.
- Humanos parecen humanos.
- Vehículos parecen supercars.
- Hay iluminación/materiales urbanos reconocibles.
- HUD/pausa/wanted/misión se ven claramente.

## Cierre esperado

- Actualizar docs del repo.
- Actualizar wiki Obsidian.
- Actualizar NotebookLM o dejar fuente Markdown lista.
- Commit y push a GitHub.
- No declarar PASS visual sin screenshots reales validadas.
