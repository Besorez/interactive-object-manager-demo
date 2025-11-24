// Copyright © 2025 Vladyslav Popushoi. All rights reserved.

#include "Subsystems/InteractiveObjectManagerSubsystem.h"
#include "InteractiveObjectManagerLog.h"

#include "Components/InteractiveObjectComponent.h"
#include "Settings/InteractiveObjectSettings.h"
#include "Settings/InteractiveObjectManagerDeveloperSettings.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"

UInteractiveObjectManagerSubsystem::UInteractiveObjectManagerSubsystem()
    : NextObjectId(1)
    , SelectedObjectId(INDEX_NONE)
{
}

void UInteractiveObjectManagerSubsystem::Deinitialize()
{
    RegisteredObjects.Empty();
    SelectedObject.Reset();
    SelectedObjectId = INDEX_NONE;

    Super::Deinitialize();
}

void UInteractiveObjectManagerSubsystem::SpawnDefaultObject()
{
    UInteractiveObjectSettings* Settings = UInteractiveObjectSettings::Get();
    if (Settings == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("InteractiveObjectManagerSubsystem::SpawnDefaultObject: Settings object is null, using Cube as fallback.")
        );

        SpawnObjectOfType(EInteractiveObjectSpawnType::Cube);
        return;
    }

    const EInteractiveObjectSpawnType DefaultType = Settings->GetDefaultSpawnType();
    SpawnObjectOfType(DefaultType);
}

void UInteractiveObjectManagerSubsystem::SpawnObjectOfType(EInteractiveObjectSpawnType SpawnType)
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("InteractiveObjectManagerSubsystem::SpawnObjectOfType: World is null.")
        );
        return;
    }

    const UInteractiveObjectManagerDeveloperSettings* DeveloperSettings = GetDefault<UInteractiveObjectManagerDeveloperSettings>();
    if (DeveloperSettings == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("InteractiveObjectManagerSubsystem::SpawnObjectOfType: Developer settings are null.")
        );
        return;
    }

    // Resolve cube and sphere classes from developer settings.
    UClass* CubeClass = DeveloperSettings->CubePrimitiveClass.LoadSynchronous();
    UClass* SphereClass = DeveloperSettings->SpherePrimitiveClass.LoadSynchronous();

    if (CubeClass == nullptr && SphereClass == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("InteractiveObjectManagerSubsystem::SpawnObjectOfType: No primitive classes configured in developer settings.")
        );
        return;
    }

    TSubclassOf<AActor> ClassToSpawn = nullptr;

    switch (SpawnType)
    {
    case EInteractiveObjectSpawnType::Cube:
        ClassToSpawn = CubeClass;
        break;

    case EInteractiveObjectSpawnType::Sphere:
        ClassToSpawn = SphereClass;
        break;

    case EInteractiveObjectSpawnType::Random:
    {
        const bool bHasCube = (CubeClass != nullptr);
        const bool bHasSphere = (SphereClass != nullptr);

        if (bHasCube && bHasSphere)
        {
            const bool bChooseCube = FMath::RandBool();
            ClassToSpawn = bChooseCube ? CubeClass : SphereClass;
        }
        else if (bHasCube)
        {
            ClassToSpawn = CubeClass;
        }
        else if (bHasSphere)
        {
            ClassToSpawn = SphereClass;
        }
        break;
    }

    default:
        break;
    }

    if (ClassToSpawn == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("InteractiveObjectManagerSubsystem::SpawnObjectOfType: No class resolved for spawn type %d."),
            static_cast<int32>(SpawnType)
        );
        return;
    }

    // Randomized spawn transform for the demo: around world origin in a small radius.
    const float SpawnRadius = 1000.0f;
    const float SpawnHeight = 100.0f;

    const float AngleRadians = FMath::FRand() * 2.0f * PI;
    const float Distance = FMath::FRandRange(0.0f, SpawnRadius);

    const float SpawnX = FMath::Cos(AngleRadians) * Distance;
    const float SpawnY = FMath::Sin(AngleRadians) * Distance;

    const FVector SpawnLocation(SpawnX, SpawnY, SpawnHeight);
    const FRotator SpawnRotation = FRotator::ZeroRotator;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AActor* NewActor = World->SpawnActor<AActor>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
    if (NewActor == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("InteractiveObjectManagerSubsystem::SpawnObjectOfType: Failed to spawn actor for class '%s'."),
            *GetNameSafe(ClassToSpawn)
        );
        return;
    }

    // Apply default color and scale via interactive component if present.
    UInteractiveObjectComponent* InteractiveComponent = NewActor->FindComponentByClass<UInteractiveObjectComponent>();
    if (InteractiveComponent == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("InteractiveObjectManagerSubsystem::SpawnObjectOfType: Spawned actor '%s' has no UInteractiveObjectComponent."),
            *GetNameSafe(NewActor)
        );
        return;
    }

    if (UInteractiveObjectSettings* Settings = UInteractiveObjectSettings::Get())
    {
        const FInteractiveObjectRuntimeSettings RuntimeSettings = Settings->GetRuntimeSettingsCopy();

        InteractiveComponent->ApplyColor(RuntimeSettings.DefaultColor);
        InteractiveComponent->ApplyScale(RuntimeSettings.DefaultScale.X);
    }
}

void UInteractiveObjectManagerSubsystem::RegisterInteractiveObject(UInteractiveObjectComponent* InteractiveComponent)
{
    if (InteractiveComponent == nullptr)
    {
        return;
    }

    CleanupInvalidRecords();

    // Avoid duplicate registration.
    if (FInteractiveObjectRecord* ExistingRecord = FindRecordByComponent(InteractiveComponent))
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("InteractiveObjectManagerSubsystem: Attempted to register component '%s' that is already registered with Id %d."),
            *GetNameSafe(InteractiveComponent),
            ExistingRecord->ObjectId
        );
        return;
    }

    FInteractiveObjectRecord NewRecord;
    NewRecord.ObjectId = NextObjectId++;
    NewRecord.Component = InteractiveComponent;

    RegisteredObjects.Add(NewRecord);

    UE_LOG(
        LogInteractiveObjectManager,
        Log,
        TEXT("Registered interactive object component '%s' with Id %d."),
        *GetNameSafe(InteractiveComponent),
        NewRecord.ObjectId
    );

    // Autoselect first registered object if nothing is selected yet.
    if (SelectedObjectId == INDEX_NONE)
    {
        SelectedObjectId = NewRecord.ObjectId;
        SelectedObject = InteractiveComponent;
        BroadcastSelectedObjectChanged();
    }

    BroadcastObjectsListChanged();
}

void UInteractiveObjectManagerSubsystem::UnregisterInteractiveObject(UInteractiveObjectComponent* InteractiveComponent)
{
    if (InteractiveComponent == nullptr)
    {
        return;
    }

    CleanupInvalidRecords();

    for (int32 Index = 0; Index < RegisteredObjects.Num(); ++Index)
    {
        FInteractiveObjectRecord& Record = RegisteredObjects[Index];
        if (Record.Component.Get() == InteractiveComponent)
        {
            UE_LOG(
                LogInteractiveObjectManager,
                Log,
                TEXT("Unregistered interactive object component '%s' with Id %d."),
                *GetNameSafe(InteractiveComponent),
                Record.ObjectId
            );

            const int32 RemovedId = Record.ObjectId;

            RegisteredObjects.RemoveAt(Index);

            if (SelectedObjectId == RemovedId)
            {
                SelectedObjectId = INDEX_NONE;
                SelectedObject.Reset();
                BroadcastSelectedObjectChanged();
            }

            BroadcastObjectsListChanged();
            return;
        }
    }
}

void UInteractiveObjectManagerSubsystem::GetInteractiveObjectsList(TArray<FInteractiveObjectListItem>& OutItems)
{
    CleanupInvalidRecords();

    OutItems.Reset();
    OutItems.Reserve(RegisteredObjects.Num());

    for (const FInteractiveObjectRecord& Record : RegisteredObjects)
    {
        UInteractiveObjectComponent* InteractiveComponent = Record.Component.Get();
        if (InteractiveComponent == nullptr)
        {
            continue;
        }

        AActor* OwnerActor = InteractiveComponent->GetOwner();
        const FString DisplayName = (OwnerActor != nullptr) ? OwnerActor->GetName() : FString(TEXT("Unknown"));

        FInteractiveObjectListItem ListItem;
        ListItem.Id = Record.ObjectId;
        ListItem.DisplayName = InteractiveComponent->GetDisplayNameForUI();

        OutItems.Add(ListItem);
    }
}

bool UInteractiveObjectManagerSubsystem::SelectObjectById(int32 ObjectId)
{
    if (ObjectId == SelectedObjectId)
    {
        return false;
    }

    CleanupInvalidRecords();

    if (FInteractiveObjectRecord* Record = FindRecordById(ObjectId))
    {
        SelectedObjectId = Record->ObjectId;
        SelectedObject = Record->Component;

        BroadcastSelectedObjectChanged();
        return true;
    }

    return false;
}

bool UInteractiveObjectManagerSubsystem::SelectObjectByIndex(int32 Index)
{
    CleanupInvalidRecords();

    if (!RegisteredObjects.IsValidIndex(Index))
    {
        return false;
    }

    FInteractiveObjectRecord& Record = RegisteredObjects[Index];
    if (!Record.Component.IsValid())
    {
        return false;
    }

    if (Record.ObjectId == SelectedObjectId)
    {
        return false;
    }

    SelectedObjectId = Record.ObjectId;
    SelectedObject = Record.Component;

    BroadcastSelectedObjectChanged();
    return true;
}

bool UInteractiveObjectManagerSubsystem::ClearSelection()
{
    if (SelectedObjectId == INDEX_NONE && !SelectedObject.IsValid())
    {
        return false;
    }

    SelectedObjectId = INDEX_NONE;
    SelectedObject.Reset();

    BroadcastSelectedObjectChanged();
    return true;
}

FInteractiveObjectListItem UInteractiveObjectManagerSubsystem::GetSelectedObjectInfo(bool& bOutIsValid) const
{
    bOutIsValid = false;

    FInteractiveObjectListItem Result;
    Result.Id = INDEX_NONE;

    if (SelectedObjectId == INDEX_NONE || !SelectedObject.IsValid())
    {
        return Result;
    }

    UInteractiveObjectComponent* InteractiveComponent = SelectedObject.Get();
    if (InteractiveComponent == nullptr)
    {
        return Result;
    }

    AActor* OwnerActor = InteractiveComponent->GetOwner();
    const FString DisplayName = (OwnerActor != nullptr) ? OwnerActor->GetName() : FString(TEXT("Unknown"));

    Result.Id = SelectedObjectId;
    Result.DisplayName = DisplayName;

    bOutIsValid = true;
    return Result;
}

//bool UInteractiveObjectManagerSubsystem::GetSelectedObjectProperties(FLinearColor& OutColor, float& OutUniformScale) const
//{
//    OutColor = FLinearColor::White;
//    OutUniformScale = 1.0f;
//
//    if (!SelectedObject.IsValid())
//    {
//        UE_LOG(
//            LogInteractiveObjectManager,
//            Log,
//            TEXT("GetSelectedObjectProperties: no selected object.")
//        );
//        return false;
//    }
//
//    UInteractiveObjectComponent* InteractiveComponent = SelectedObject.Get();
//    if (InteractiveComponent == nullptr)
//    {
//        UE_LOG(
//            LogInteractiveObjectManager,
//            Warning,
//            TEXT("GetSelectedObjectProperties: selected object pointer is invalid.")
//        );
//        return false;
//    }
//
//    OutColor = InteractiveComponent->GetCurrentColor();
//    OutUniformScale = InteractiveComponent->GetCurrentScale();
//    return true;
//}

void UInteractiveObjectManagerSubsystem::GetSelectedObjectVisualState(bool& bOutHasSelection, FLinearColor& OutColor, float& OutScale) const
{
    bOutHasSelection = false;

    // Default fallback: safe defaults from settings or hardcoded values.
    FInteractiveObjectRuntimeSettings RuntimeDefaults;
    RuntimeDefaults.ApplySafeDefaults();

    if (const UInteractiveObjectSettings* Settings = UInteractiveObjectSettings::Get())
    {
        RuntimeDefaults = Settings->GetRuntimeSettingsCopy();
    }

    OutColor = RuntimeDefaults.DefaultColor;
    OutScale = RuntimeDefaults.DefaultScale.X;

    if (SelectedObjectId == INDEX_NONE || !SelectedObject.IsValid())
    {
        return;
    }

    UInteractiveObjectComponent* InteractiveComponent = SelectedObject.Get();
    if (InteractiveComponent == nullptr)
    {
        return;
    }

    bOutHasSelection = true;
    OutColor = InteractiveComponent->GetCurrentColor();
    OutScale = InteractiveComponent->GetCurrentScale();
}

bool UInteractiveObjectManagerSubsystem::SetSelectedObjectColor(const FLinearColor& NewColor)
{
    if (!SelectedObject.IsValid())
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Log,
            TEXT("SetSelectedObjectColor: no selected object.")
        );
        return false;
    }

    UInteractiveObjectComponent* InteractiveComponent = SelectedObject.Get();
    if (InteractiveComponent == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("SetSelectedObjectColor: selected object pointer is invalid.")
        );
        return false;
    }

    InteractiveComponent->ApplyColor(NewColor);
    return true;
}

bool UInteractiveObjectManagerSubsystem::SetSelectedObjectUniformScale(float NewUniformScale)
{
    if (!SelectedObject.IsValid())
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Log,
            TEXT("SetSelectedObjectUniformScale: no selected object.")
        );
        return false;
    }

    UInteractiveObjectComponent* InteractiveComponent = SelectedObject.Get();
    if (InteractiveComponent == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("SetSelectedObjectUniformScale: selected object pointer is invalid.")
        );
        return false;
    }

    InteractiveComponent->ApplyScale(NewUniformScale);
    return true;
}

bool UInteractiveObjectManagerSubsystem::DeleteSelectedObject()
{
    if (!SelectedObject.IsValid())
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Log,
            TEXT("DeleteSelectedObject: no selected object.")
        );
        return false;
    }

    UInteractiveObjectComponent* InteractiveComponent = SelectedObject.Get();
    AActor* OwnerActor = (InteractiveComponent != nullptr) ? InteractiveComponent->GetOwner() : nullptr;
    const int32 ObjectIdToRemove = SelectedObjectId;

    SelectedObjectId = INDEX_NONE;
    SelectedObject.Reset();

    int32 RemovedIndex = INDEX_NONE;

    for (int32 Index = 0; Index < RegisteredObjects.Num(); ++Index)
    {
        if (RegisteredObjects[Index].ObjectId == ObjectIdToRemove)
        {
            RegisteredObjects.RemoveAt(Index);
            RemovedIndex = Index;
            break;
        }
    }

    if (OwnerActor != nullptr)
    {
        OwnerActor->Destroy();
    }

    if (RegisteredObjects.Num() > 0)
    {
        const FInteractiveObjectRecord& FirstRecord = RegisteredObjects[0];
        SelectedObjectId = FirstRecord.ObjectId;
        SelectedObject = FirstRecord.Component;
    }

    BroadcastObjectsListChanged();
    BroadcastSelectedObjectChanged();

    const bool bSuccess = (RemovedIndex != INDEX_NONE);

    UE_LOG(
        LogInteractiveObjectManager,
        Log,
        TEXT("DeleteSelectedObject: Id %d, success = %s."),
        ObjectIdToRemove,
        bSuccess ? TEXT("true") : TEXT("false")
    );

    return bSuccess;
}

void UInteractiveObjectManagerSubsystem::CleanupInvalidRecords()
{
    for (int32 Index = RegisteredObjects.Num() - 1; Index >= 0; --Index)
    {
        if (!RegisteredObjects[Index].Component.IsValid())
        {
            RegisteredObjects.RemoveAt(Index);
        }
    }

    InvalidateSelectionIfNoLongerValid();
}

UInteractiveObjectManagerSubsystem::FInteractiveObjectRecord* UInteractiveObjectManagerSubsystem::FindRecordById(int32 ObjectId)
{
    for (FInteractiveObjectRecord& Record : RegisteredObjects)
    {
        if (Record.ObjectId == ObjectId && Record.Component.IsValid())
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
        SelectedObjectId = INDEX_NONE;
        SelectedObject.Reset();
        BroadcastSelectedObjectChanged();
        return;
    }

    bool bFound = false;
    for (const FInteractiveObjectRecord& Record : RegisteredObjects)
    {
        if (Record.ObjectId == SelectedObjectId && Record.Component == SelectedObject)
        {
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        SelectedObjectId = INDEX_NONE;
        SelectedObject.Reset();
        BroadcastSelectedObjectChanged();
    }
}

void UInteractiveObjectManagerSubsystem::BroadcastObjectsListChanged()
{
    TArray<FInteractiveObjectListItem> Items;
    GetInteractiveObjectsList(Items);

    OnObjectsListChanged.Broadcast(Items);
}

void UInteractiveObjectManagerSubsystem::BroadcastSelectedObjectChanged()
{
    OnSelectedObjectChanged.Broadcast(SelectedObjectId);
}
