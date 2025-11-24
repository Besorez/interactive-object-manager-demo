// Copyright © 2025 Vladyslav Popushoi. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InteractiveObjectManagerTypes.h"
#include "InteractiveObjectManagerDeveloperSettings.generated.h"

class AActor;

/**
 * Editor facing settings for the Interactive Object Manager.
 *
 * These settings are exposed in Project Settings under:
 * Project Settings - Game - Interactive Object Manager
 *
 * Responsibilities:
 * - Allow designers to choose which actor classes are used for cube and sphere primitives.
 * - Keep configuration in config files without hard coded asset paths.
 *
 * Runtime defaults for spawn type, color and scale are still handled by UInteractiveObjectSettings.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Interactive Object Manager"))
class INTERACTIVEOBJECTMANAGER_API UInteractiveObjectManagerDeveloperSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UInteractiveObjectManagerDeveloperSettings();

    // UDeveloperSettings interface

    /** Places this settings object under the "Game" category in Project Settings. */
    virtual FName GetCategoryName() const override;

    /**
     * Actor class used as a cube primitive in the demo.
     *
     * Expected to reference a Blueprint such as BP_InteractiveCube
     * that already has UInteractiveObjectComponent attached and configured.
     */
    UPROPERTY(EditAnywhere, Config, Category = "Primitives", meta = (ToolTip = "Actor class used to represent cube primitives in the demo. Should have UInteractiveObjectComponent attached."))
    TSoftClassPtr<AActor> CubePrimitiveClass;

    /**
     * Actor class used as a sphere primitive in the demo.
     *
     * Expected to reference a Blueprint such as BP_InteractiveSphere
     * that already has UInteractiveObjectComponent attached and configured.
     */
    UPROPERTY(EditAnywhere, Config, Category = "Primitives", meta = (ToolTip = "Actor class used to represent sphere primitives in the demo. Should have UInteractiveObjectComponent attached."))
    TSoftClassPtr<AActor> SpherePrimitiveClass;
};
