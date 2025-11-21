// Copyright © 2025 Vladyslav Popushoi. All rights reserved.

#include "Subsystems/InteractiveObjectManagerSubsystem.h"
#include "InteractiveObjectManagerLog.h"

#include "Components/InteractiveObjectComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UInteractiveObjectManagerSubsystem::UInteractiveObjectManagerSubsystem()
    : NextObjectId(1)
    , SelectedObjectId(INDEX_NONE)
{
}

void UInteractiveObjectManagerSubsystem::Deinitialize()
{
    RegisteredObjects.Reset();
    SelectedObjectId = INDEX_NONE;
    SelectedObject.Reset();

    Super::Deinitialize();
}

void UInteractiveObjectManagerSubsystem::RegisterInteractiveObject(UInteractiveObjectComponent* InteractiveComponent)
{
    if (InteractiveComponent == nullptr)
    {
        UE_LOG(LogInteractiveObjectManager, Warning, TEXT("RegisterInteractiveObject called with null component."));
        return;
    }

    CleanupInvalidRecords();

    FInteractiveObjectRecord* ExistingRecord = FindRecordByComponent(InteractiveComponent);
    if (ExistingRecord != nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Log,
            TEXT("Interactive object component '%s' is already registered with Id %d."),
            *InteractiveComponent->GetName(),
            ExistingRecord->ObjectId
        );
        ExistingRecord->Component = InteractiveComponent;
        BroadcastObjectsListChanged();
        return;
    }

    FInteractiveObjectRecord NewRecord;
    NewRecord.ObjectId = NextObjectId++;
    NewRecord.Component = InteractiveComponent;

    RegisteredObjects.Add(MoveTemp(NewRecord));

    UE_LOG(
        LogInteractiveObjectManager,
        Log,
        TEXT("Registered interactive object component '%s' with Id %d."),
        *InteractiveComponent->GetName(),
        RegisteredObjects.Last().ObjectId
    );

    BroadcastObjectsListChanged();
}

void UInteractiveObjectManagerSubsystem::UnregisterInteractiveObject(UInteractiveObjectComponent* InteractiveComponent)
{
    if (InteractiveComponent == nullptr)
    {
        UE_LOG(LogInteractiveObjectManager, Warning, TEXT("UnregisterInteractiveObject called with null component."));
        return;
    }

    CleanupInvalidRecords();

    bool bListChanged = false;
    bool bSelectionChanged = false;

    for (int32 Index = RegisteredObjects.Num() - 1; Index >= 0; --Index)
    {
        FInteractiveObjectRecord& Record = RegisteredObjects[Index];
        if (Record.Component.Get() == InteractiveComponent)
        {
            const int32 RemovedId = Record.ObjectId;

            RegisteredObjects.RemoveAt(Index);
            bListChanged = true;

            if (RemovedId == SelectedObjectId)
            {
                SelectedObjectId = INDEX_NONE;
                SelectedObject.Reset();
                bSelectionChanged = true;
            }

            UE_LOG(
                LogInteractiveObjectManager,
                Log,
                TEXT("Unregistered interactive object component '%s' with Id %d."),
                *InteractiveComponent->GetName(),
                RemovedId
            );

            break;
        }
    }

    if (bListChanged)
    {
        BroadcastObjectsListChanged();
    }

    if (bSelectionChanged)
    {
        BroadcastSelectedObjectChanged();
    }
}

void UInteractiveObjectManagerSubsystem::GetInteractiveObjectsList(TArray<FInteractiveObjectListItem>& OutItems)
{
    CleanupInvalidRecords();

    OutItems.Reset();

    for (const FInteractiveObjectRecord& Record : RegisteredObjects)
    {
        UInteractiveObjectComponent* InteractiveComponent = Record.Component.Get();
        if (InteractiveComponent == nullptr)
        {
            continue;
        }

        AActor* OwnerActor = InteractiveComponent->GetOwner();
        const FString DisplayName = (OwnerActor != nullptr) ? OwnerActor->GetName() : InteractiveComponent->GetName();

        FInteractiveObjectListItem Item;
        Item.Id = Record.ObjectId;
        Item.DisplayName = DisplayName;

        OutItems.Add(MoveTemp(Item));
    }
}

bool UInteractiveObjectManagerSubsystem::SelectObjectById(int32 ObjectId)
{
    CleanupInvalidRecords();
    InvalidateSelectionIfNoLongerValid();

    FInteractiveObjectRecord* Record = FindRecordById(ObjectId);
    if (Record == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("SelectObjectById called with unknown Id %d."),
            ObjectId
        );
        return false;
    }

    UInteractiveObjectComponent* InteractiveComponent = Record->Component.Get();
    if (InteractiveComponent == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("SelectObjectById called for Id %d, but component is no longer valid."),
            ObjectId
        );
        return false;
    }

    if (SelectedObjectId == ObjectId && SelectedObject.Get() == InteractiveComponent)
    {
        // Already selected
        return false;
    }

    SelectedObjectId = ObjectId;
    SelectedObject = InteractiveComponent;

    UE_LOG(
        LogInteractiveObjectManager,
        Log,
        TEXT("Selected interactive object with Id %d ('%s')."),
        ObjectId,
        *InteractiveComponent->GetName()
    );

    BroadcastSelectedObjectChanged();
    return true;
}

bool UInteractiveObjectManagerSubsystem::SelectObjectByIndex(int32 Index)
{
    CleanupInvalidRecords();
    InvalidateSelectionIfNoLongerValid();

    if (!RegisteredObjects.IsValidIndex(Index))
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("SelectObjectByIndex called with invalid index %d."),
            Index
        );
        return false;
    }

    const FInteractiveObjectRecord& Record = RegisteredObjects[Index];
    return SelectObjectById(Record.ObjectId);
}

bool UInteractiveObjectManagerSubsystem::ClearSelection()
{
    InvalidateSelectionIfNoLongerValid();

    if (SelectedObjectId == INDEX_NONE && !SelectedObject.IsValid())
    {
        return false;
    }

    SelectedObjectId = INDEX_NONE;
    SelectedObject.Reset();

    UE_LOG(LogInteractiveObjectManager, Log, TEXT("Cleared interactive object selection."));

    BroadcastSelectedObjectChanged();
    return true;
}

FInteractiveObjectListItem UInteractiveObjectManagerSubsystem::GetSelectedObjectInfo(bool& bOutIsValid) const
{
    bOutIsValid = false;

    FInteractiveObjectListItem Result;

    if (SelectedObjectId == INDEX_NONE)
    {
        return Result;
    }

    UInteractiveObjectComponent* InteractiveComponent = SelectedObject.Get();
    if (InteractiveComponent == nullptr)
    {
        return Result;
    }

    AActor* OwnerActor = InteractiveComponent->GetOwner();
    const FString DisplayName = (OwnerActor != nullptr) ? OwnerActor->GetName() : InteractiveComponent->GetName();

    Result.Id = SelectedObjectId;
    Result.DisplayName = DisplayName;

    bOutIsValid = true;
    return Result;
}

bool UInteractiveObjectManagerSubsystem::SetSelectedObjectColor(const FLinearColor& NewColor)
{
    InvalidateSelectionIfNoLongerValid();

    if (SelectedObjectId == INDEX_NONE)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("SetSelectedObjectColor called but no object is selected.")
        );
        return false;
    }

    UInteractiveObjectComponent* InteractiveComponent = SelectedObject.Get();
    if (InteractiveComponent == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("SetSelectedObjectColor called, but selected component is no longer valid.")
        );
        SelectedObjectId = INDEX_NONE;
        SelectedObject.Reset();
        BroadcastSelectedObjectChanged();
        return false;
    }

    InteractiveComponent->ApplyColor(NewColor);
    return true;
}

bool UInteractiveObjectManagerSubsystem::SetSelectedObjectUniformScale(float NewUniformScale)
{
    InvalidateSelectionIfNoLongerValid();

    if (SelectedObjectId == INDEX_NONE)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("SetSelectedObjectUniformScale called but no object is selected.")
        );
        return false;
    }

    UInteractiveObjectComponent* InteractiveComponent = SelectedObject.Get();
    if (InteractiveComponent == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("SetSelectedObjectUniformScale called, but selected component is no longer valid.")
        );
        SelectedObjectId = INDEX_NONE;
        SelectedObject.Reset();
        BroadcastSelectedObjectChanged();
        return false;
    }

    InteractiveComponent->ApplyScale(NewUniformScale);
    return true;
}

bool UInteractiveObjectManagerSubsystem::DeleteSelectedObject()
{
    InvalidateSelectionIfNoLongerValid();

    if (SelectedObjectId == INDEX_NONE)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("DeleteSelectedObject called but no object is selected.")
        );
        return false;
    }

    UInteractiveObjectComponent* InteractiveComponent = SelectedObject.Get();
    if (InteractiveComponent == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("DeleteSelectedObject called, but selected component is no longer valid.")
        );
        SelectedObjectId = INDEX_NONE;
        SelectedObject.Reset();
        BroadcastSelectedObjectChanged();
        return false;
    }

    AActor* OwnerActor = InteractiveComponent->GetOwner();
    if (OwnerActor == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("DeleteSelectedObject: selected component '%s' has no valid owner actor."),
            *InteractiveComponent->GetName()
        );
        return false;
    }

    const int32 RemovedId = SelectedObjectId;

    UE_LOG(
        LogInteractiveObjectManager,
        Log,
        TEXT("Deleting selected interactive object with Id %d ('%s')."),
        RemovedId,
        *OwnerActor->GetName()
    );

    SelectedObjectId = INDEX_NONE;
    SelectedObject.Reset();
    BroadcastSelectedObjectChanged();

    OwnerActor->Destroy();

    return true;
}

void UInteractiveObjectManagerSubsystem::CleanupInvalidRecords()
{
    for (int32 Index = RegisteredObjects.Num() - 1; Index >= 0; --Index)
    {
        const FInteractiveObjectRecord& Record = RegisteredObjects[Index];
        if (!Record.Component.IsValid())
        {
            UE_LOG(
                LogInteractiveObjectManager,
                Log,
                TEXT("Removing invalid interactive object record with Id %d."),
                Record.ObjectId
            );
            RegisteredObjects.RemoveAt(Index);
        }
    }
}

UInteractiveObjectManagerSubsystem::FInteractiveObjectRecord* UInteractiveObjectManagerSubsystem::FindRecordById(int32 ObjectId)
{
    for (FInteractiveObjectRecord& Record : RegisteredObjects)
    {
        if (Record.ObjectId == ObjectId)
        {
            return &Record;
        }
    }

    return nullptr;
}

UInteractiveObjectManagerSubsystem::FInteractiveObjectRecord* UInteractiveObjectManagerSubsystem::FindRecordByComponent(UInteractiveObjectComponent* InteractiveComponent)
{
    if (InteractiveComponent == nullptr)
    {
        return nullptr;
    }

    for (FInteractiveObjectRecord& Record : RegisteredObjects)
    {
        if (Record.Component.Get() == InteractiveComponent)
        {
            return &Record;
        }
    }

    return nullptr;
}

void UInteractiveObjectManagerSubsystem::InvalidateSelectionIfNoLongerValid()
{
    if (SelectedObjectId == INDEX_NONE)
    {
        return;
    }

    if (!SelectedObject.IsValid())
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Log,
            TEXT("Selected interactive object Id %d is no longer valid. Clearing selection."),
            SelectedObjectId
        );

        SelectedObjectId = INDEX_NONE;
        SelectedObject.Reset();
        BroadcastSelectedObjectChanged();
    }
}

void UInteractiveObjectManagerSubsystem::BroadcastObjectsListChanged()
{
    TArray<FInteractiveObjectListItem> Snapshot;
    GetInteractiveObjectsList(Snapshot);

    OnObjectsListChanged.Broadcast(Snapshot);
}

void UInteractiveObjectManagerSubsystem::BroadcastSelectedObjectChanged()
{
    OnSelectedObjectChanged.Broadcast(SelectedObjectId);
}