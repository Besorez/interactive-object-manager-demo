// Copyright © 2025 Vladyslav Popushoi. All rights reserved.

#include "Components/InteractiveObjectComponent.h"
#include "InteractiveObjectManagerLog.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"

DEFINE_LOG_CATEGORY(LogInteractiveObjectManager);

UInteractiveObjectComponent::UInteractiveObjectComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    CurrentColor = FLinearColor::White;
    CurrentScale = 1.0f;

    ColorParameterName = TEXT("BaseColor");

    bHasLoggedMissingMesh = false;
    bAreDynamicMaterialsInitialized = false;
}

void UInteractiveObjectComponent::BeginPlay()
{
    Super::BeginPlay();

    // Resolve mesh early so we can warn if the setup is invalid.
    UStaticMeshComponent* MeshComponent = GetEffectiveMeshComponent();
    if (MeshComponent == nullptr)
    {
        LogMissingMeshIfNeeded();
    }

    // Apply initial visual state (values set in editor).
    InitializeDynamicMaterials();
    ApplyColorInternal();
    ApplyScaleInternal();

    RegisterWithManager();
}

void UInteractiveObjectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnregisterFromManager();

    Super::EndPlay(EndPlayReason);
}

void UInteractiveObjectComponent::ApplyColor(const FLinearColor& NewColor)
{
    CurrentColor = NewColor;

    InitializeDynamicMaterials();
    ApplyColorInternal();
}

void UInteractiveObjectComponent::ApplyScale(float NewScale)
{
    const float ClampedScale = FMath::Max(NewScale, 0.01f);
    CurrentScale = ClampedScale;

    ApplyScaleInternal();
}

UStaticMeshComponent* UInteractiveObjectComponent::GetEffectiveMeshComponent()
{
    if (TargetMeshComponent.IsValid())
    {
        return TargetMeshComponent.Get();
    }

    AActor* OwnerActor = GetOwner();
    if (OwnerActor == nullptr)
    {
        return nullptr;
    }

    UStaticMeshComponent* MeshComponent = OwnerActor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComponent == nullptr)
    {
        return nullptr;
    }

    TargetMeshComponent = MeshComponent;
    return MeshComponent;
}

USceneComponent* UInteractiveObjectComponent::GetEffectiveScaleComponent()
{
    if (ScaleTargetComponent.IsValid())
    {
        return ScaleTargetComponent.Get();
    }

    if (UStaticMeshComponent* MeshComponent = GetEffectiveMeshComponent())
    {
        return MeshComponent;
    }

    AActor* OwnerActor = GetOwner();
    if (OwnerActor != nullptr)
    {
        return OwnerActor->GetRootComponent();
    }

    return nullptr;
}

void UInteractiveObjectComponent::InitializeDynamicMaterials()
{
    if (bAreDynamicMaterialsInitialized)
    {
        return;
    }

    UStaticMeshComponent* MeshComponent = GetEffectiveMeshComponent();
    if (MeshComponent == nullptr)
    {
        LogMissingMeshIfNeeded();
        return;
    }

    const int32 MaterialCount = MeshComponent->GetNumMaterials();
    DynamicMaterialInstances.Reset();

    for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
    {
        UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(MaterialIndex);
        if (DynamicMaterial != nullptr)
        {
            DynamicMaterialInstances.Add(DynamicMaterial);
        }
    }

    bAreDynamicMaterialsInitialized = true;

    UE_LOG(
        LogInteractiveObjectManager,
        Log,
        TEXT("InteractiveObjectComponent on Actor '%s' initialized %d dynamic material instances."),
        *GetNameSafe(GetOwner()),
        DynamicMaterialInstances.Num()
    );
}

void UInteractiveObjectComponent::ApplyColorInternal()
{
    if (!bAreDynamicMaterialsInitialized)
    {
        InitializeDynamicMaterials();
    }

    if (DynamicMaterialInstances.Num() == 0)
    {
        return;
    }

    for (UMaterialInstanceDynamic* DynamicMaterial : DynamicMaterialInstances)
    {
        if (DynamicMaterial != nullptr)
        {
            DynamicMaterial->SetVectorParameterValue(ColorParameterName, CurrentColor);
        }
    }
}

void UInteractiveObjectComponent::ApplyScaleInternal()
{
    USceneComponent* ScaleComponent = GetEffectiveScaleComponent();
    const FVector NewScale(CurrentScale);

    if (ScaleComponent != nullptr)
    {
        ScaleComponent->SetWorldScale3D(NewScale);
    }
    else if (AActor* OwnerActor = GetOwner())
    {
        OwnerActor->SetActorScale3D(NewScale);
    }
}

void UInteractiveObjectComponent::RegisterWithManager()
{
    UE_LOG(
        LogInteractiveObjectManager,
        Log,
        TEXT("InteractiveObjectComponent '%s' registering owner '%s' with manager."),
        *GetName(),
        *GetNameSafe(GetOwner())
    );

    // Integration with the actual manager will be wired here (World Subsystem or similar).
}

void UInteractiveObjectComponent::UnregisterFromManager()
{
    UE_LOG(
        LogInteractiveObjectManager,
        Log,
        TEXT("InteractiveObjectComponent '%s' unregistering owner '%s' from manager."),
        *GetName(),
        *GetNameSafe(GetOwner())
    );

    // Integration with the actual manager will be wired here.
}

void UInteractiveObjectComponent::LogMissingMeshIfNeeded()
{
    if (bHasLoggedMissingMesh)
    {
        return;
    }

    bHasLoggedMissingMesh = true;

    UE_LOG(
        LogInteractiveObjectManager,
        Warning,
        TEXT("InteractiveObjectComponent on Actor '%s' could not find a valid StaticMeshComponent. Color changes will be skipped."),
        *GetNameSafe(GetOwner())
    );
}