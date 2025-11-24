// Copyright © 2025 Vladyslav Popushoi. All rights reserved.

#include "Settings/InteractiveObjectManagerDeveloperSettings.h"
#include "InteractiveObjectManagerLog.h"

#include "GameFramework/Actor.h"

UInteractiveObjectManagerDeveloperSettings::UInteractiveObjectManagerDeveloperSettings()
{
    UE_LOG(
        LogInteractiveObjectManager,
        Log,
        TEXT("UInteractiveObjectManagerDeveloperSettings constructed.")
    );
}

FName UInteractiveObjectManagerDeveloperSettings::GetCategoryName() const
{
    return TEXT("Game");
}