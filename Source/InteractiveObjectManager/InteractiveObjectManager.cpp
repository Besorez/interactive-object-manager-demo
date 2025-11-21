// Copyright © 2025 Vladyslav Popushoi. All rights reserved.

#include "InteractiveObjectManager.h"

#include "Settings/InteractiveObjectSettings.h"

IMPLEMENT_MODULE(FInteractiveObjectManagerModule, InteractiveObjectManager)

void FInteractiveObjectManagerModule::StartupModule()
{
	UE_LOG(LogInteractiveObjectManager, Log, TEXT("InteractiveObjectManager module startup - initializing runtime systems"));

	// Initialize module level settings from ini.
	// This ensures that any system depending on UInteractiveObjectSettings
	// can safely query validated runtime values after module startup.
	if (UInteractiveObjectSettings* Settings = UInteractiveObjectSettings::Get())
	{
		Settings->LoadFromConfig();
		Settings->ApplyDefaultsIfInvalid();
	}

	// Note:
	// Keep this method focused on high level initialization only.
	// Detailed object management, UI wiring and configuration handling
	// should be delegated to dedicated classes within this module, not directly here.
}

void FInteractiveObjectManagerModule::ShutdownModule()
{
	UE_LOG(LogInteractiveObjectManager, Log, TEXT("InteractiveObjectManager module shutdown - releasing runtime systems"));

	// Note:
	// Use this method to perform any explicit teardown if needed.
	// In many cases Unreal will handle object lifetime automatically,
	// but explicit shutdown is a good place for deterministic cleanup.
}