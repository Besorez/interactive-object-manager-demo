// Copyright © 2025 Vladyslav Popushoi. All rights reserved.

#include "UI/InteractiveObjectManagerRootWidget.h"

#include "InteractiveObjectManagerLog.h"
#include "Settings/InteractiveObjectSettings.h"
#include "Subsystems/InteractiveObjectManagerSubsystem.h"

#include "Engine/World.h"

UInteractiveObjectManagerRootWidget::UInteractiveObjectManagerRootWidget()
{
}

void UInteractiveObjectManagerRootWidget::RequestSpawnDefaultObject()
{
    if (!ManagerSubsystem.IsValid())
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("InteractiveObjectManagerRootWidget::RequestSpawnDefaultObject called but manager subsystem is not valid.")
        );
        return;
    }

    UInteractiveObjectManagerSubsystem* Subsystem = ManagerSubsystem.Get();
    if (Subsystem == nullptr)
    {
        return;
    }

    Subsystem->SpawnDefaultObject();
}

void UInteractiveObjectManagerRootWidget::RequestSpawnObjectOfType(EInteractiveObjectSpawnType SpawnType)
{
    if (!ManagerSubsystem.IsValid())
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("InteractiveObjectManagerRootWidget::RequestSpawnObjectOfType called but manager subsystem is not valid.")
        );
        return;
    }

    UInteractiveObjectManagerSubsystem* Subsystem = ManagerSubsystem.Get();
    if (Subsystem == nullptr)
    {
        return;
    }

    Subsystem->SpawnObjectOfType(SpawnType);
}

void UInteractiveObjectManagerRootWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("InteractiveObjectManagerRootWidget: NativeConstruct called but World is null.")
        );
        return;
    }

    UInteractiveObjectManagerSubsystem* Subsystem = World->GetSubsystem<UInteractiveObjectManagerSubsystem>();
    if (Subsystem == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("InteractiveObjectManagerRootWidget: Could not find UInteractiveObjectManagerSubsystem in this World.")
        );
        return;
    }

    ManagerSubsystem = Subsystem;

    Subsystem->OnObjectsListChanged.AddDynamic(
        this,
        &UInteractiveObjectManagerRootWidget::HandleObjectsListChanged
    );

    Subsystem->OnSelectedObjectChanged.AddDynamic(
        this,
        &UInteractiveObjectManagerRootWidget::HandleSelectedObjectChanged
    );

    UE_LOG(
        LogInteractiveObjectManager,
        Log,
        TEXT("InteractiveObjectManagerRootWidget: Connected to manager subsystem in NativeConstruct and subscribed to delegates.")
    );

    SynchronizeInitialState();
}

void UInteractiveObjectManagerRootWidget::NativeDestruct()
{
    if (ManagerSubsystem.IsValid())
    {
        UInteractiveObjectManagerSubsystem* Subsystem = ManagerSubsystem.Get();
        if (Subsystem != nullptr)
        {
            Subsystem->OnObjectsListChanged.RemoveDynamic(
                this,
                &UInteractiveObjectManagerRootWidget::HandleObjectsListChanged
            );

            Subsystem->OnSelectedObjectChanged.RemoveDynamic(
                this,
                &UInteractiveObjectManagerRootWidget::HandleSelectedObjectChanged
            );

            UE_LOG(
                LogInteractiveObjectManager,
                Log,
                TEXT("InteractiveObjectManagerRootWidget: Unsubscribed from manager subsystem in NativeDestruct.")
            );
        }
    }

    ManagerSubsystem.Reset();

    Super::NativeDestruct();
}

void UInteractiveObjectManagerRootWidget::RequestSelectObjectById(int32 ObjectId)
{
    if (!ManagerSubsystem.IsValid())
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("InteractiveObjectManagerRootWidget::RequestSelectObjectById called but manager subsystem is not valid.")
        );
        return;
    }

    UInteractiveObjectManagerSubsystem* Subsystem = ManagerSubsystem.Get();
    if (Subsystem == nullptr)
    {
        return;
    }

    const bool bSelectionChanged = Subsystem->SelectObjectById(ObjectId);
    if (!bSelectionChanged)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Log,
            TEXT("InteractiveObjectManagerRootWidget::RequestSelectObjectById did not change selection for Id %d."),
            ObjectId
        );
    }
}

void UInteractiveObjectManagerRootWidget::HandleObjectsListChanged(const TArray<FInteractiveObjectListItem>& Objects)
{
    OnObjectsListUpdated(Objects);
}

void UInteractiveObjectManagerRootWidget::HandleSelectedObjectChanged(int32 SelectedObjectId)
{
    if (!ManagerSubsystem.IsValid())
    {
        const FText NoneText = FText::FromString(TEXT("None"));
        OnSelectedObjectInfoUpdated(false, INDEX_NONE, NoneText);
        return;
    }

    UInteractiveObjectManagerSubsystem* Subsystem = ManagerSubsystem.Get();
    if (Subsystem == nullptr)
    {
        const FText NoneText = FText::FromString(TEXT("None"));
        OnSelectedObjectInfoUpdated(false, INDEX_NONE, NoneText);
        return;
    }

    bool bHasSelection = false;
    const FInteractiveObjectListItem SelectedItem = Subsystem->GetSelectedObjectInfo(bHasSelection);

    if (!bHasSelection)
    {
        const FText NoneText = FText::FromString(TEXT("None"));
        OnSelectedObjectInfoUpdated(false, INDEX_NONE, NoneText);
    }
    else
    {
        const FText DisplayNameText = FText::FromString(SelectedItem.DisplayName);
        OnSelectedObjectInfoUpdated(true, SelectedItem.Id, DisplayNameText);
    }
}

void UInteractiveObjectManagerRootWidget::SynchronizeInitialState()
{
    if (!ManagerSubsystem.IsValid())
    {
        return;
    }

    UInteractiveObjectManagerSubsystem* Subsystem = ManagerSubsystem.Get();
    if (Subsystem == nullptr)
    {
        return;
    }

    // Initial list snapshot.
    TArray<FInteractiveObjectListItem> Items;
    Subsystem->GetInteractiveObjectsList(Items);
    OnObjectsListUpdated(Items);

    // Initial selection snapshot.
    bool bHasSelection = false;
    const FInteractiveObjectListItem SelectedItem = Subsystem->GetSelectedObjectInfo(bHasSelection);

    if (!bHasSelection)
    {
        const FText NoneText = FText::FromString(TEXT("None"));
        OnSelectedObjectInfoUpdated(false, INDEX_NONE, NoneText);
    }
    else
    {
        const FText DisplayNameText = FText::FromString(SelectedItem.DisplayName);
        OnSelectedObjectInfoUpdated(true, SelectedItem.Id, DisplayNameText);
    }
}

void UInteractiveObjectManagerRootWidget::RequestApplyColor(const FLinearColor& NewColor)
{
    if (!ManagerSubsystem.IsValid())
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("InteractiveObjectManagerRootWidget::RequestApplyColor called but manager subsystem is not valid.")
        );
        return;
    }

    UInteractiveObjectManagerSubsystem* Subsystem = ManagerSubsystem.Get();
    if (Subsystem == nullptr)
    {
        return;
    }

    const bool bApplied = Subsystem->SetSelectedObjectColor(NewColor);
    if (!bApplied)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Log,
            TEXT("InteractiveObjectManagerRootWidget::RequestApplyColor did not apply color, probably no object is selected.")
        );
    }
}

void UInteractiveObjectManagerRootWidget::RequestApplyScale(float NewUniformScale)
{
    if (!ManagerSubsystem.IsValid())
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("InteractiveObjectManagerRootWidget::RequestApplyScale called but manager subsystem is not valid.")
        );
        return;
    }

    UInteractiveObjectManagerSubsystem* Subsystem = ManagerSubsystem.Get();
    if (Subsystem == nullptr)
    {
        return;
    }

    const bool bApplied = Subsystem->SetSelectedObjectUniformScale(NewUniformScale);
    if (!bApplied)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Log,
            TEXT("InteractiveObjectManagerRootWidget::RequestApplyScale did not apply scale, probably no object is selected.")
        );
    }
}

void UInteractiveObjectManagerRootWidget::RequestDeleteSelectedObject()
{
    if (!ManagerSubsystem.IsValid())
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("InteractiveObjectManagerRootWidget::RequestDeleteSelectedObject called but manager subsystem is not valid.")
        );
        return;
    }

    UInteractiveObjectManagerSubsystem* Subsystem = ManagerSubsystem.Get();
    if (Subsystem == nullptr)
    {
        return;
    }

    const bool bDeleted = Subsystem->DeleteSelectedObject();
    if (!bDeleted)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Log,
            TEXT("InteractiveObjectManagerRootWidget::RequestDeleteSelectedObject did not delete anything, probably no object is selected.")
        );
    }
}

void UInteractiveObjectManagerRootWidget::GetCurrentSettings(FInteractiveObjectSettingsViewData& OutSettings)
{
    UInteractiveObjectSettings* Settings = UInteractiveObjectSettings::Get();
    if (Settings == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Error,
            TEXT("InteractiveObjectManagerRootWidget::GetCurrentSettings: Settings object is null. Using hardcoded defaults.")
        );

        FInteractiveObjectRuntimeSettings RuntimeDefaults;
        RuntimeDefaults.ApplySafeDefaults();

        OutSettings.DefaultSpawnType = RuntimeDefaults.DefaultSpawnType;
        OutSettings.DefaultColor = RuntimeDefaults.DefaultColor;
        OutSettings.DefaultUniformScale = RuntimeDefaults.DefaultScale.X;

        return;
    }

    Settings->ToViewData(OutSettings);
}

void UInteractiveObjectManagerRootWidget::ApplySettingsFromUI(const FInteractiveObjectSettingsViewData& NewSettings)
{
    UInteractiveObjectSettings* Settings = UInteractiveObjectSettings::Get();
    if (Settings == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("InteractiveObjectManagerRootWidget::ApplySettingsFromUI: Settings object is null. Changes will be ignored.")
        );
        return;
    }

    Settings->UpdateFromViewData(NewSettings);
    Settings->ApplyDefaultsIfInvalid();
}

void UInteractiveObjectManagerRootWidget::SaveSettingsToIni()
{
    UInteractiveObjectSettings* Settings = UInteractiveObjectSettings::Get();
    if (Settings == nullptr)
    {
        UE_LOG(
            LogInteractiveObjectManager,
            Warning,
            TEXT("InteractiveObjectManagerRootWidget::SaveSettingsToIni: Settings object is null. Nothing will be saved.")
        );
        return;
    }

    Settings->SaveToConfig();
}