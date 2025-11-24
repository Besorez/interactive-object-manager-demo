// Copyright © 2025 Vladyslav Popushoi. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "IOM_PlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * Player controller for the Interactive Object Manager demo level.
 *
 * Responsibilities:
 * - Switch between navigation mode (fly around the level) and UI interaction mode.
 * - Route Enhanced Input actions to pawn movement and camera look.
 *
 * Navigation mode:
 * - Active while the navigation action (for example, right mouse button) is held.
 * - Mouse cursor is hidden.
 * - Input mode is GameOnly.
 * - WASD (and other configured keys) move the pawn, mouse controls the camera.
 *
 * UI mode:
 * - Active when navigation is not held.
 * - Mouse cursor is visible.
 * - Input mode is GameAndUI.
 * - Movement and look input are ignored.
 */
UCLASS()
class AIOM_PlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AIOM_PlayerController();

protected:
    // APlayerController interface
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    /** Mapping context that defines movement, look and navigation actions. */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    /** 2D movement input (X = strafe, Y = forward/backward). */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;

    /** 2D look input (X = yaw, Y = pitch). */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> LookAction;

    /**
     * Vertical movement axis (for example Q and E keys).
     * Used to move the pawn up and down while in navigation mode.
     */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> VerticalMoveAction;

    /**
     * Digital action used to toggle navigation mode while held.
     * Typically bound to the right mouse button.
     */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> NavigationModeAction;

    /**
     * Digital action used to request exiting the game.
     * Typically bound to the Escape key.
     */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> ExitGameAction;

private:
    /** Indicates whether the controller is currently in navigation mode. */
    UPROPERTY(VisibleInstanceOnly, Category = "State")
    bool bIsNavigationModeActive;

    /** Called when the navigation mode action is pressed (started). */
    void OnNavigationModeStarted(const FInputActionValue& ActionValue);

    /** Called when the navigation mode action is released (completed). */
    void OnNavigationModeCompleted(const FInputActionValue& ActionValue);

    /** Handles movement input while in navigation mode. */
    void HandleMoveInput(const FInputActionValue& ActionValue);

    /** Handles look input while in navigation mode. */
    void HandleLookInput(const FInputActionValue& ActionValue);

    /** Handles vertical movement input while in navigation mode. */
    void HandleVerticalMoveInput(const FInputActionValue& ActionValue);

    /** Handles exit request (for example Esc key). */
    void HandleExitRequested(const FInputActionValue& ActionValue);

    /** Applies GameOnly input mode and hides the mouse cursor. */
    void EnterNavigationMode();

    /** Applies GameAndUI input mode and shows the mouse cursor. */
    void EnterUiMode();
};