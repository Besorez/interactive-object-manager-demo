// Copyright © 2025 Vladyslav Popushoi. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogInteractiveObjectManager, Log, All);

/**
 * Runtime module for the Interactive Object Manager technical assessment.
 *
 * Responsibilities:
 * - Provide a dedicated runtime module entry point for the Interactive Object Manager feature set.
 * - Own and initialize core systems responsible for managing selectable and manipulable actors.
 * - Expose a clear separation between engine level module startup and higher level gameplay logic.
 *
 * This module is intentionally kept thin. It should only contain:
 * - Module lifecycle wiring (StartupModule, ShutdownModule).
 * - High level logging and basic sanity checks.
 * - Registration or bootstrap of subsystems that live in this module.
 *
 * Detailed gameplay logic, UI integration and configuration handling should be implemented
 * in dedicated classes and subsystems within this module, not directly in the module class.
 */
class FInteractiveObjectManagerModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation. Called when the module is loaded into memory. */
	virtual void StartupModule() override;

	/** IModuleInterface implementation. Called before the module is unloaded from memory. */
	virtual void ShutdownModule() override;
};