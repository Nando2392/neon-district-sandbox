# GameInstance Cook Limitation — Fix Ticket

**Estado:** OPEN (post-benchmark)
**Prioridad:** Medium (no bloquea packaging ni gameplay en PIE)
**Fecha:** 2026-08-15

## Problema

El GameInstanceClass custom `UNDGameInstance` no se incluye en el pak empaquetado
por el cook de UE 5.8. Las clases C++ que no están referenciadas por Blueprints
o assets en `/Content` no se exportan al pak.

## Síntoma

```text
LogEngine: Error: Unable to load GameInstance Class '/Script/NeonDistrict.UNDGameInstance'.
Falling back to generic UGameInstance.
```

## Workaround

- En PIE/editor: funciona correctamente.
- En build empaquetado: save.write/save.load fallan (cast devuelve nullptr).

## Fix propuesto

Crear un Blueprint derivado de `UNDGameInstance` y usarlo como GameInstanceClass
en `Config/DefaultEngine.ini`. El cook incluirá el Blueprint + el C++ parent.

**Steps:**
1. En editor: `Class → Blueprint Class → All Classes → UNDGameInstance → Create`
2. Guardar como `Content/Blueprints/BP_GameInstance.uasset`
3. En `DefaultEngine.ini`: cambiar `GameInstanceClass=/Script/NeonDistrict.BP_GameInstance`
4. Re-cocinar y validar save/load en exe empaquetado.

## Tags
`#packaging`, `#GameInstance`, `#cook`, `#save-load`
