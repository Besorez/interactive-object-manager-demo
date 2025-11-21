#include "InteractiveObjectManager.h"

#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogInteractiveObjectManager);

IMPLEMENT_MODULE(FInteractiveObjectManagerModule, InteractiveObjectManager)

void FInteractiveObjectManagerModule::StartupModule()
{
	UE_LOG(LogInteractiveObjectManager, Log, TEXT("InteractiveObjectManager module startup - initializing runtime systems"));
	// Note:
	// Keep this method focused on high level initialization only.
	// Detailed object management, UI wiring and configuration handling
	// should be delegated to dedicated classes within this module.
}

void FInteractiveObjectManagerModule::ShutdownModule()
{
	UE_LOG(LogInteractiveObjectManager, Log, TEXT("InteractiveObjectManager module shutdown - releasing runtime systems"));
	// Note:
	// Use this method to perform any explicit teardown if needed.
	// In many cases Unreal will handle object lifetime automatically,
	// but explicit shutdown is a good place for deterministic cleanup.
}
