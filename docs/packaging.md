# Packaging — Neon District Sandbox

Cómo se genera el ejecutable Windows y dónde queda.

> Estado: **pendiente de motor**. UE 5.6 + launcher instalado pendiente de login
> del usuario. El flujo de abajo es el procedimiento exacto a ejecutar.

## Requisitos

- Unreal Engine 5.6 instalado (Epic Games Launcher).
- Visual Studio 2022 + Windows SDK (ya instalados en esta máquina).
- Proyecto abierto y compilado al menos una vez (generar VS files → Build).

## Procedimiento

### Opción A — Editor (recomendada)

1. Abrir `NeonDistrictSandbox.uproject`.
2. `File > Package Project > Windows > Win64`.
3. Elegir carpeta de salida (sugerida: `C:\Users\fjmn2\Dev\neon-district-sandbox\dist`).
4. Esperar cook+stage (~5-15 min la primera vez; shaders/DDC se cachean).
5. Ejecutable: `dist/Windows/NeonDistrictSandbox.exe`.

### Opción B — CLI (RunUAT)

Desde la raíz del motor:

```
"<UE_ROOT>\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun \
  -project="C:\Users\fjmn2\Dev\neon-district-sandbox\NeonDistrictSandbox.uproject" \
  -platform=Win64 -clientconfig=Development \
  -cook -stage -pak -archive \
  -archivedirectory="C:\Users\fjmn2\Dev\neon-district-sandbox\dist"
```

Salida: `dist/Windows/NeonDistrictSandbox.exe`.

## Notas para el packaging

- `dist/` y `Packaged/` están en `.gitignore` — el binario no se sube al repo.
- El juego corre **sin assets externos**: la ciudad, UI, materiales y menú se
  generan en runtime. Verificar en el exe empaquetado que:
  - `ND_MainMenu` y `ND_City` existen en `Content/Maps/` (creados en editor).
  - Si se asignaron skeletal mesh / sonidos, están referenciados por soft-path
    y se cookean solos.
- Logs de packaging: `<ProjectDir>/Saved/Logs/` y la consola de RunUAT. Ante
  fallo, copiar el log a `docs/process.md` (sección fallos) y corregir.

## Verificación post-packaging

1. `dist/Windows/NeonDistrictSandbox.exe` arranca y llega al menú.
2. Jugar → ciudad se construye (world builder) → gate PIE completo.
3. Captura de pantalla del exe empaquetado → `docs/screenshots/`.
