#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatTypes.h"
#include "KianaCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class UAnimSequence;
class AStrikeJaeger;
struct FInputActionValue;

UCLASS()
class KIANACOMBATDEMO_API AKianaCharacter : public ACharacter
{
	GENERATED_BODY()

public:
 AStrikeJaeger* GetLockedEnemy() const { return bTargetLocked ? LockedEnemy.Get() : nullptr; }
 float NextComboOpportunity = 0.f;
	AKianaCharacter();

	// Prototype regression hooks. They invoke the same handlers as the mapped
	// keyboard actions, so a PIE smoke test can exercise combat without a UI
	// automation dependency.
	UFUNCTION(BlueprintCallable, Exec, Category = "Combat|Debug")
	void DebugAttack() { HandleAttack(); }
	UFUNCTION(BlueprintCallable, Category = "Combat|Debug")
	void DebugAttackReleased() { HandleAttackReleased(); }
	UFUNCTION(BlueprintCallable, Exec, Category = "Combat|Debug")
	void DebugBranch() { HandleBranch(); }
	UFUNCTION(BlueprintCallable, Exec, Category = "Combat|Debug")
	void DebugDodge() { HandleDodge(); }
	UFUNCTION(BlueprintCallable, Exec, Category = "Combat|Debug")
	void DebugSkill() { HandleSkill(); }
	UFUNCTION(BlueprintCallable, Category = "Combat|Debug")
	void DebugMove(float Right, float Forward);
	UFUNCTION(BlueprintCallable, Category = "Combat|Debug")
	void DebugRunHeld(bool bHeld) { bDebugRunHeld = bHeld; }
	UFUNCTION(BlueprintPure, Category = "Combat|Debug")
	UAnimSequence* GetActiveSequence() const { return ActiveSequence; }

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;
 virtual void FellOutOfWorld(const UDamageType& DamageType) override;
 UPROPERTY(EditAnywhere, Category="Combat|Death", meta=(ClampMin="100"))
 float VoidFallDistance = 1200.f;
 float SpawnHeight = 0.f;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	// --- Notify callbacks ---
	void OnComboWindowBegin();
	void OnComboWindowEnd();
	void OnAttackWindowBegin();
	void OnAttackWindowEnd();
	void OnDodgeIFrameBegin();
	void OnDodgeIFrameEnd();
	void OnSkillWindowBegin();
	void OnSkillWindowEnd();

	UFUNCTION(BlueprintPure, Category = "Combat|State")
	float GetHealthPercent() const;
 float GetCurrentHealth() const { return CurrentHealth; }
 float GetMaximumHealth() const { return MaxHealth; }
 float GetSkillCooldown() const { return SkillCooldownRemaining; }
 float GetSkillReadyPercent() const { return SkillCooldownDuration > 0.f ? FMath::Clamp(1.f-SkillCooldownRemaining/SkillCooldownDuration,0.f,1.f) : 1.f; }
	UFUNCTION(BlueprintPure, Category = "Combat|State")
	bool IsDead() const { return bIsDead; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Move;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input") TObjectPtr<UInputAction> IA_MoveForward;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input") TObjectPtr<UInputAction> IA_MoveBack;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input") TObjectPtr<UInputAction> IA_MoveLeft;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input") TObjectPtr<UInputAction> IA_MoveRight;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Look;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_LookX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_LookY;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Attack;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Branch;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_LockTarget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Dodge;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Skill;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Attack1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Attack2;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Attack3;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Attack3_Branch;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Attack4;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Attack4_Branch;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Attack5;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_EvadeFront;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_EvadeBack;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Skill;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Hit_H;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> AM_Hit_L;

	// The project currently keeps the AnimBP graph intentionally minimal. These
	// sequences are played through the mesh single-node instance so every combat
	// action remains visible even before the graph is authored by hand.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Idle;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_IdleAlt;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Walk;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Run;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_RunStart;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_RunStopLeft;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_RunStopRight;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Attack1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Attack2;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Attack3;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Attack3_Branch;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Attack4;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Attack4_Branch;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Attack5;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_AttackQTE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_EvadeFront;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_EvadeBack;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Skill;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Hit_H;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Hit_L;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|SingleNode")
	TObjectPtr<UAnimSequence> AS_Die;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats", meta = (ClampMin = "1"))
	float MaxHealth = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float AttackDamage = 25.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float SkillDamage = 60.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float SkillCooldownDuration = 6.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float AttackTraceRadius = 140.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float AttackTraceForward = 110.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float SkillRadius = 350.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float WalkSpeed = 165.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float RunSpeed = 600.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Locomotion")
	float RunStopMinSpeed = 250.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Locomotion")
	float RunStopMinRunTime = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Locomotion")
	float RunStartMaxSpeed = 80.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Locomotion", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float RunStopMaxDuration = 0.35f;
	UPROPERTY(EditAnywhere, Category = "Combat|Locomotion", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float RunStartPlayRate = 1.15f;
	UPROPERTY(EditAnywhere, Category = "Combat|Locomotion", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float RunStopPlayRate = 1.7f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Target", meta = (ClampMin = "30.0", ClampMax = "720.0"))
	float TargetTurnSpeed = 240.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dodge", meta = (ClampMin = "0"))
	float DodgeForwardDistance = 335.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dodge", meta = (ClampMin = "0"))
	float DodgeBackDistance = 250.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dodge", meta = (ClampMin = "0"))
	float DodgeSideDistance = 290.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dodge", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float DodgeMoveDuration = 0.36f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dodge", meta = (ClampMin = "0.5", ClampMax = "2.5"))
	float DodgePlayRate = 1.25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dodge", meta = (ClampMin = "0.01", ClampMax = "0.06"))
	float DodgeStepInterval = 0.02f;

private:
	void HandleMove(const FInputActionValue& Value);
	void HandleForward(const FInputActionValue& Value);
	void HandleBack(const FInputActionValue& Value);
	void HandleLeft(const FInputActionValue& Value);
	void HandleRight(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleLookX(const FInputActionValue& Value);
	void HandleLookY(const FInputActionValue& Value);
	void HandleAttack();
	void HandleGamepadAttack();
	void HandleGamepadAttackReleased();
	void UpdateGamepadInput(float DeltaTime);
	bool bGamepadAttackHeld = false;
	bool bGamepadMoving = false;
	bool bGamepadRunning = false;
	void HandleBranch();
	void HandleLockTarget();
	void HandleDodge();
	void HandleSkill();
	void HandleAttackReleased();

	void StartCombo();
	void AdvanceCombo();
	UAnimSequence* ResolveNextComboSequence();
	void PlayAttackSequence(UAnimSequence* Sequence);
	void PlaySequence(UAnimSequence* Sequence, bool bLooping);
	void PlayLocomotionAnimation();
	void FinishAction();
	void CancelActionTimers();
	void ScheduleWindow(FTimerHandle& BeginHandle, FTimerHandle& EndHandle,
		float BeginTime, float EndTime, TFunction<void()> BeginCallback,
		TFunction<void()> EndCallback);
	void ResetCombo();
	void PerformAttackTrace();
	void PerformSkillBlast();
	void FaceTargetOnce();
	void FaceLockedTarget(float DeltaTime = 0.f);
	void UpdateMovementSpeed();
	void Die();
	AStrikeJaeger* GetEnemy() const;
	AStrikeJaeger* FindNearestEnemy() const;
	void ToggleTargetLock();

	float CurrentHealth = 100.f;
	ECombatState CombatState = ECombatState::Idle;
	int32 ComboIndex = 0;
	bool bBranchRequested = false;
	bool bComboWindowOpen = false;
	bool bBufferedAttack = false;
	int32 BufferedAttackCount = 0;
	bool bAttackHeld = false;
	float AttackHeldSeconds = 0.f;
	float AttackInputCooldown = 0.f;
	float AttackBufferRemaining = 0.f;
	double MouseComboRequestExpiresAt = -1.0;
	float AttackElapsed = 0.f;
	float MovementCancelTime = 0.f;
	UPROPERTY(EditAnywhere, Category = "Combat|Attack", meta = (ClampMin = "0.02", ClampMax = "0.25"))
	float AttackClickDebounce = 0.05f;
	UPROPERTY(EditAnywhere, Category = "Combat|Attack", meta = (ClampMin = "0.1", ClampMax = "0.5"))
	float AttackBufferDuration = 0.50f;
	UPROPERTY(EditAnywhere, Category = "Combat|Attack", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float ComboPlayRate = 1.35f;
	UPROPERTY(EditAnywhere, Category = "Combat|Attack", meta = (ClampMin = "0.2", ClampMax = "0.8"))
	float ComboWindowStart = 0.22f;
	UPROPERTY(EditAnywhere, Category = "Combat|Attack", meta = (ClampMin = "0.3", ClampMax = "0.95"))
	float ComboWindowEnd = 0.92f;
	bool bComboAdvanceConsumed = false;
	uint32 ComboGeneration = 0;
	float DodgeFollowupRemaining = 0.f;
	bool bAttackWindowOpen = false;
	bool bDodgeInvuln = false;
	bool bIsDead = false;
	bool bTargetLocked = false;
	TObjectPtr<AStrikeJaeger> LockedEnemy;
	bool bRunToggled = false;
	bool bDebugRunHeld = false;
	float RunTravelTime = 0.f;
	float LastLocomotionSpeed = 0.f;
	float DodgeElapsed = 0.f;
	float DodgeMoveRemaining = 0.f;
	float DodgeDistanceRemaining = 0.f;
	FVector DodgeWorldDirection = FVector::ZeroVector;
	FVector2D MoveAxis = FVector2D::ZeroVector;
	float MoveInputMagnitude = 0.f;
	float PreviousMoveInputMagnitude = 0.f;
	FVector LastMoveDirection = FVector::ZeroVector;
	bool bMoveForward = false;
	bool bMoveBack = false;
	bool bMoveLeft = false;
	bool bMoveRight = false;
	bool bWasMoving = false;
	float RunStartRemaining = 0.f;
	float RunStopRemaining = 0.f;
	float IdleElapsed = 0.f;
	float SkillCooldownRemaining = 0.f;
	UAnimSequence* ActiveSequence = nullptr;
	bool bActionPlaying = false;
	TSet<TWeakObjectPtr<AActor>> HitActorsThisSwing;
	FTimerHandle ActionEndTimer;
	FTimerHandle AttackWindowBeginTimer;
	FTimerHandle AttackWindowEndTimer;
	FTimerHandle ComboWindowBeginTimer;
	FTimerHandle ComboWindowEndTimer;
	FTimerHandle DodgeIFrameBeginTimer;
	FTimerHandle DodgeIFrameEndTimer;
	FTimerHandle SkillWindowBeginTimer;
	FTimerHandle SkillWindowEndTimer;
	FTimerHandle AttackBufferTimer;
};

