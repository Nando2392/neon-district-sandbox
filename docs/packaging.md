# Packaging — Neon District Sandbox

Estado: **PASS técnico**. El ejecutable Windows fue generado y ejecutado en local contra Unreal Engine 5.8.

## Requisitos

- Unreal Engine 5.8: `C:\Program Files\Epic Games\UE_5.8`
- Visual Studio 2022 + Windows SDK
- Descriptor: `NeonDistrictSandbox.uproject`
- Módulo C++ runtime: `NeonDistrict` (no renombrar)

## Comando usado

```bat
"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun ^
  -project="C:\Users\fjmn2\Dev\neon-district-sandbox\NeonDistrictSandbox.uproject" ^
  -noP4 ^
  -platform=Win64 ^
  -clientconfig=Development ^
  -serverconfig=Development ^
  -build ^
  -cook ^
  -stage ^
  -pak ^
  -archive ^
  -archivedirectory="C:\Users\fjmn2\Dev\neon-district-sandbox\dist"
```

Helper local usado durante la sesión: `C:\Users\fjmn2\AppData\Local\Temp\run_build.bat`.

## Salida esperada

```text
dist/Windows/
├── NeonDistrictSandbox.exe
├── Engine/
└── NeonDistrictSandbox/
    ├── Binaries/Win64/NeonDistrictSandbox.exe
    ├── Content/Paks/NeonDistrictSandbox-Windows.pak
    └── Saved/
```

Ejecutable para el usuario: `dist/Windows/NeonDistrictSandbox.exe`.

## Verificación runtime

Antes de ejecutar benchmark, limpiar residuos de config suelta vieja:

```bash
cd C:/Users/fjmn2/Dev/neon-district-sandbox
rm -f dist/Windows/NeonDistrictSandbox.uproject
rm -rf dist/Windows/NeonDistrictSandbox/Config
rm -f dist/Windows/NeonDistrictSandbox/Saved/Logs/NeonDistrictSandbox.log \
      dist/Windows/NeonDistrictSandbox/Saved/Benchmark/NDBenchmarkResult.txt
cd dist/Windows
./NeonDistrictSandbox.exe -game -benchmark -log -unattended -nosplash
```

Evidencia final obtenida:

```text
Browse: /Game/Maps/ND_City?Name=Player
LoadMap: /Game/Maps/ND_City?Name=Player
Game class is 'NDGameMode'
[NDBenchmark] Benchmark runner started in 'ND_City'
=== RESULT: 25 passed, 0 failed ===
```

## Pitfalls ya resueltos

- `DefaultEngine.ini` necesita secciones con slash: `[/Script/EngineSettings.GameMapsSettings]`. Sin slash, UE ignora defaults y cae a `OpenWorld`.
- No copiar `NeonDistrictSandbox.uproject` a `dist/Windows/` raíz. Eso hizo que Unreal calculara mal `GameDir` y puede romper ICU/content lookup.
- Borrar `dist/Windows/NeonDistrictSandbox/Config` si contiene config suelta vieja; el config correcto está en el pak tras el recook.
- Target files actuales: `NeonDistrictSandbox.Target.cs` y `NeonDistrictSandboxEditor.Target.cs`; el módulo C++ sigue `NeonDistrict`.

## Estado visual

Packaging PASS no equivale a asset PASS. Las capturas del exe muestran ND_City procedural, pero todavía es una maqueta/greybox con cubos/materiales básicos. Próximo hito: renderizar y assetear la maqueta 3D.
