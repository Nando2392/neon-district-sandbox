#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Script para generar assets de Enhanced Input automáticamente.
Ejecutar con: ue4editor-cr.exe NeonDistrict.uproject -run=CreateInputAssets -game -log

Alternativa: Ejecutar desde Content/Editor/GenerateInputAssets.py
"""

import unreal

# Configuración
INPUT_ACTIONS = [
    {"Name": "IA_Move", "ValueType": "Axis2D", "Keys": ["W", "S", "A", "D", "Up", "Down", "Left", "Right"]},
    {"Name": "IA_Look", "ValueType": "Axis2D", "Keys": ["MouseX", "MouseY"]},
    {"Name": "IA_Jump", "ValueType": "Boolean", "Keys": ["SpaceBar"]},
    {"Name": "IA_Sprint", "ValueType": "Boolean", "Keys": ["LeftShift"]},
    {"Name": "IA_Interact", "ValueType": "Boolean", "Keys": ["E"]},
    {"Name": "IA_Vehicle", "ValueType": "Boolean", "Keys": ["F"]},
    {"Name": "IA_Pause", "ValueType": "Boolean", "Keys": ["Escape"]},
    {"Name": "IA_QuickSave", "ValueType": "Boolean", "Keys": ["F5"]},
    {"Name": "IA_QuickLoad", "ValueType": "Boolean", "Keys": ["F9"]}
]

def create_input_action(name, value_type):
    """Crea un InputAction asset"""
    asset_path = f"/Game/Input/Actions/{name}"
    
    # Verificar si ya existe
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    if asset_registry.get_asset_paths_by_name(unreal.Name(name)):
        unreal.log(f"InputAction {name} ya existe")
        return None
    
    # Crear el asset
    package_name = unreal.PackageName(asset_path)
    asset = unreal.AssetToolHelpers.create_asset(
        name, 
        asset_path, 
        unreal.InputAction, 
        unreal.InputActionFactory()
    )
    
    # Configurar el valor type
    if asset:
        editor_asset_lib = unreal.EditorAssetLibrary()
        editor_asset_lib.set_editor_property(asset, "value_type", value_type)
        editor_asset_lib.save_asset(asset)
        unreal.log(f"Creado InputAction: {name}")
    
    return asset

def create_input_mapping_context():
    """Crea el InputMappingContext principal"""
    asset_path = "/Game/Input/Mappings/ND_DefaultContext"
    
    # Verificar si ya existe
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    if asset_registry.get_asset_paths_by_name(unreal.Name("ND_DefaultContext")):
        unreal.log("InputMappingContext ND_DefaultContext ya existe")
        return None
    
    # Crear el asset
    asset = unreal.AssetToolHelpers.create_asset(
        "ND_DefaultContext",
        asset_path,
        unreal.InputMappingContext,
        unreal.InputMappingContextFactory()
    )
    
    if asset:
        editor_asset_lib = unreal.EditorAssetLibrary()
        editor_asset_lib.save_asset(asset)
        unreal.log("Creado InputMappingContext: ND_DefaultContext")
    
    return asset

def map_keys_to_action(context_asset, action_name, keys):
    """Mapea teclas a un InputAction en el contexto"""
    # Esta función requeriría acceso a la API de edición del mapping
    # Por ahora, el usuario deberá hacerlo manualmente o usar un Blueprint
    pass

def main():
    unreal.log("=" * 60)
    unreal.log("GENERANDO ASSETS DE INPUT PARA NEON DISTRICT SANDBOX")
    unreal.log("=" * 60)
    
    # Crear carpeta si no existe
    editor_asset_lib = unreal.EditorAssetLibrary()
    
    # Crear los InputActions
    created_actions = {}
    for action in INPUT_ACTIONS:
        asset = create_input_action(action["Name"], action["ValueType"])
        if asset:
            created_actions[action["Name"]] = asset
    
    # Crear el InputMappingContext
    context = create_input_mapping_context()
    
    unreal.log("=" * 60)
    unreal.log(f"CREADOS {len(created_actions)} InputActions")
    unreal.log("AHORA VE A Content Browser > ND_DefaultContext > Edit Mappings")
    unreal.log("Y mapea las teclas según el archivo INPUT_GUIDE.md")
    unreal.log("=" * 60)

if __name__ == "__main__":
    main()