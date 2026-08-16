# Vehicle Art Architecture — Neon District Sandbox

## Visión General

El repositorio implementa un vehículo híbrido: **code-driven procedural shell + A/B third-party review candidate**. La arquitectura actual es una maqueta funcional que expone los puntos críticos que necesitan refinamiento artístico para producción.

---

## Lo que EXISTE

### 1. Mesh Visual
- **Collision Chassis**: `BodyMesh` (cube básico ×100) con colisión activa Chaos
- **Authored Body**: `SM_CarConceptReview.uasset` (7.9MB) en `Content/ThirdPartyReview/` - mesh de revisión CC-BY
- **Procedural Visual Shell**: `BodyVisualMesh` (ProceduralMeshComponent) - 6 estaciones + 36 vértices
- **Componentes visuales por separado**:
  - BodyMesh (colisión) → oculto render
  - AuthoredBodyMesh → opcional fallback A/B
  - BodyVisualMesh → procedural coupe-shell moderno
  - CabinMesh, WindshieldMesh, SpoilerMesh, FrontLightMesh, RearLightMesh
  - TrunkDeckMesh, RearBumperMesh, HoodMesh, NoseMesh
  - IntakeLeftMesh, IntakeRightMesh, FrontSplitterMesh, SideSkirtMesh
  - RearDiffuserMesh, MirrorLeftMesh, MirrorRightMesh
  - WheelFL, WheelFR, WheelRL, WheelRR (cilindros básicos)
- **Rotación Y-Forwarded → X-Forwarded**: AuthoredBodyMesh rotado 90° para alinear con chassis X-forward de Chaos

### 2. Materials
- **Runtime Material System**: `UMaterialInstanceDynamic` generado desde `/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial`
- **Función `ApplyAuthoredCoupeMaterials()`**: detecta slots Glass/Tire/Rim/Brake/FrontLight/RearLight y aplica colores específicos:
  - Glass: azul oscuro (0.002, 0.008, 0.018), emisivo 1.0
  - Tire/Technical: negro carbón (0.004, 0.004, 0.006)
  - Rim: gris medio (0.28, 0.30, 0.34)
  - Brake: rojo sangre (0.50, 0.02, 0.015)
  - FrontLight: blanco frío (0.78, 0.92, 1.0), emisivo 4.0
  - RearLight: rojo intenso (1.0, 0.025, 0.035), emisivo 4.5
- **Materiales en ThirdPartyReview**: Paint_1_Carmine, Paint_2_Carmine, Glass, Headlight, Brakelight, Rim1, Rim2, Tiretread, Tireside, Interior_X, Dashboard, Disc, Hardware, Signallight, Floormat, Mechanical, Mirror, Panel_Sides, Brake, Signallight

### 3. Collision/Chaos
- **UChaosWheeledVehicleMovementComponent** configurado en código
- **WheelSetup mínimo**: 4 ruedas con `UChaosVehicleWheel::StaticClass()`
- **Parámetros físicos hardcodeados**:
  - Masa: 1300 kg
  - MaxRPM: 6800
  - FinalRatio: 3.2
  - Steering curve: 0→90° @ 0-1800 RPM, 0.25 @ 3600 RPM
- **Advertencias existentes**:
  - `LogVehicle: Warning: Can't create vehicle ... Bone name for wheel 0 is not set`
  - `LogVehicle: Warning: ... has no torque curve defined, disabling mechanical simulation`

### 4. LOD/Cook
- **LOD**: NO DEFINIDO - solo procedural single-section
- **Cook Settings**: DirectoriesToAlwaysCook incluye `/Game/Vehicles` pero carpeta vacía
- **Procedural mesh**: Generado en runtime, no serializable sin cook explícito

---

## Lo que FALTA (gap producción)

### 1. Mesh Artístico Final
- ❌ `Content/Vehicles/` está vacío → no hay slot por defecto
- ❌ SM_CarConceptReview es candidato A/B, no art final
- ❌ No hay LOD 1/2/3 (distance fade)
- ❌ No hay componentes destructibles (breakable)
- ❌ Sin material slots por mesh (pintura remocable)

### 2. Materials Realistas
- ❌ Texturas PBR faltantes: Albedo/Normal/Metallic/Roughness
- ❌ Materiales de cristal con transmisión, refacción, Fresnel
- ❌ LLantas con normal maps, parches de neumático, calibrilla
- ❌ Luces con materiales de emisivo controlado por parámetros
- ❌ Coating metaplástico/neón con anisotropy, subsurface scattering

### 3. Asset Pipeline / Import Scripts
- ❌ No hay scripts FBX import pipeline
- ❌ No hay configuración de importación automática
- ❌ build.bat/build.sh son solo de compilación
- ❌ No hay documentación de workflow artístico

### 4. Collision Realista
- ❌ WheelSetup necesita BoneName válido (warning actual)
- ❌ Need ConvexHull per wheel (no cilindro básico)
- ❌ Need Compound collision (chassis + components)
- ❌ Need TireFrictionProfile, SuspensionData refinados

### 5. LOD System
- ❌ No hay generación automática (re-distance)
- ❌ No hay materiales por LOD
- ❌ Need cook settings para LOD groups

### 6. Documentación
- ❌ No hay SKILL.md ni pipeline guides
- ❌ No hay referencias a Blender, Substance, Quixel

---

## Arquitectura Recomendada para Producción

```
Content/Vehicles/ND_CyclonK77/
├── Mesh/
│   ├── SK_NDCyclonK77_Base.fbx        # Main body
│   ├── SK_NDCyclonK77_Chassis_Col.UPROPERTY  # Simplified collision
│   ├── SK_NDCyclonK77_LOD1.fbx        # 50% polys
│   ├── SK_NDCyclonK77_LOD2.fbx        # 25% polys
│   ├── SK_NDCyclonK77_Wheel_FL.fbx
│   ├── SK_NDCyclonK77_Wheel_FR.fbx
│   └── SK_NDCyclonK77_Wheel_RL.fbx
│
├── Materials/
│   ├── M_NDCyclonK77_Body_Paint       # Albedo + Metallic + Roughness + Normal
│   ├── M_NDCyclonK77_Glass            # Transmission + Refraction + Fresnel
│   ├── M_NDCyclonK77_Rim              # Anisotropy + Scratch map
│   ├── M_NDCyclonK77_Tire             # Tread normal + Rubber roughness
│   ├── M_NDCyclonK77_Light_Head       # Emissive float param
│   ├── M_NDCyclonK77_Light_Tail       # Emissive float param
│   └── M_NDCyclonK77_Specials
│
├── Textures/
│   ├── T_NDCyclonK77_Body_Albedo.exr
│   ├── T_NDCyclonK77_Body_Normal.exr
│   ├── T_NDCyclonK77_Body_Metallic.exr
│   ├── T_NDCyclonK77_Body_Roughness.exr
│   └── [Glass, Rim, Tire textures...]
│
├── Physics/
│   └── vehicle_assets.uasset  # Chaos vehicle setup asset (editor)
│
└── Configurations/
    └── VehicleCollision.uasset  # Shape profile, wheel bones
```

### Implementación C++ Necesaria
```cpp
// Reemplazar FObjectFinder con VehicleAsset UAsset
// Agregar BoneName: "wheel_fl", "wheel_fr", "wheel_rl", "wheel_rr"
// Agregar TorqueCurve: FRichCurve editable en editor
// Agregar VehicleProfile: Sport/Standard/Heavy
```

---

## Checklist de Migración

| Componente | Estado | Acción |
|------------|--------|--------|
| Mesh principal | ⚠️ Review candidate | Importar SK_NDCyclonK77_Base |
| Materials PBR | ❌ Falta | Crear materiales con texture maps |
| Collision realista | ⚠️ Warn | Asignar BoneNames, crear convex hulls |
| LOD system | ❌ Falta | Generar LOD 1/2, configurar Relevancy |
| Wheel physics | ⚠️ Basic | Definir perfil cámara, fuerza, amortiguación |
| Cook/ pak | ✅ Config | Verificar DirectoriesToAlwaysCook |
| Import scripts | ❌ Falta | Crear FBX Import Pipeline doc |

---

## Conclusión

El código C++ provee una **base física sólida** con Chaos WheeledVehicle. El **gap artístico es crítico**: faltan assets reales, texturas PBR, LOD y documentación de pipeline. La arquitectura debe evolucionar de "code-driven procedural" a "artist-driven assets with runtime fallback" para producción final.