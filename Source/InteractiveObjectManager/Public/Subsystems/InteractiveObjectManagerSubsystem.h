// Copyright © 2025 Vladyslav Popushoi. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "InteractiveObjectManagerTypes.h"
#include "InteractiveObjectManagerSubsystem.generated.h"

class UInteractiveObjectComponent;

/**
 * Lightweight item used by UI to present interactive objects.
 */
USTRUCT(BlueprintType)
struct FInteractiveObjectListItem
{
    GENERATED_BODY()

    /** Runtime Id assigned by the subsystem. */
    UPROPERTY(BlueprintReadOnly, Category = "InteractiveObjectManager")
    int32 Id = INDEX_NONE;

    /** Human readable display name for UI. */
    UPROPERTY(BlueprintReadOnly, Category = "InteractiveObjectManager")
    FString DisplayName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractiveObjectsListChangedDynamic, const TArray<FInteractiveObjectListItem>&, Objects);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSelectedInteractiveObjectChangedDynamic, int32, SelectedObjectId);

/**
 * World level subsystem that keeps track of all interactive objects in a world
 * and exposes a simple selection and operation API for UI.
 *
 * Primitive actor classes (cube, sphere) are resolved from
 * UInteractiveObjectManagerDeveloperSettings in Project Settings.
 */
UCLASS()
class INTERACTIVEOBJECTMANAGER_API UInteractiveObjectManagerSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UInteractiveObjectManagerSubsystem();

    // UWorldSubsystem interface
    virtual void Deinitialize() override;

    /**
     * Spawns a new interactive primitive using default settings.
     *
     * Default spawn type is taken from UInteractiveObjectSettings.
     * Primitive actor class is resolved from developer settings.
     */
    UFUNCTION(BlueprintCallable, Category = "InteractiveObjectManager")
    void SpawnDefaultObject();

    /**
     * Spawns a new interactive primitive of the given type.
     *
     * Cube or sphere actor class is resolved from developer settings
     * (no hardcoded asset paths in code).
     */
    UFUNCTION(BlueprintCallable, Category = "InteractiveObjectManager")
    void SpawnObjectOfType(EInteractiveObjectSpawnType SpawnType);

    /** Registers an interactive object component in this world. */
    void RegisterInteractiveObject(UInteractiveObjectComponent* InteractiveComponent);

    /** Unregisters an interactive object component from this world. */
    void UnregisterInteractiveObject(UInteractiveObjectComponent* InteractiveComponent);

    /** Returns a lightweight snapshot of all interactive objects for UI. */
    UFUNCTION(BlueprintCallable, Category = "InteractiveObjectManager")
    void GetInteractiveObjectsList(TArray<FInteractiveObjectListItem>& OutItems);

    /** Selects an object by its runtime Id. Returns true if selection changed. */
    UFUNCTION(BlueprintCallable, Category = "InteractiveObjectManager")
    bool SelectObjectById(int32 ObjectId);

    /** Selects an object by its index in the current list. Mostly for simple debug cases. */
    UFUNCTION(BlueprintCallable, Category = "InteractiveObjectManager")
    bool SelectObjectByIndex(int32 Index);

    /** Clears current selection. Returns true if there was a selection before. */
    UFUNCTION(BlueprintCallable, Category = "InteractiveObjectManager")
    bool ClearSelection();

    /** Returns info about currently selected object, if any. */
    UFUNCTION(BlueprintPure, Category = "InteractiveObjectManager")
    FInteractiveObjectListItem GetSelectedObjectInfo(bool& bOutIsValid) const;

    /** Sets color on the currently selected object. Returns true if operation succeeded. */
    UFUNCTION(BlueprintCallable, Category = "InteractiveObjectManager")
    bool SetSelectedObjectColor(const FLinearColor& NewColor);

    /** Sets uniform scale on the currently selected object. Returns true if operation succeeded. */
    UFUNCTION(BlueprintCallable, Category = "InteractiveObjectManager")
    bool SetSelectedObjectUniformScale(float NewUniformScale);

    /** Destroys the currently selected object. Returns true if an object was removed. */
    UFUNCTION(BlueprintCallable, Category = "InteractiveObjectManager")
    bool DeleteSelectedObject();

    /** Fired whenever the list of interactive objects changes. */
    UPROPERTY(BlueprintAssignable, Category = "InteractiveObjectManager")
    FInteractiveObjectsListChangedDynamic OnObjectsListChanged;

    /** Fired whenever the selected object changes. SelectedObjectId can be INDEX_NONE. */
    UPROPERTY(BlueprintAssignable, Category = "InteractiveObjectManager")
    FSelectedInteractiveObjectChangedDynamic OnSelectedObjectChanged;

private:
    struct FInteractiveObjectRecord
    {
        int32 ObjectId = INDEX_NONE;
        TWeakObjectPtr<UInteractiveObjectComponent> Component;
    };

    /** Next runtime Id to assign to a newly registered object. */
    int32 NextObjectId;

    /** Id of the currently selected interactive object, or INDEX_NONE if none is selected. */
    int32 SelectedObjectId;

    /** Weak pointer to the currently selected component. */
    TWeakObjectPtr<UInteractiveObjectComponent> SelectedObject;

    /** All interactive objects registered in this world. */
    TArray<FInteractiveObjectRecord> RegisteredObjects;

    void CleanupInvalidRecords();

    FInteractiveObjectRecord* FindRecordById(int32 ObjectId);
    FInteractiveObjectRecord* FindRecordByComponent(UInteractiveObjectComponent* InteractiveComponent);

    void InvalidateSelectionIfNoLongerValid();
    void BroadcastObjectsListChanged();
    void BroadcastSelectedObjectChanged();
};