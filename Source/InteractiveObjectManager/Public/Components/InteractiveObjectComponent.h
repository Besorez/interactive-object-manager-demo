// Copyright © 2025 Vladyslav Popushoi. All rights reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "InteractiveObjectComponent.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UMaterialInstanceDynamic;
class UInteractiveObjectManagerSubsystem;

/**
 * Attach this component to any Actor to make it manageable by the Interactive Object Manager.
 *
 * Responsibilities:
 * - Stores current color and uniform scale for the interactive object.
 * - Locates the target StaticMeshComponent (auto or explicit).
 * - Creates dynamic material instances on demand and applies color changes.
 * - Applies uniform scale to the mesh or the owning Actor.
 * - Registers and unregisters with the Interactive Object Manager subsystem.
 */
UCLASS(ClassGroup = (Interactive), meta = (BlueprintSpawnableComponent))
class INTERACTIVEOBJECTMANAGER_API UInteractiveObjectComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInteractiveObjectComponent();

    /** Set a new color for this interactive object and apply it to dynamic materials. */
    UFUNCTION(BlueprintCallable, Category = "Interactive Object")
    void ApplyColor(const FLinearColor& NewColor);

    /** Set a new uniform scale for this interactive object and apply it. */
    UFUNCTION(BlueprintCallable, Category = "Interactive Object")
    void ApplyScale(float NewScale);

    /** Returns the current color stored by this component. */
    UFUNCTION(BlueprintCallable, Category = "Interactive Object")
    FLinearColor GetCurrentColor() const;

    /** Returns the current uniform scale stored by this component. */
    UFUNCTION(BlueprintCallable, Category = "Interactive Object")
    float GetCurrentScale() const;

    /** Returns a display name that should be shown in UI lists. */
    UFUNCTION(BlueprintCallable, Category = "Interactive Object")
    FString GetDisplayNameForUI() const;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    /** Current color for this interactive object. Applied to dynamic material instances. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactive Object", meta = (AllowPrivateAccess = "true"))
    FLinearColor CurrentColor;

    /** Current uniform scale (X = Y = Z) for this interactive object. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactive Object", meta = (ClampMin = "0.01", AllowPrivateAccess = "true"))
    float CurrentScale;

    /**
     * Optional explicit target StaticMeshComponent.
     * If not set, the component will try to find a UStaticMeshComponent on the owner Actor.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactive Object", meta = (AllowPrivateAccess = "true"))
    TWeakObjectPtr<UStaticMeshComponent> TargetMeshComponent;

    /**
     * Optional explicit component to receive scale changes.
     * If not set, the component falls back to the mesh or the Actor root.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactive Object", meta = (AllowPrivateAccess = "true"))
    TWeakObjectPtr<USceneComponent> ScaleTargetComponent;

    /** Optional label that overrides actor name in UI lists. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactive Object", meta = (AllowPrivateAccess = "true"))
    FString DisplayLabel;

    /** Dynamic material instances created on the target mesh when color is changed. */
    UPROPERTY(Transient)
    TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterialInstances;

    /** Material parameter name used to drive the color on dynamic material instances. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactive Object", meta = (AllowPrivateAccess = "true"))
    FName ColorParameterName;

    /** Avoids spamming logs when a mesh cannot be found. */
    bool bHasLoggedMissingMesh;

    /** Tracks whether dynamic material instances were already initialized. */
    bool bAreDynamicMaterialsInitialized;

    /** Cached pointer to the world manager subsystem. */
    TWeakObjectPtr<UInteractiveObjectManagerSubsystem> CachedManagerSubsystem;

    /** Resolve or cache the target StaticMeshComponent for this interactive object. */
    UStaticMeshComponent* GetEffectiveMeshComponent();

    /**
     * Resolve the component that will receive uniform scale.
     * Priority:
     * - ScaleTargetComponent (if set)
     * - Effective mesh component
     * - Owner root component
     */
    USceneComponent* GetEffectiveScaleComponent();

    /** Create dynamic material instances on the target mesh if not already created. */
    void InitializeDynamicMaterials();

    /** Apply the currently stored color to all dynamic material instances. */
    void ApplyColorInternal();

    /** Apply the currently stored uniform scale to the chosen scale target. */
    void ApplyScaleInternal();

    /** Register this interactive object in the manager subsystem. */
    void RegisterWithManager();

    /** Unregister this interactive object from the manager subsystem. */
    void UnregisterFromManager();

    /** Log a warning about missing mesh once. */
    void LogMissingMeshIfNeeded();
};