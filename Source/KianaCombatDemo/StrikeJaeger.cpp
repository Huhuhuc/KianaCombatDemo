#include "StrikeJaeger.h"
#include "CombatFeedback.h"
#include "CombatAnimInstance.h"
#include "KianaCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "CombatHUDWidget.h"

AStrikeJaeger::AStrikeJaeger()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->bRunPhysicsWithNoController = true;
		MoveComp->MaxWalkSpeed = MoveSpeed;
		MoveComp->RotationRate = FRotator(0.f, 0.f, TurnSpeed);
	}
}

void AStrikeJaeger::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bRunPhysicsWithNoController = true;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	PlaySequence(AS_Idle, true);
}

void AStrikeJaeger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsDead) { return; }
	if (CooldownRemaining > 0.f)
	{
		CooldownRemaining = FMath::Max(0.f, CooldownRemaining - DeltaTime);
	}
	LocomotionActionRemaining = FMath::Max(0.f, LocomotionActionRemaining - DeltaTime);
	DodgeCooldown = FMath::Max(0.f, DodgeCooldown - DeltaTime);
	if (DisplacementTime > 0.f)
	{
		const float StepTime = FMath::Min(DeltaTime, DisplacementTime);
		const float Step = DisplacementRemaining * StepTime / DisplacementTime;
		FHitResult Hit;
		AddActorWorldOffset(DisplacementDirection * Step, true, &Hit);
		DisplacementRemaining = Hit.bBlockingHit ? 0.f : DisplacementRemaining - Step;
		DisplacementTime -= StepTime;
	}
	switch (EnemyState)
	{
	case EEnemyState::Idle:      TickIdle(); break;
	case EEnemyState::Approach:  TickApproach(DeltaTime); break;
	case EEnemyState::Telegraph: TickTelegraph(DeltaTime); break;
	case EEnemyState::Attack:
		// Test interval overlap, so a slow frame does not skip the hit window.
		if (StateTimer <= DamageWindowEnd && StateTimer + DeltaTime >= DamageWindowBegin) { PerformAttackTrace(); }
		StateTimer += DeltaTime;
		break;
	case EEnemyState::Recover:   TickRecover(DeltaTime); break;
	case EEnemyState::Stunned:   break;
	default: break;
	}
	if (!bActionPlaying && EnemyState != EEnemyState::Dead)
	{
		UpdateLocomotionAnimation();
	}
}

AKianaCharacter* AStrikeJaeger::GetPlayer() const
{
	for (TActorIterator<AKianaCharacter> It(GetWorld()); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

void AStrikeJaeger::EnterState(EEnemyState NewState)
{
	if (EnemyState != NewState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatDiag] EnemyState %d -> %d"), static_cast<int32>(EnemyState), static_cast<int32>(NewState));
	}
	EnemyState = NewState;
	StateTimer = 0.f;
	if (NewState != EEnemyState::Approach)
	{
		GetCharacterMovement()->StopMovementImmediately();
	}
	if (NewState == EEnemyState::Telegraph)
	{
		LocomotionActionRemaining = 0.f;
		PlaySequence(AS_Idle, true);
	}
}

void AStrikeJaeger::TickIdle()
{
	AKianaCharacter* Player = GetPlayer();
	if (!Player || Player->IsDead()) { return; }
	const float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
	if (Dist <= AttackRange && CooldownRemaining <= 0.f)
	{
		EnterState(EEnemyState::Telegraph);
	}
	else if (Dist <= DetectRadius)
	{
		EnterState(EEnemyState::Approach);
	}
}

void AStrikeJaeger::TickApproach(float DeltaTime)
{
	AKianaCharacter* Player = GetPlayer();
	if (!Player || Player->IsDead()) { EnterState(EEnemyState::Idle); return; }
	StateTimer += DeltaTime;
	const FVector ToPlayer = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	const float Dist = FVector::Dist2D(GetActorLocation(), Player->GetActorLocation());
	FaceTarget(DeltaTime);
	if (Dist > DetectRadius * 1.2f) { EnterState(EEnemyState::Idle); return; }
	if (Dist <= AttackRange && CooldownRemaining <= 0.f)
	{
		EnterState(EEnemyState::Telegraph);
		return;
	}
	if (Dist < AttackRange * 0.55f && DodgeCooldown <= 0.f && AttackIndex > 0)
	{
		StartDodge(DodgeIndex++ % 3);
		return;
	}
	FVector Direction = ToPlayer;
	float Speed = MoveSpeed;
	if (Dist <= AttackRange + 80.f && CooldownRemaining > 0.f)
	{
		Speed = MoveSpeed * 0.45f;
		Direction = Dist < AttackRange * 0.8f ? -ToPlayer
			: FVector::CrossProduct(FVector::UpVector, ToPlayer) * (AttackIndex % 2 == 0 ? 1.f : -1.f);
	}
	else if (Dist < AttackRange + 200.f) { Speed = MoveSpeed * 0.55f; }
	GetCharacterMovement()->MaxWalkSpeed = Speed;
	AddMovementInput(Direction, 1.f);
}

void AStrikeJaeger::TickTelegraph(float DeltaTime)
{
	AKianaCharacter* Player = GetPlayer();
	if (!Player || Player->IsDead()) { EnterState(EEnemyState::Idle); return; }
	if (StateTimer < TelegraphDuration * 0.65f) { FaceTarget(DeltaTime); }
	StateTimer += DeltaTime;
	if (StateTimer >= TelegraphDuration)
	{
		StartAttack();
	}
}

void AStrikeJaeger::StartAttack()
{
	ClearActionTimers();
	EnterState(EEnemyState::Attack);
	UAnimSequence* Sequence = PickAttackSequence();
	++AttackIndex;
	if (!Sequence) { FinishAction(); return; }
	bDamageAppliedThisSwing = false;
	CurrentAttackDuration = Sequence->GetPlayLength() / FMath::Max(AttackPlayRate, 0.1f);
 const int32 ClipIndex=(AttackIndex-1)%6;
 const FVector2D Window=AttackHitWindows.IsValidIndex(ClipIndex)?AttackHitWindows[ClipIndex]:FVector2D(.30f,.48f);
 DamageWindowBegin=CurrentAttackDuration*FMath::Clamp(Window.X,0.f,.99f);
 DamageWindowEnd=CurrentAttackDuration*FMath::Clamp(Window.Y,FMath::Clamp(Window.X,0.f,.99f)+.01f,1.f);
	bActionPlaying = true;
	PlaySequence(Sequence, false);
	UE_LOG(LogTemp, Display, TEXT("[EnemyCombat] Attack %d %s duration=%.2f"), AttackIndex, *Sequence->GetName(), CurrentAttackDuration);
	GetWorldTimerManager().SetTimer(ActionEndTimer, this, &AStrikeJaeger::FinishAction, FMath::Max(0.05f, CurrentAttackDuration), false);
}

void AStrikeJaeger::TickRecover(float DeltaTime)
{
	StateTimer += DeltaTime;
	if (StateTimer >= RecoverDuration)
	{
		EnterState(EEnemyState::Idle);
	}
}

UAnimSequence* AStrikeJaeger::PickAttackSequence() const
{
	switch (AttackIndex % 6)
	{
	case 0: return AS_Attack01;
	case 1: return AS_Attack02;
	case 2: return AS_Attack03;
	case 3: return AS_Attack04;
	case 4: return AS_Attack05;
	default: return AS_Attack06;
	}
}

void AStrikeJaeger::PlaySequence(UAnimSequence* Sequence, bool bLooping)
{
	if (!GetMesh() || !Sequence) { return; }
	if (!Cast<UCombatAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		GetMesh()->SetAnimInstanceClass(UCombatAnimInstance::StaticClass());
	}
	if (UAnimSingleNodeInstance* Player = GetMesh()->GetSingleNodeInstance())
	{
		Player->SetAnimationAsset(Sequence, bLooping);
		Player->SetPlaying(true);
	}
	if (UAnimSingleNodeInstance* SingleNode = GetMesh()->GetSingleNodeInstance())
	{
		SingleNode->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		SingleNode->SetPlayRate((Sequence == AS_Attack01 || Sequence == AS_Attack02 || Sequence == AS_Attack03 ||
			Sequence == AS_Attack04 || Sequence == AS_Attack05 || Sequence == AS_Attack06) ? AttackPlayRate : 1.f);
	}
	ActiveSequence = Sequence;
	UE_LOG(LogTemp, Display, TEXT("[EnemyCombat] Animation %s"), *Sequence->GetName());
}

void AStrikeJaeger::UpdateLocomotionAnimation()
{
	if (bActionPlaying || !GetMesh()) { return; }
	const float Speed = GetVelocity().Size2D();
	const bool bMoving = Speed > 5.f;
	if (LocomotionActionRemaining > 0.f)
	{
		if ((ActiveSequence == AS_RunStart && bMoving) || (ActiveSequence == AS_RunEnd && Speed < MoveSpeed * 0.75f)) { return; }
		LocomotionActionRemaining = 0.f;
	}
	const bool bWasRunning = bRunStarted;
	if (!bMoving && bWasRunning && AS_RunEnd && (ActiveSequence == AS_Run || ActiveSequence == AS_RunStart))
	{
		bRunStarted = false;
		PlaySequence(AS_RunEnd, false);
		LocomotionActionRemaining = FMath::Min(0.35f, AS_RunEnd->GetPlayLength());
		LastMoveSpeed = 0.f;
		return;
	}
	UAnimSequence* Desired = bMoving ? (Speed > MoveSpeed * 0.75f ? AS_Run : AS_Walk) : AS_Idle;
	if (bMoving && Speed <= MoveSpeed * 0.75f)
	{
		const FVector VelocityDir = GetVelocity().GetSafeNormal2D();
		const float Side = FVector::DotProduct(GetActorRightVector(), VelocityDir);
		const float Forward = FVector::DotProduct(GetActorForwardVector(), VelocityDir);
		if (Forward < -0.35f && AS_WalkBack) { Desired = AS_WalkBack; }
		else if (Side < -0.35f && AS_WalkLeft) { Desired = AS_WalkLeft; }
		else if (Side > 0.35f && AS_WalkRight) { Desired = AS_WalkRight; }
	}
	if (bMoving && Speed > MoveSpeed * 0.75f && !bRunStarted && AS_RunStart)
	{
		bRunStarted = true;
		PlaySequence(AS_RunStart, false);
		LocomotionActionRemaining = FMath::Min(0.4f, AS_RunStart->GetPlayLength());
		return;
	}
	if (!bMoving) { bRunStarted = false; }
	if (bMoving && Speed <= MoveSpeed * 0.75f) { bRunStarted = false; }
	LastMoveSpeed = Speed;
	if (Desired && ActiveSequence != Desired)
	{
		PlaySequence(Desired, true);
		LocomotionActionRemaining = 0.f;
	}
	if (bMoving && (ActiveSequence == AS_Run || ActiveSequence == AS_Walk ||
		ActiveSequence == AS_WalkBack || ActiveSequence == AS_WalkLeft || ActiveSequence == AS_WalkRight))
	{
		if (UAnimSingleNodeInstance* Node = GetMesh()->GetSingleNodeInstance())
		{
			const float ReferenceSpeed = ActiveSequence == AS_Run ? MoveSpeed : MoveSpeed * 0.5f;
			Node->SetPlayRate(FMath::Clamp(Speed / FMath::Max(ReferenceSpeed, 1.f), 0.65f, 1.35f));
		}
	}
}

void AStrikeJaeger::ClearActionTimers()
{
	GetWorldTimerManager().ClearTimer(ActionEndTimer);
	GetWorldTimerManager().ClearTimer(DamageTimer);
	GetWorldTimerManager().ClearTimer(DoneTimer);
	LocomotionActionRemaining = 0.f;
}

void AStrikeJaeger::FinishAction()
{
	if (bIsDead) { return; }
	const bool bFinishedAttack = EnemyState == EEnemyState::Attack;
	ClearActionTimers();
	bActionPlaying = false;
	bStunEnding = false;
	DisplacementRemaining = DisplacementTime = 0.f;
	CooldownRemaining = FMath::Max(CooldownRemaining, bFinishedAttack ? AttackInterval : 0.6f);
	EnterState(EEnemyState::Recover);
	UpdateLocomotionAnimation();
}

void AStrikeJaeger::PerformAttackTrace()
{
	if (bDamageAppliedThisSwing || EnemyState != EEnemyState::Attack) { return; }
	AKianaCharacter* Player = GetPlayer();
	if (!Player || Player->IsDead()) { return; }
	const FVector Origin = GetActorLocation() + GetActorForwardVector() * AttackTraceForward;
	const float Dist = FVector::Dist(Origin, Player->GetActorLocation());
	if (Dist > AttackTraceRadius) { return; }
	bDamageAppliedThisSwing = true;
	UE_LOG(LogTemp, Warning, TEXT("[CombatDiag] EnemyHit time=%.2f dist=%.2f damage=%.2f"), GetWorld()->GetTimeSeconds(), Dist, AttackDamage);
	Player->TakeDamage(AttackDamage, FDamageEvent(), GetController(), this);
}

void AStrikeJaeger::FaceTarget(float DeltaTime)
{
	AKianaCharacter* Player = GetPlayer();
	if (!Player) { return; }
	const FVector ToTarget = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (ToTarget.IsNearlyZero()) { return; }
	const FRotator Desired = ToTarget.Rotation();
	const FRotator Current = GetActorRotation();
	const float Delta = FMath::FindDeltaAngleDegrees(Current.Yaw, Desired.Yaw);
	const float MaxStep = TurnSpeed * DeltaTime;
	const float Step = FMath::Clamp(Delta, -MaxStep, MaxStep);
	SetActorRotation(FRotator(0.f, Current.Yaw + Step, 0.f));
}

void AStrikeJaeger::ApplyCombatDamage(float Amount, AActor* DamageCauser)
{
	if (bIsDead || Amount <= 0.f) { return; }
	CurrentHealth = FMath::Max(0.f, CurrentHealth - Amount);
 ACombatFeedback::Hit(this,DamageCauser);
	UCombatHUDWidget::OnCombatDamage.Broadcast(GetActorLocation(), Amount);
	if (CurrentHealth <= 0.f) { Die(); return; }
	// Damage still applies during stun, without restarting its recovery forever.
	if (EnemyState == EEnemyState::Stunned) { return; }
	const bool bHeavy = Amount >= 40.f;
	const bool bFromBack = IsValid(DamageCauser) && FVector::DotProduct(GetActorForwardVector(),
		(DamageCauser->GetActorLocation() - GetActorLocation()).GetSafeNormal2D()) < 0.f;
	ClearActionTimers();
	EnterState(EEnemyState::Hurt);
	bActionPlaying = true;
	ApplyHitReaction(Amount, DamageCauser);
	UAnimSequence* Hit = bHeavy ? (bFromBack ? AS_HitBackHeavy : AS_Hit_H)
		: (bFromBack ? AS_HitBackLight : AS_Hit_L);
	if (!Hit) { Hit = bHeavy ? AS_Hit_H : AS_Hit_L; }
	if (Hit) { PlaySequence(Hit, false); }
	const float HitDuration = Hit ? FMath::Min(Hit->GetPlayLength(), bHeavy ? 0.45f : 0.3f) : 0.15f;
	if (bHeavy)
	{
		GetWorldTimerManager().SetTimer(ActionEndTimer, this, &AStrikeJaeger::StartStun, HitDuration, false);
	}
	else
	{
		++LightHitsTaken;
		GetWorldTimerManager().SetTimer(ActionEndTimer, [this]()
		{
			if (bIsDead) { return; }
			if (LightHitsTaken >= 3 && DodgeCooldown <= 0.f)
			{
				LightHitsTaken = 0;
				StartDodge(DodgeIndex++ % 3);
			}
			else { FinishAction(); }
		}, HitDuration, false);
	}
}

void AStrikeJaeger::ApplyHitReaction(float Amount, AActor* DamageCauser)
{
	DisplacementRemaining = DisplacementTime = 0.f;
	if (!IsValid(DamageCauser)) { return; }
	DisplacementDirection = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal2D();
	DisplacementRemaining = Amount >= 40.f ? HeavyHitKnockback : 18.f;
	DisplacementTime = Amount >= 40.f ? 0.24f : 0.12f;
}

void AStrikeJaeger::StartStun()
{
	if (bIsDead) { return; }
	ClearActionTimers();
	EnterState(EEnemyState::Stunned);
	bActionPlaying = true;
	bStunEnding = false;
	DisplacementRemaining = DisplacementTime = 0.f;
	if (!AS_Stun_Start) { BeginStunLoop(); return; }
	PlaySequence(AS_Stun_Start, false);
	GetWorldTimerManager().SetTimer(ActionEndTimer, this, &AStrikeJaeger::BeginStunLoop,
		FMath::Max(0.05f, AS_Stun_Start->GetPlayLength()), false);
}

void AStrikeJaeger::BeginStunLoop()
{
	if (bIsDead || EnemyState != EEnemyState::Stunned) { return; }
	if (AS_Stun_Loop) { PlaySequence(AS_Stun_Loop, true); }
	GetWorldTimerManager().SetTimer(ActionEndTimer, this, &AStrikeJaeger::FinishStun, FMath::Max(0.05f, StunDuration), false);
}

void AStrikeJaeger::FinishStun()
{
	if (bIsDead || EnemyState != EEnemyState::Stunned) { return; }
	ClearActionTimers();
	if (!AS_Stun_End) { FinishAction(); return; }
	bStunEnding = true;
	PlaySequence(AS_Stun_End, false);
	GetWorldTimerManager().SetTimer(ActionEndTimer, this, &AStrikeJaeger::FinishAction,
		FMath::Max(0.05f, AS_Stun_End->GetPlayLength()), false);
}

void AStrikeJaeger::StartDodge(int32 Direction)
{
	if (bIsDead || EnemyState == EEnemyState::Stunned || EnemyState == EEnemyState::Evading) { return; }
	Direction = FMath::Clamp(Direction, 0, 2);
	UAnimSequence* Sequence = Direction == 0 ? AS_Dodge : (Direction == 1 ? AS_DodgeLeft : AS_DodgeRight);
	if (!Sequence) { FinishAction(); return; }
	ClearActionTimers();
	EnterState(EEnemyState::Evading);
	bActionPlaying = true;
	DodgeCooldown = 4.f;
	DisplacementDirection = Direction == 0 ? -GetActorForwardVector()
		: GetActorRightVector() * (Direction == 1 ? -1.f : 1.f);
	DisplacementRemaining = 160.f;
	DisplacementTime = 0.35f;
	PlaySequence(Sequence, false);
	GetWorldTimerManager().SetTimer(ActionEndTimer, this, &AStrikeJaeger::FinishAction,
		FMath::Max(0.35f, Sequence->GetPlayLength()), false);
}

void AStrikeJaeger::Die()
{
	bIsDead = true;
	EnterState(EEnemyState::Dead);
	GetWorldTimerManager().ClearAllTimersForObject(this);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}
	GetWorldTimerManager().ClearTimer(ActionEndTimer);
	GetWorldTimerManager().ClearTimer(DamageTimer);
	GetWorldTimerManager().ClearTimer(DoneTimer);
	PlaySequence(AS_Death, false);
}

float AStrikeJaeger::GetHealthPercent() const
{
	return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f;
}

