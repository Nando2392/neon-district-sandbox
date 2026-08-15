# NotebookLM Update — Neon District Sandbox — 2026-08-15

## Estado actual

La sesión cerró una **maqueta 3D jugable empaquetada** de Neon District Sandbox en Unreal Engine 5.8.

- Repo: `C:\Users\fjmn2\Dev\neon-district-sandbox`
- GitHub: `https://github.com/Nando2392/neon-district-sandbox`
- Exe standalone: `dist/Windows/NeonDistrictSandbox.exe`
- Wiki Obsidian: `C:\Users\fjmn2\Dev\Alien AI Studio\Neon District Sandbox\`

## Evidencia técnica final

```text
BuildCookRun: BUILD SUCCESSFUL
Runtime: dist/Windows/NeonDistrictSandbox.exe -game -benchmark -log -unattended -nosplash
Map: /Game/Maps/ND_City
Game class: NDGameMode
=== RESULT: 25 passed, 0 failed ===
```

Resultado final guardado en:

- `dist/Windows/NeonDistrictSandbox/Saved/Benchmark/NDBenchmarkResult.txt`
- `dist/Windows/NeonDistrictSandbox/Saved/Logs/NeonDistrictSandbox.log`
- `docs/screenshots/current_benchmark_contact_sheet.png`

## Qué quedó resuelto

- El exe ya no carga `OpenWorld`; carga `/Game/Maps/ND_City`.
- El GameMode correcto es `NDGameMode`.
- Save/load funciona en packaged build.
- Movimiento benchmark funciona (`moved 50.4 cm`).
- La causa raíz final de movimiento/geometría era escala incorrecta: `ANDWorldBuilder::AddBox()` pasaba dimensiones en centímetros como escala cruda al cubo de Unreal, que mide 100uu por lado. Ahora aplica `Scale / 100.0f`.

## Estado honesto visual

Esto NO es un juego visual final. Es una maqueta 3D jugable/vertical slice técnico.

- Hay screenshots actuales y ND_City visible.
- Pero la escena sigue greybox/placeholder: cubos, materiales básicos, humanos/vehículos no suficientemente reconocibles.
- Próxima sesión: Asset/Render Pass completo.

## Próxima sesión

Objetivo: renderizar y assetear la maqueta 3D.

Usar:

- `unreal-engine-2026`
- `asset-pipeline-2026`
- `game-assets`
- `unreal-packaging-runtime`
- `unreal-troubleshooting-debugging`
- skills especializadas nuevas del usuario

Criterio de aceptación: mantener `=== RESULT: 25 passed, 0 failed ===` y añadir screenshots reales donde humanos, vehículos, ciudad, HUD, pausa/wanted/misión sean reconocibles y presentables.
