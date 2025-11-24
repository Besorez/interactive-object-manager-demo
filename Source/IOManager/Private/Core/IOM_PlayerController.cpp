// Copyright © 2025 Vladyslav Popushoi. All rights reserved.

#include "Core/IOM_PlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

AIOM_PlayerController::AIOM_PlayerController()
{
    bIsNavigationModeActive = false;

    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
}

void AIOM_PlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Register the default mapping context with the local player subsystem.
    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (DefaultMappingContext != nullptr)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }

    // Start in UI mode so that the player can interact with widgets immediately.
    EnterUiMode();
}

void AIOM_PlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
    if (EnhancedInputComponent == nullptr)
    {
        return;
    }

    // Navigation mode toggle (for example, right mouse button hold).
    if (NavigationModeAction != nullptr)
    {
        EnhancedInputComponent->BindAction(
            NavigationModeAction,
            ETriggerEvent::Started,
            this,
            &AIOM_PlayerController::OnNavigationModeStarted
        );

        EnhancedInputComponent->BindAction(
            NavigationModeAction,
            ETriggerEvent::Completed,
            this,
            &AIOM_PlayerController::OnNavigationModeCompleted
        );
    }

    // Movement input.
    if (MoveAction != nullptr)
    {
        EnhancedInputComponent->BindAction(
            MoveAction,
            ETriggerEvent::Triggered,
            this,
            &AIOM_PlayerController::HandleMoveInput
        );
    }

    // Vertical movement input (for example Q and E).
    if (VerticalMoveAction != nullptr)
    {
        EnhancedInputComponent->BindAction(
            VerticalMoveAction,
            ETriggerEvent::Triggered,
            this,
            &AIOM_PlayerController::HandleVerticalMoveInput
        );
    }

    // Look input.
    if (LookAction != nullptr)
    {
        EnhancedInputComponent->BindAction(
            LookAction,
            ETriggerEvent::Triggered,
            this,
            &AIOM_PlayerController::HandleLookInput
        );
    }

    // Exit game input (for example Escape key).
    if (ExitGameAction != nullptr)
    {
        EnhancedInputComponent->BindAction(
            ExitGameAction,
            ETriggerEvent::Started,
            this,
            &AIOM_PlayerController::HandleExitRequested
        );
    }
}

void AIOM_PlayerController::OnNavigationModeStarted(const FInputActionValue& ActionValue)
{
    bIsNavigationModeActive = true;
    EnterNavigationMode();
}

void AIOM_PlayerController::OnNavigationModeCompleted(const FInputActionValue& ActionValue)
{
    bIsNavigationModeActive = false;
    EnterUiMode();
}

void AIOM_PlayerController::HandleMoveInput(const FInputActionValue& ActionValue)
{
    if (!bIsNavigationModeActive)
    {
        return;
    }

    const FVector2D MovementVector = ActionValue.Get<FVector2D>();

    APawn* ControlledPawn = GetPawn();
    if (ControlledPawn == nullptr)
    {
        return;
    }

    const FRotator CurrentControlRotation = GetControlRotation();

    const FVector ForwardDirection = UKismetMathLibrary::GetForwardVector(CurrentControlRotation);
    const FVector RightDirection = UKismetMathLibrary::GetRightVector(CurrentControlRotation);

    if (!MovementVector.IsNearlyZero())
    {
        if (!FMath::IsNearlyZero(MovementVector.Y))
        {
            ControlledPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
        }

        if (!FMath::IsNearlyZero(MovementVector.X))
        {
            ControlledPawn->AddMovementInput(RightDirection, MovementVector.X);
        }
    }
}

void AIOM_PlayerController::HandleLookInput(const FInputActionValue& ActionValue)
{
    if (!bIsNavigationModeActive)
    {
        return;
    }

    const FVector2D LookAxisValue = ActionValue.Get<FVector2D>();

    if (!LookAxisValue.IsNearlyZero())
    {
        AddYawInput(LookAxisValue.X);
        AddPitchInput(LookAxisValue.Y);
    }
}

void AIOM_PlayerController::HandleVerticalMoveInput(const FInputActionValue& ActionValue)
{
    if (!bIsNavigationModeActive)
    {
        return;
    }

    const float AxisValue = ActionValue.Get<float>();
    if (FMath::IsNearlyZero(AxisValue))
    {
        return;
    }

    APawn* ControlledPawn = GetPawn();
    if (ControlledPawn == nullptr)
    {
        return;
    }

    const FVector UpDirection = FVector::UpVector;
    ControlledPawn->AddMovementInput(UpDirection, AxisValue);
}

void AIOM_PlayerController::HandleExitRequested(const FInputActionValue& ActionValue)
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    UKismetSystemLibrary::QuitGame(
        World,
        this,
        EQuitPreference::Quit,
        false
    );
}

void AIOM_PlayerController::EnterNavigationMode()
{
    bShowMouseCursor = false;

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
}

void AIOM_PlayerController::EnterUiMode()
{
    bShowMouseCursor = true;

    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);
}