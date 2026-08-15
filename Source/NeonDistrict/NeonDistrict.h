// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * NeonDistrict module. Provides the core gameplay framework including
 * GameInstance, WorldBuilder, Mission/Wanted systems, and character/vehicle gameplay.
 */
class FNeonDistrictModule : public IModuleInterface
{
public:
	/** Called when the module is loaded and ready to use */
	virtual void StartupModule() override {}

	/** Called before the module is unloaded. */
	virtual void ShutdownModule() override {}
};