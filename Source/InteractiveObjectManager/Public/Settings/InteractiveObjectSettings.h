// Copyright © 2025 Vladyslav Popushoi. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"
#include "InteractiveObjectManagerTypes.h"
#include "UObject/NoExportTypes.h"
#include "InteractiveObjectSettings.generated.h"

/**
 * Runtime settings for the Interactive Object Manager.
 *
 * This structure represents a validated snapshot of configuration values that are used
 * by the runtime systems. It is populated from ini via UInteractiveObjectSettings and
 * can be safely passed to UI or other systems as a value object.
 */
USTRUCT(BlueprintType)
struct INTERACTIVEOBJECTMANAGER_API FInteractiveObjectRuntimeSettings
{
	GENERATED_BODY()

	/** Default spawn type used when creating new interactive objects. */
	UPROPERTY(EditAnywhere, Category = "InteractiveObjectManager", meta = (ToolTip = "Default primitive type used for newly spawned interactive objects."))
	EInteractiveObjectSpawnType DefaultSpawnType = EInteractiveObjectSpawnType::Cube;

	/** Default color applied to newly spawned interactive objects. */
	UPROPERTY(EditAnywhere, Category = "InteractiveObjectManager", meta = (ToolTip = "Default linear color used when spawning new interactive objects."))
	FLinearColor DefaultColor = FLinearColor::White;

	/** Default non uniform scale applied to newly spawned interactive objects. */
	UPROPERTY(EditAnywhere, Category = "InteractiveObjectManager", meta = (ToolTip = "Default scale applied to interactive objects when no explicit scale is provided."))
	FVector DefaultScale = FVector(1.0f, 1.0f, 1.0f);

	/**
	 * Validates the current settings values.
	 *
	 * Returns true if all values are considered safe to use at runtime,
	 * for example the default scale is strictly positive and non zero.
	 */
	bool IsValid() const;

	/**
	 * Applies safe default values to all fields.
	 *
	 * Intended to be used when config values are missing or invalid so that
	 * the system can continue running without hard failures.
	 */
	void ApplySafeDefaults();
};

/**
 * Central configuration object for the Interactive Object Manager.
 *
 * Responsibilities:
 * - Load settings from ini files via GConfig.
 * - Save validated settings back to ini.
 * - Provide a thread safe runtime copy of settings to the rest of the module.
 *
 * Ini section: [InteractiveObjectManager.Settings]
 */
UCLASS(Config = Game, DefaultConfig)
class INTERACTIVEOBJECTMANAGER_API UInteractiveObjectSettings : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Returns the singleton instance of the settings object.
	 *
	 * The instance is provided via GetMutableDefault and is expected
	 * to live for the entire application lifetime.
	 */
	static UInteractiveObjectSettings* Get();

	/**
	 * Loads settings from the configured ini section using GConfig.
	 *
	 * This method reads raw values from the ini file, performs validation and
	 * updates the internal runtime settings structure. Invalid or missing values
	 * are replaced with safe defaults and a warning is logged.
	 */
	void LoadFromConfig();

	/**
	 * Saves the current runtime settings to the ini file using GConfig.
	 *
	 * The method writes the validated runtime values into the ini section and
	 * flushes the config so that changes persist between application runs.
	 */
	void SaveToConfig() const;

	/**
	 * Validates the current runtime settings and applies safe defaults when required.
	 *
	 * This method does not touch the ini file. It only ensures that the internal
	 * runtime settings structure is in a safe and consistent state.
	 */
	void ApplyDefaultsIfInvalid();

	/**
	 * Returns a copy of the current runtime settings in a thread safe way.
	 *
	 * Callers receive a value copy that can be safely used without any additional
	 * synchronization.
	 */
	FInteractiveObjectRuntimeSettings GetRuntimeSettingsCopy() const;

	/**
	 * Replaces the current runtime settings with the provided values.
	 *
	 * The provided settings are not automatically persisted to ini.
	 * Call SaveToConfig after updating the runtime settings if persistence is required.
	 */
	void UpdateRuntimeSettings(const FInteractiveObjectRuntimeSettings& NewSettings);

	/** Editor facing default spawn type, used for inspection and tweaking in the editor. */
	UPROPERTY(EditAnywhere, Category = "InteractiveObjectManager", meta = (ToolTip = "Default spawn type exposed to the editor. Does not automatically write to ini."))
	EInteractiveObjectSpawnType Editor_DefaultSpawnType = EInteractiveObjectSpawnType::Cube;

	/** Editor facing default color, used for inspection and tweaking in the editor. */
	UPROPERTY(EditAnywhere, Category = "InteractiveObjectManager", meta = (ToolTip = "Default color exposed to the editor. Does not automatically write to ini."))
	FLinearColor Editor_DefaultColor = FLinearColor::White;

	/** Editor facing default scale, used for inspection and tweaking in the editor. */
	UPROPERTY(EditAnywhere, Category = "InteractiveObjectManager", meta = (ToolTip = "Default scale exposed to the editor. Does not automatically write to ini."))
	FVector Editor_DefaultScale = FVector(1.0f, 1.0f, 1.0f);

	/** Convenience accessor for the default spawn type. */
	EInteractiveObjectSpawnType GetDefaultSpawnType() const;

	/** Convenience accessor for the default color. */
	FLinearColor GetDefaultColor() const;

	/** Convenience accessor for the default scale. */
	FVector GetDefaultScale() const;

private:
	/** Runtime validated settings used by the Interactive Object Manager systems. */
	FInteractiveObjectRuntimeSettings RuntimeSettings;

	/** Synchronization primitive used to protect access to RuntimeSettings. */
	mutable FCriticalSection SettingsCriticalSection;

	/** Returns the ini section name used by this settings object. */
	static const TCHAR* GetConfigSectionName();

	/** Returns the ini key name used for the default spawn type. */
	static const TCHAR* GetDefaultSpawnTypeKey();

	/** Returns the ini key name used for the default color. */
	static const TCHAR* GetDefaultColorKey();

	/** Returns the ini key name used for the default scale. */
	static const TCHAR* GetDefaultScaleKey();

	/**
	 * Loads the default spawn type from config into the provided settings structure.
	 *
	 * Invalid or missing values are replaced with a safe default and logged.
	 */
	void LoadSpawnTypeFromConfig(FInteractiveObjectRuntimeSettings& OutSettings);

	/**
	 * Loads the default color from config into the provided settings structure.
	 *
	 * Invalid or missing values are replaced with a safe default and logged.
	 */
	void LoadColorFromConfig(FInteractiveObjectRuntimeSettings& OutSettings);

	/**
	 * Loads the default scale from config into the provided settings structure.
	 *
	 * Invalid or missing values are replaced with a safe default and logged.
	 */
	void LoadScaleFromConfig(FInteractiveObjectRuntimeSettings& OutSettings);

	/** Writes the default spawn type from the runtime settings to the ini file. */
	void SaveSpawnTypeToConfig(const FInteractiveObjectRuntimeSettings& InSettings) const;

	/** Writes the default color from the runtime settings to the ini file. */
	void SaveColorToConfig(const FInteractiveObjectRuntimeSettings& InSettings) const;

	/** Writes the default scale from the runtime settings to the ini file. */
	void SaveScaleToConfig(const FInteractiveObjectRuntimeSettings& InSettings) const;

	/**
	 * Logs a warning about an invalid config value for a given key.
	 *
	 * KeyName is the logical key that failed validation, Reason provides human readable
	 * context that can be copied into the project documentation or wiki.
	 */
	static void LogInvalidValue(const FString& KeyName, const FString& Reason);
};