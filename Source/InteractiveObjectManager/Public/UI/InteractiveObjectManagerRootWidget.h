// Copyright © 2025 Vladyslav Popushoi. All rights reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "InteractiveObjectManagerTypes.h"
#include "InteractiveObjectManagerRootWidget.generated.h"

class UInteractiveObjectManagerSubsystem;
struct FInteractiveObjectSettingsViewData;

/**
 * Root CommonUI widget for the Interactive Object Manager demo.
 *
 * Responsibilities:
 * - Connects to UInteractiveObjectManagerSubsystem.
 * - Listens for list and selection changes.
 * - Bridges subsystem data to Blueprint via events and simple request functions.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class INTERACTIVEOBJECTMANAGER_API UInteractiveObjectManagerRootWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    UInteractiveObjectManagerRootWidget();

    /**
     * Called from Main tab when user presses Spawn (default) button.
     * Asks the manager subsystem to spawn an object using default settings.
     */
    UFUNCTION(BlueprintCallable, Category = "InteractiveObjectManager")
    void RequestSpawnDefaultObject();

    /**
     * Called from Main tab when user presses Spawn Cube or Spawn Sphere buttons.
     * Asks the manager subsystem to spawn an object of the given primitive type.
     */
    UFUNCTION(BlueprintCallable, Category = "InteractiveObjectManager")
    void RequestSpawnObjectOfType(EInteractiveObjectSpawnType SpawnType);

    /**
     * Called from list entry widgets when user clicks an object in the list.
     * Forwards selection request to the manager subsystem.
     */
    UFUNCTION(BlueprintCallable, Category = "InteractiveObjectManager")
    void RequestSelectObjectById(int32 ObjectId);

    /**
     * Called from Main tab when user presses Apply color button.
     * Forwards color request to the manager subsystem for the currently selected object.
     */
    UFUNCTION(BlueprintCallable, Category = "InteractiveObjectManager")
    void RequestApplyColor(const FLinearColor& NewColor);

    /**
     * Called from Main tab when user presses Apply scale button.
     * Forwards uniform scale request to the manager subsystem for the currently selected object.
     */
    UFUNCTION(BlueprintCallable, Category = "InteractiveObjectManager")
    void RequestApplyScale(float NewUniformScale);

    /**
     * Called from Main tab when user presses Delete selected button.
     * Asks the manager subsystem to delete the currently selected object.
     */
    UFUNCTION(BlueprintCallable, Category = "InteractiveObjectManager")
    void RequestDeleteSelectedObject();

    /**
     * Called from Settings tab when it is activated or needs to refresh values.
     * Fills OutSettings with the current runtime settings for UI.
     */
    UFUNCTION(BlueprintCallable, Category = "InteractiveObjectManager|Settings")
    void GetCurrentSettings(FInteractiveObjectSettingsViewData& OutSettings);

    /**
     * Called from Settings tab when user presses Apply or Save.
     * Updates runtime settings from UI values without touching the ini file.
     */
    UFUNCTION(BlueprintCallable, Category = "InteractiveObjectManager|Settings")
    void ApplySettingsFromUI(const FInteractiveObjectSettingsViewData& NewSettings);

    /**
     * Called from Settings tab when user presses Save.
     * Saves current validated runtime settings to the ini file.
     */
    UFUNCTION(BlueprintCallable, Category = "InteractiveObjectManager|Settings")
    void SaveSettingsToIni();

protected:
    // We use Construct/Destruct instead of OnActivated/OnDeactivated,
    // because CommonUI activation requires CommonGameViewportClient,
    // which is not configured in this demo project.
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /**
     * Called whenever the list of interactive objects changes.
     * Blueprint is expected to rebuild the visual list (ScrollBox/ListView) from this data.
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "InteractiveObjectManager")
    void OnObjectsListUpdated(const TArray<FInteractiveObjectListItem>& Objects);

    /**
     * Called whenever current selection changes.
     * If bHasSelection is false, SelectedObjectId will be INDEX_NONE and SelectedDisplayName can be "None".
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "InteractiveObjectManager")
    void OnSelectedObjectInfoUpdated(bool bHasSelection, int32 SelectedObjectId, const FText& SelectedDisplayName);

private:
    /** Cached pointer to the world subsystem. */
    TWeakObjectPtr<UInteractiveObjectManagerSubsystem> ManagerSubsystem;

    /** Delegate handler for list changes. */
    UFUNCTION()
    void HandleObjectsListChanged(const TArray<FInteractiveObjectListItem>& Objects);

    /** Delegate handler for selection changes. */
    UFUNCTION()
    void HandleSelectedObjectChanged(int32 SelectedObjectId);

    /** Performs an initial sync from the subsystem when the widget is constructed. */
    void SynchronizeInitialState();
};