#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	Idle       UMETA(DisplayName = "Idle"),
	Attacking  UMETA(DisplayName = "Attacking"),
	Evading    UMETA(DisplayName = "Evading"),
	CastingSkill UMETA(DisplayName = "Casting Skill"),
	Hurt       UMETA(DisplayName = "Hurt"),
	Dead       UMETA(DisplayName = "Dead"),
};

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle       UMETA(DisplayName = "Idle"),
	Approach   UMETA(DisplayName = "Approach"),
	Telegraph  UMETA(DisplayName = "Telegraph"),
	Attack     UMETA(DisplayName = "Attack"),
	Recover    UMETA(DisplayName = "Recover"),
	Stunned    UMETA(DisplayName = "Stunned"),
	Dead       UMETA(DisplayName = "Dead"),
	Hurt,
	Evading,
};

