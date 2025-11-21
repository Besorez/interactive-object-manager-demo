// Copyright © 2025 Vladyslav Popushoi. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveObjectManagerTypes.generated.h"

/**
 * Spawn type used by the Interactive Object Manager for default object creation.
 *
 * Defines which primitive shape is used when the system spawns a new interactive object
 * without an explicit type override.
 */
UENUM(BlueprintType)
enum class EInteractiveObjectSpawnType : uint8
{
	/** Spawn a default cube shaped interactive object. */
	Cube UMETA(DisplayName = "Cube"),

	/** Spawn a default sphere shaped interactive object. */
	Sphere UMETA(DisplayName = "Sphere"),

	/**
	 * Spawn a randomly selected supported primitive type.
	 * Exact selection rules are defined by the Interactive Object Manager.
	 */
	Random UMETA(DisplayName = "Random")
};