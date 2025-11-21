// Copyright © 2025 Vladyslav Popushoi. All rights reserved.

#include "Settings/InteractiveObjectSettings.h"

#include "Misc/ConfigCacheIni.h"
#include "Misc/ScopeLock.h"

DEFINE_LOG_CATEGORY(LogInteractiveObjectSettings);

// Helper functions with internal linkage.

/**
 * Attempts to parse a spawn type enum value from a string.
 *
 * Accepts case insensitive textual names such as "Cube", "Sphere", "Random".
 * Returns true on success and writes the result into OutType.
 */
static bool TryParseSpawnType(const FString& InString, EInteractiveObjectSpawnType& OutType)
{
	if (InString.Equals(TEXT("Cube"), ESearchCase::IgnoreCase))
	{
		OutType = EInteractiveObjectSpawnType::Cube;
		return true;
	}

	if (InString.Equals(TEXT("Sphere"), ESearchCase::IgnoreCase))
	{
		OutType = EInteractiveObjectSpawnType::Sphere;
		return true;
	}

	if (InString.Equals(TEXT("Random"), ESearchCase::IgnoreCase))
	{
		OutType = EInteractiveObjectSpawnType::Random;
		return true;
	}

	return false;
}

/**
 * Converts a spawn type enum value into a textual representation suitable for config.
 */
static FString SpawnTypeToString(EInteractiveObjectSpawnType Type)
{
	switch (Type)
	{
	case EInteractiveObjectSpawnType::Cube:
		return TEXT("Cube");
	case EInteractiveObjectSpawnType::Sphere:
		return TEXT("Sphere");
	case EInteractiveObjectSpawnType::Random:
		return TEXT("Random");
	default:
		return TEXT("Cube");
	}
}

// FInteractiveObjectRuntimeSettings

bool FInteractiveObjectRuntimeSettings::IsValid() const
{
	const bool bScaleNonZero = !DefaultScale.IsNearlyZero();
	const bool bScalePositive = (DefaultScale.X > 0.0f && DefaultScale.Y > 0.0f && DefaultScale.Z > 0.0f);
	return bScaleNonZero && bScalePositive;
}

void FInteractiveObjectRuntimeSettings::ApplySafeDefaults()
{
	DefaultSpawnType = EInteractiveObjectSpawnType::Cube;
	DefaultColor = FLinearColor::White;
	DefaultScale = FVector(1.0f, 1.0f, 1.0f);
}

// UInteractiveObjectSettings

UInteractiveObjectSettings* UInteractiveObjectSettings::Get()
{
	return GetMutableDefault<UInteractiveObjectSettings>();
}

void UInteractiveObjectSettings::LoadFromConfig()
{
	FInteractiveObjectRuntimeSettings TempSettings;
	TempSettings.ApplySafeDefaults();

	LoadSpawnTypeFromConfig(TempSettings);
	LoadColorFromConfig(TempSettings);
	LoadScaleFromConfig(TempSettings);

	if (!TempSettings.IsValid())
	{
		LogInvalidValue(TEXT("RuntimeSettings"), TEXT("Invalid values detected while loading from config. Applying safe defaults."));
		TempSettings.ApplySafeDefaults();
	}

	{
		FScopeLock Lock(&SettingsCriticalSection);
		RuntimeSettings = TempSettings;
	}

	// Keep editor facing properties in sync for inspection.
	Editor_DefaultSpawnType = RuntimeSettings.DefaultSpawnType;
	Editor_DefaultColor = RuntimeSettings.DefaultColor;
	Editor_DefaultScale = RuntimeSettings.DefaultScale;
}

void UInteractiveObjectSettings::SaveToConfig() const
{
	if (GConfig == nullptr)
	{
		return;
	}

	FInteractiveObjectRuntimeSettings LocalSettings;
	{
		FScopeLock Lock(&SettingsCriticalSection);
		LocalSettings = RuntimeSettings;
	}

	SaveSpawnTypeToConfig(LocalSettings);
	SaveColorToConfig(LocalSettings);
	SaveScaleToConfig(LocalSettings);

	GConfig->Flush(false, GGameIni);
}

void UInteractiveObjectSettings::ApplyDefaultsIfInvalid()
{
	FScopeLock Lock(&SettingsCriticalSection);

	if (!RuntimeSettings.IsValid())
	{
		LogInvalidValue(TEXT("RuntimeSettings"), TEXT("Invalid runtime settings detected. Applying safe defaults."));
		RuntimeSettings.ApplySafeDefaults();
	}
}

FInteractiveObjectRuntimeSettings UInteractiveObjectSettings::GetRuntimeSettingsCopy() const
{
	FScopeLock Lock(&SettingsCriticalSection);
	return RuntimeSettings;
}

void UInteractiveObjectSettings::UpdateRuntimeSettings(const FInteractiveObjectRuntimeSettings& NewSettings)
{
	FInteractiveObjectRuntimeSettings ValidatedSettings = NewSettings;

	if (!ValidatedSettings.IsValid())
	{
		LogInvalidValue(TEXT("RuntimeSettings"), TEXT("UpdateRuntimeSettings received invalid values. Applying safe defaults."));
		ValidatedSettings.ApplySafeDefaults();
	}

	{
		FScopeLock Lock(&SettingsCriticalSection);
		RuntimeSettings = ValidatedSettings;
	}

	Editor_DefaultSpawnType = RuntimeSettings.DefaultSpawnType;
	Editor_DefaultColor = RuntimeSettings.DefaultColor;
	Editor_DefaultScale = RuntimeSettings.DefaultScale;
}

EInteractiveObjectSpawnType UInteractiveObjectSettings::GetDefaultSpawnType() const
{
	FScopeLock Lock(&SettingsCriticalSection);
	return RuntimeSettings.DefaultSpawnType;
}

FLinearColor UInteractiveObjectSettings::GetDefaultColor() const
{
	FScopeLock Lock(&SettingsCriticalSection);
	return RuntimeSettings.DefaultColor;
}

FVector UInteractiveObjectSettings::GetDefaultScale() const
{
	FScopeLock Lock(&SettingsCriticalSection);
	return RuntimeSettings.DefaultScale;
}

const TCHAR* UInteractiveObjectSettings::GetConfigSectionName()
{
	return TEXT("InteractiveObjectManager.Settings");
}

const TCHAR* UInteractiveObjectSettings::GetDefaultSpawnTypeKey()
{
	return TEXT("DefaultSpawnType");
}

const TCHAR* UInteractiveObjectSettings::GetDefaultColorKey()
{
	return TEXT("DefaultColor");
}

const TCHAR* UInteractiveObjectSettings::GetDefaultScaleKey()
{
	return TEXT("DefaultScale");
}

void UInteractiveObjectSettings::LoadSpawnTypeFromConfig(FInteractiveObjectRuntimeSettings& OutSettings)
{
	if (GConfig == nullptr)
	{
		return;
	}

	FString Value;
	if (!GConfig->GetString(GetConfigSectionName(), GetDefaultSpawnTypeKey(), Value, GGameIni))
	{
		LogInvalidValue(TEXT("DefaultSpawnType"), TEXT("Key not found in config. Using default value."));
		return;
	}

	EInteractiveObjectSpawnType ParsedType;
	if (!TryParseSpawnType(Value, ParsedType))
	{
		LogInvalidValue(TEXT("DefaultSpawnType"), FString::Printf(TEXT("Invalid value '%s' in config. Using default value."), *Value));
		return;
	}

	OutSettings.DefaultSpawnType = ParsedType;
}

void UInteractiveObjectSettings::LoadColorFromConfig(FInteractiveObjectRuntimeSettings& OutSettings)
{
	if (GConfig == nullptr)
	{
		return;
	}

	FString Value;
	if (!GConfig->GetString(GetConfigSectionName(), GetDefaultColorKey(), Value, GGameIni))
	{
		LogInvalidValue(TEXT("DefaultColor"), TEXT("Key not found in config. Using default value."));
		return;
	}

	FLinearColor ParsedColor;
	if (!ParsedColor.InitFromString(Value))
	{
		LogInvalidValue(TEXT("DefaultColor"), FString::Printf(TEXT("Invalid value '%s' in config. Using default value."), *Value));
		return;
	}

	OutSettings.DefaultColor = ParsedColor;
}

void UInteractiveObjectSettings::LoadScaleFromConfig(FInteractiveObjectRuntimeSettings& OutSettings)
{
	if (GConfig == nullptr)
	{
		return;
	}

	FString Value;
	if (!GConfig->GetString(GetConfigSectionName(), GetDefaultScaleKey(), Value, GGameIni))
	{
		LogInvalidValue(TEXT("DefaultScale"), TEXT("Key not found in config. Using default value."));
		return;
	}

	FVector ParsedScale;
	if (!ParsedScale.InitFromString(Value))
	{
		LogInvalidValue(TEXT("DefaultScale"), FString::Printf(TEXT("Invalid value '%s' in config. Using default value."), *Value));
		return;
	}

	OutSettings.DefaultScale = ParsedScale;
}

void UInteractiveObjectSettings::SaveSpawnTypeToConfig(const FInteractiveObjectRuntimeSettings& InSettings) const
{
	if (GConfig == nullptr)
	{
		return;
	}

	const FString Value = SpawnTypeToString(InSettings.DefaultSpawnType);
	GConfig->SetString(GetConfigSectionName(), GetDefaultSpawnTypeKey(), *Value, GGameIni);
}

void UInteractiveObjectSettings::SaveColorToConfig(const FInteractiveObjectRuntimeSettings& InSettings) const
{
	if (GConfig == nullptr)
	{
		return;
	}

	const FString Value = InSettings.DefaultColor.ToString();
	GConfig->SetString(GetConfigSectionName(), GetDefaultColorKey(), *Value, GGameIni);
}

void UInteractiveObjectSettings::SaveScaleToConfig(const FInteractiveObjectRuntimeSettings& InSettings) const
{
	if (GConfig == nullptr)
	{
		return;
	}

	const FString Value = InSettings.DefaultScale.ToString();
	GConfig->SetString(GetConfigSectionName(), GetDefaultScaleKey(), *Value, GGameIni);
}

void UInteractiveObjectSettings::LogInvalidValue(const FString& KeyName, const FString& Reason)
{
	UE_LOG(LogInteractiveObjectSettings, Warning, TEXT("InteractiveObjectSettings key '%s': %s"), *KeyName, *Reason);
}