#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatTypes.h"
#include "StrikeJaeger.generated.h"

class AKianaCharacter;
class UAnimMontage;
class UAnimSequence;

UCLASS()
class KIANACOMBATDEMO_API AStrikeJaeger : public ACharacter
{
	GENERATED_BODY()

public:
 // Normalized per-clip windows. Defaults preserve existing timing pending visual calibration.
 UPROPERTY(EditAnywhere, Category="Combat|Timing")
 TArray<FVector2D> AttackHitWindows = {FVector2D(.30f,.48f),FVector2D(.30f,.48f),FVector2D(.30f,.48f),FVector2D(.30f,.48f),FVector2D(.30f,.48f),FVector2D(.30f,.48f)};
	AStrikeJaeger();

	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyCombatDamage(float Amount, AActor* DamageCauser);
	UFUNCTION(BlueprintPure, Category = "Combat|State")
	float GetHealthPercent() const;
	UFUNCTION(BlueprintPure, Category = "Combat|State")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category = "Combat|Debug")
	UAnimSequence* GetActiveSequence() const { return ActiveSequence; }
	UFUNCTION(BlueprintCallable, Category = "Combat|Debug")
	void DebugDodge(int32 Direction) { StartDodge(Direction); }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Attack01;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Attack02;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Attack03;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Hit_H;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Hit_L;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Stun_Start;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Stun_Loop;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Stun_End;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Death;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Idle;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Walk;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_RunStart;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Run;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_RunEnd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_WalkBack;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_WalkLeft;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_WalkRight;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Dodge;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_DodgeLeft;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_DodgeRight;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Attack01;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Attack02;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Attack03;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Attack04;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Attack05;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Attack06;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Hit_H;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Hit_L;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_HitBackLight;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_HitBackHeavy;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Stun_Start;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Stun_Loop;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Stun_End;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Death;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float MaxHealth = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float DetectRadius = 1200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float AttackRange = 250.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float AttackInterval = 2.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float TelegraphDuration = 0.8f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float RecoverDuration = 0.9f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float MoveSpeed = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float AttackDamage = 18.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float AttackTraceRadius = 140.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float AttackTraceForward = 120.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float TurnSpeed = 420.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float AttackPlayRate = 1.15f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float StunDuration = 1.4f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Interaction")
	float LightHitStop = 0.06f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Interaction")
	float HeavyHitKnockback = 180.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	EEnemyState EnemyState = EEnemyState::Idle;

private:
	void EnterState(EEnemyState NewState);
	void TickIdle();
	void TickApproach(float DeltaTime);
	void TickTelegraph(float DeltaTime);
	void StartAttack();
	void TickRecover(float DeltaTime);
	void PerformAttackTrace();
	void FaceTarget(float DeltaTime);
	void Die();
	void PlaySequence(UAnimSequence* Sequence, bool bLooping);
	void UpdateLocomotionAnimation();
	void StartStun();
	void FinishStun();
	void ApplyHitReaction(float Amount, AActor* DamageCauser);
	void StartDodge(int32 Direction);
	void ClearActionTimers();
	void BeginStunLoop();
	void FinishAction();
	AKianaCharacter* GetPlayer() const;
	UAnimSequence* PickAttackSequence() const;

	float CurrentHealth = 300.f;
	float StateTimer = 0.f;
	float CooldownRemaining = 0.f;
	bool bIsDead = false;
	bool bDamageAppliedThisSwing = false;
	bool bActionPlaying = false;
	bool bRunStarted = false;
	bool bStunEnding = false;
	float DodgeCooldown = 0.f;
	int32 DodgeIndex = 0;
	float DisplacementRemaining = 0.f;
	float DisplacementTime = 0.f;
	FVector DisplacementDirection = FVector::ZeroVector;
	float DamageWindowBegin = 0.f;
	float DamageWindowEnd = 0.f;
	float CurrentAttackDuration = 0.f;
	int32 LightHitsTaken = 0;
	float LastMoveSpeed = 0.f;
	float LocomotionActionRemaining = 0.f;
	int32 AttackIndex = 0;
	UAnimSequence* ActiveSequence = nullptr;
	FTimerHandle ActionEndTimer;
	FTimerHandle DamageTimer;
	FTimerHandle DoneTimer;
};

