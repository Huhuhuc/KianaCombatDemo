#include "KianaCharacter.h"
#include "CombatFeedback.h"
#include "CombatHUDWidget.h"
#include "CombatAnimInstance.h"
#include "StrikeJaeger.h"
#include "CombatTypes.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "Engine/OverlapResult.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

AKianaCharacter::AKianaCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetCapsuleComponent());
	SpringArm->TargetArmLength = 400.f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SocketOffset = FVector(0.f, 0.f, 80.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->MaxWalkSpeed = RunSpeed;
		MoveComp->MaxAcceleration = 2400.f;
		MoveComp->BrakingDecelerationWalking = 1800.f;
		MoveComp->GroundFriction = 8.f;
		MoveComp->RotationRate = FRotator(0.f, 0.f, 720.f);
	}
}

void AKianaCharacter::BeginPlay()
{
	Super::BeginPlay();
 SpawnHeight = GetActorLocation().Z;
 if (auto* PC = Cast<APlayerController>(GetController()); PC && PC->IsLocalController())
 {
  if (auto* HUD = CreateWidget<UCombatHUDWidget>(PC, UCombatHUDWidget::StaticClass())) { HUD->AddToViewport(); }
 }
	CurrentHealth = MaxHealth;

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		APlayerController* MutablePC = const_cast<APlayerController*>(PC);
		MutablePC->bShowMouseCursor = false;
		MutablePC->SetIgnoreLookInput(false);
		MutablePC->SetIgnoreMoveInput(false);
		MutablePC->SetInputMode(FInputModeGameOnly());
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Sub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (DefaultMappingContext)
				{
					Sub->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}
	}

	// Mesh/AnimClass may be swapped in a Blueprint subclass; reassert combat defaults here.
	bUseControllerRotationYaw = false;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->MaxWalkSpeed = RunSpeed;
	}
	PlayLocomotionAnimation();
}

void AKianaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindKey(EKeys::Gamepad_FaceButton_Left, IE_Pressed, this, &AKianaCharacter::HandleGamepadAttack);
	PlayerInputComponent->BindKey(EKeys::Gamepad_FaceButton_Left, IE_Released, this, &AKianaCharacter::HandleGamepadAttackReleased);
	PlayerInputComponent->BindKey(EKeys::Gamepad_FaceButton_Bottom, IE_Pressed, this, &AKianaCharacter::HandleDodge);
	PlayerInputComponent->BindKey(EKeys::Gamepad_FaceButton_Top, IE_Pressed, this, &AKianaCharacter::HandleSkill);
	PlayerInputComponent->BindKey(EKeys::Gamepad_RightThumbstick, IE_Pressed, this, &AKianaCharacter::HandleLockTarget);
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AKianaCharacter::HandleMove);
		EIC->BindAction(IA_Move, ETriggerEvent::Completed, this, &AKianaCharacter::HandleMove);
		EIC->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &AKianaCharacter::HandleForward);
		EIC->BindAction(IA_MoveForward, ETriggerEvent::Completed, this, &AKianaCharacter::HandleForward);
		EIC->BindAction(IA_MoveBack, ETriggerEvent::Triggered, this, &AKianaCharacter::HandleBack);
		EIC->BindAction(IA_MoveBack, ETriggerEvent::Completed, this, &AKianaCharacter::HandleBack);
		EIC->BindAction(IA_MoveLeft, ETriggerEvent::Triggered, this, &AKianaCharacter::HandleLeft);
		EIC->BindAction(IA_MoveLeft, ETriggerEvent::Completed, this, &AKianaCharacter::HandleLeft);
		EIC->BindAction(IA_MoveRight, ETriggerEvent::Triggered, this, &AKianaCharacter::HandleRight);
		EIC->BindAction(IA_MoveRight, ETriggerEvent::Completed, this, &AKianaCharacter::HandleRight);
		EIC->BindAction(IA_LookX, ETriggerEvent::Triggered, this, &AKianaCharacter::HandleLookX);
		EIC->BindAction(IA_LookY, ETriggerEvent::Triggered, this, &AKianaCharacter::HandleLookY);
		EIC->BindAction(IA_Attack, ETriggerEvent::Started, this, &AKianaCharacter::HandleAttack);
		EIC->BindAction(IA_Attack, ETriggerEvent::Completed, this, &AKianaCharacter::HandleAttackReleased);
		EIC->BindAction(IA_Dodge, ETriggerEvent::Started, this, &AKianaCharacter::HandleDodge);
		EIC->BindAction(IA_Skill, ETriggerEvent::Started, this, &AKianaCharacter::HandleSkill);
		EIC->BindAction(IA_LockTarget, ETriggerEvent::Started, this, &AKianaCharacter::HandleLockTarget);
	}
}

void AKianaCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
 if (!bIsDead && GetActorLocation().Z < SpawnHeight - VoidFallDistance)
 {
  Die();
 }
 if (bIsDead) { return; }
	UpdateGamepadInput(DeltaTime);
	AttackHeldSeconds = bAttackHeld ? AttackHeldSeconds + DeltaTime : 0.f;
	AttackInputCooldown = FMath::Max(0.f, AttackInputCooldown - DeltaTime);
	AttackBufferRemaining = FMath::Max(0.f, AttackBufferRemaining - DeltaTime);
	if (AttackBufferRemaining <= 0.f && CombatState != ECombatState::Attacking)
	{
		BufferedAttackCount = 0;
		bBufferedAttack = false;
	}
	DodgeFollowupRemaining = FMath::Max(0.f, DodgeFollowupRemaining - DeltaTime);
	if (CombatState == ECombatState::Attacking)
	{
		AttackElapsed += DeltaTime;
		// Movement can exit the late recovery, after damage and the initial
		// continuation opportunity. Fresh attack intent takes priority.
		if (AttackElapsed >= MovementCancelTime && MoveInputMagnitude > 0.1f &&
			!bGamepadAttackHeld && BufferedAttackCount == 0 && MouseComboRequestExpiresAt < GetWorld()->GetTimeSeconds())
		{
			FinishAction();
		}
	}
	if (bTargetLocked && (!IsValid(LockedEnemy) || LockedEnemy->IsDead()))
	{
		bTargetLocked = false;
		LockedEnemy = nullptr;
	}
	if (bTargetLocked && CombatState != ECombatState::Dead)
	{
		FaceLockedTarget(DeltaTime);
	}
	const APlayerController* PC = Cast<APlayerController>(GetController());
	const bool bRunHeld = bDebugRunHeld || (PC && (PC->IsInputKeyDown(EKeys::LeftShift) || PC->IsInputKeyDown(EKeys::RightShift)));
	bRunToggled = MoveInputMagnitude > 0.1f && (bRunHeld || bGamepadRunning);
	UpdateMovementSpeed();
	if (CombatState == ECombatState::Evading)
	{
		DodgeElapsed += DeltaTime;
		if (DodgeMoveRemaining > 0.f)
		{
			const float StepTime = FMath::Min(DeltaTime, DodgeMoveRemaining);
			const float StepDistance = DodgeDistanceRemaining * (StepTime / DodgeMoveRemaining);
			FHitResult Sweep;
			AddActorWorldOffset(DodgeWorldDirection * StepDistance, true, &Sweep);
			DodgeDistanceRemaining = Sweep.bBlockingHit ? 0.f : DodgeDistanceRemaining - StepDistance;
			DodgeMoveRemaining -= StepTime;
		}
		if (DodgeElapsed >= 0.70f && MoveInputMagnitude > 0.1f) { FinishAction(); }
	}
	if (!bIsDead && CombatState == ECombatState::Idle && MoveInputMagnitude > 0.1f)
	{
		const FRotator YawRot(0.f, GetControlRotation().Yaw, 0.f);
		LastMoveDirection = (FRotationMatrix(YawRot).GetUnitAxis(EAxis::X) * MoveAxis.Y
			+ FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y) * MoveAxis.X).GetSafeNormal2D();
		AddMovementInput(LastMoveDirection, MoveInputMagnitude);
		// Target lock owns facing. Movement still follows the camera/left stick,
		// but must not fight the smooth lock rotation in the same frame.
		if (!bTargetLocked)
		{
			SetActorRotation(FMath::RInterpConstantTo(GetActorRotation(), LastMoveDirection.Rotation(), DeltaTime, 720.f));
		}
	}
	else if (MoveInputMagnitude <= 0.1f)
	{
		MoveAxis = FVector2D::ZeroVector;
	}
	RunStartRemaining = FMath::Max(0.f, RunStartRemaining - DeltaTime);
	RunStopRemaining = FMath::Max(0.f, RunStopRemaining - DeltaTime);
	IdleElapsed = (!bActionPlaying && MoveInputMagnitude < 0.1f) ? IdleElapsed + DeltaTime : 0.f;
	if (SkillCooldownRemaining > 0.f)
	{
		SkillCooldownRemaining = FMath::Max(0.f, SkillCooldownRemaining - DeltaTime);
	}
	if (CombatState == ECombatState::Attacking && bAttackWindowOpen)
	{
		PerformAttackTrace();
	}
	if (!bActionPlaying && CombatState != ECombatState::Dead)
	{
		PlayLocomotionAnimation();
	}
}

void AKianaCharacter::HandleMove(const FInputActionValue& Value)
{
	if (bIsDead) { return; }
	const FVector2D Axis = Value.Get<FVector2D>().GetClampedToMaxSize(1.f);
	PreviousMoveInputMagnitude = MoveInputMagnitude;
	MoveAxis = Axis;
	MoveInputMagnitude = Axis.Size();
	const FRotator YawRot(0.f, GetControlRotation().Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
	if (MoveInputMagnitude > KINDA_SMALL_NUMBER)
	{
		LastMoveDirection = (Forward * Axis.Y + Right * Axis.X).GetSafeNormal2D();
	}
	if (MoveInputMagnitude <= 0.1f)
	{
		MoveAxis = FVector2D::ZeroVector;
	}
}

void AKianaCharacter::HandleLook(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(-Axis.Y);
}

void AKianaCharacter::DebugMove(float Right, float Forward)
{
	bMoveRight = Right > 0.f;
	bMoveLeft = Right < 0.f;
	bMoveForward = Forward > 0.f;
	bMoveBack = Forward < 0.f;
	HandleMove(FInputActionValue(FVector2D(Right, Forward)));
}

void AKianaCharacter::HandleLookX(const FInputActionValue& Value)
{
	AddControllerYawInput(Value.Get<float>());
}

void AKianaCharacter::HandleLookY(const FInputActionValue& Value)
{
	AddControllerPitchInput(-Value.Get<float>());
}

void AKianaCharacter::HandleGamepadAttack()
{
	bGamepadAttackHeld = true;
	if (CombatState == ECombatState::Idle) { HandleAttack(); }
	else if (CombatState == ECombatState::Attacking && bComboWindowOpen) { OnComboWindowBegin(); }
}

void AKianaCharacter::HandleGamepadAttackReleased()
{
	bGamepadAttackHeld = false;
	HandleAttackReleased();
}

void AKianaCharacter::UpdateGamepadInput(float DeltaTime)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || bIsDead || !InputEnabled())
	{
		bGamepadAttackHeld = false;
		bGamepadRunning = false;
		return;
	}
	const float LX = PC->GetInputAnalogKeyState(EKeys::Gamepad_LeftX);
	const float LY = PC->GetInputAnalogKeyState(EKeys::Gamepad_LeftY);
	const float RX = PC->GetInputAnalogKeyState(EKeys::Gamepad_RightX);
	const float RY = PC->GetInputAnalogKeyState(EKeys::Gamepad_RightY);
	const FVector2D Move(LX, LY);
	if (Move.SizeSquared() > 0.04f)
	{
		HandleMove(FInputActionValue(Move.GetClampedToMaxSize(1.f)));
		bGamepadMoving = true;
	}
	else if (bGamepadMoving)
	{
		HandleMove(FInputActionValue(FVector2D::ZeroVector));
		bGamepadMoving = false;
	}
	if (!PC->IsLookInputIgnored() && (FMath::Abs(RX) > 0.08f || FMath::Abs(RY) > 0.08f))
	{
		FRotator View = PC->GetControlRotation();
		View.Yaw += (FMath::Abs(RX) > 0.08f ? RX : 0.f) * 150.f * DeltaTime;
		View.Pitch = FMath::Clamp(FRotator::NormalizeAxis(View.Pitch) -
			(FMath::Abs(RY) > 0.08f ? RY : 0.f) * 100.f * DeltaTime, -65.f, 65.f);
		PC->SetControlRotation(View);
	}
	// Left shoulder is a hold-to-run modifier on a controller.
	const bool bRun = PC->IsInputKeyDown(EKeys::Gamepad_LeftShoulder);
	bGamepadRunning = bRun && bGamepadMoving;
	// Poll release as well, so unplugging a controller cannot latch auto-attack.
	if (!PC->IsInputKeyDown(EKeys::Gamepad_FaceButton_Left)) { bGamepadAttackHeld = false; }
	if (bGamepadAttackHeld && CombatState == ECombatState::Idle && AttackInputCooldown <= 0.f)
	{
		HandleAttack();
	}
}

void AKianaCharacter::HandleAttack()
{
	if (bIsDead) { return; }
	// Started already represents a distinct press. A frame-time debounce drops
	// legitimate queued clicks when multiple input events arrive in a slow frame.
	AttackInputCooldown = AttackClickDebounce;
	bAttackHeld = true;
	AttackHeldSeconds = 0.f;
	if (CombatState == ECombatState::Idle)
	{
		if (DodgeFollowupRemaining > 0.f && AS_AttackQTE)
		{
			DodgeFollowupRemaining = 0.f;
			ComboIndex = 0;
			PlayAttackSequence(AS_AttackQTE);
			return;
		}
		StartCombo();
	}
	else if (CombatState == ECombatState::Attacking && ComboIndex > 0 && ComboIndex < 5)
	{
		// Each distinct press purchases one remaining strike, capped at this cycle.
		BufferedAttackCount = FMath::Min(BufferedAttackCount + 1, 5 - ComboIndex);
        const float BufferLife=FMath::Max(AttackBufferDuration,NextComboOpportunity-AttackElapsed+0.10f);
		MouseComboRequestExpiresAt = GetWorld()->GetTimeSeconds() + BufferLife;
		bBufferedAttack = BufferedAttackCount > 0;
		AttackBufferRemaining = BufferLife;
		if (bComboWindowOpen) { OnComboWindowBegin(); }
	}
}

void AKianaCharacter::HandleAttackReleased()
{
	bAttackHeld = false;
	AttackHeldSeconds = 0.f;
}

void AKianaCharacter::HandleLockTarget()
{
	if (bIsDead || CombatState == ECombatState::Dead) { return; }
	UE_LOG(LogTemp, Warning, TEXT("[CombatDiag] LockTarget input received"));
	ToggleTargetLock();
}

void AKianaCharacter::HandleBranch()
{
	if (CombatState == ECombatState::Idle)
	{
		bBranchRequested = true;
		StartCombo();
	}
	else if (CombatState == ECombatState::Attacking)
	{
		bBranchRequested = true;
	}
}

void AKianaCharacter::HandleDodge()
{
	if (bIsDead || CombatState == ECombatState::Evading || CombatState == ECombatState::CastingSkill)
	{
		return;
	}
	const bool bHasMoveInput = MoveInputMagnitude > 0.1f && !MoveAxis.IsNearlyZero();
	const FVector InputDir = bHasMoveInput ? LastMoveDirection : GetActorForwardVector();
	UAnimSequence* Sequence = AS_EvadeFront;
	const FVector CameraForward = FRotationMatrix(FRotator(0.f, GetControlRotation().Yaw, 0.f)).GetUnitAxis(EAxis::X);
	const FVector CameraRight = FRotationMatrix(FRotator(0.f, GetControlRotation().Yaw, 0.f)).GetUnitAxis(EAxis::Y);
	const float ForwardAmount = FVector::DotProduct(InputDir, CameraForward);
	const bool bBackDodge = bHasMoveInput && ForwardAmount < -0.45f;
	if (bBackDodge)
	{
		Sequence = AS_EvadeBack;
	}
	if (!Sequence) { return; }
	const FVector DodgeDirection = InputDir.GetSafeNormal2D();
	if (DodgeDirection.IsNearlyZero()) { return; }
	// Dodge has priority over a locomotion action and cancels its stale timers.
	bWasMoving = false;
	const float SideAmount = FMath::Abs(FVector::DotProduct(DodgeDirection, CameraRight));
	const float Distance = ForwardAmount < -0.45f ? DodgeBackDistance
		: (SideAmount > 0.45f ? DodgeSideDistance : DodgeForwardDistance);
	SetActorRotation((bBackDodge ? -DodgeDirection : DodgeDirection).Rotation());
	CombatState = ECombatState::Evading;
	DodgeElapsed = 0.f;
	DodgeMoveRemaining = DodgeMoveDuration;
	DodgeDistanceRemaining = Distance;
	DodgeWorldDirection = DodgeDirection;
 ACombatFeedback::Spawn(GetWorld(),TEXT("Dodge"),GetActorLocation()-FVector(0,0,GetCapsuleComponent()->GetScaledCapsuleHalfHeight()-5.f),(-DodgeDirection).Rotation(),0.4f);
	DodgeFollowupRemaining = 0.f;
	CancelActionTimers();
	ResetCombo();
	GetCharacterMovement()->StopMovementImmediately();
	PlaySequence(Sequence, false);
	if (UAnimSingleNodeInstance* DodgeNode = GetMesh()->GetSingleNodeInstance())
	{
		DodgeNode->SetPlayRate(DodgePlayRate);
	}
	bActionPlaying = true;
	const float Duration = Sequence->GetPlayLength();
	GetWorldTimerManager().SetTimer(ActionEndTimer, this, &AKianaCharacter::FinishAction, Duration, false);
	ScheduleWindow(DodgeIFrameBeginTimer, DodgeIFrameEndTimer, 0.02f, FMath::Min(0.38f, Duration / DodgePlayRate),
		[this]() { OnDodgeIFrameBegin(); }, [this]() { OnDodgeIFrameEnd(); });
}

void AKianaCharacter::HandleSkill()
{
	if (bIsDead || CombatState == ECombatState::CastingSkill || SkillCooldownRemaining > 0.f || !AS_Skill)
	{
		return;
	}
	CombatState = ECombatState::CastingSkill;
	DodgeFollowupRemaining = 0.f;
	CancelActionTimers();
	ResetCombo();
	SkillCooldownRemaining = SkillCooldownDuration;
	PlaySequence(AS_Skill, false);
	bActionPlaying = true;
	const float Duration = AS_Skill ? AS_Skill->GetPlayLength() : 1.f;
	GetWorldTimerManager().SetTimer(ActionEndTimer, this, &AKianaCharacter::FinishAction, Duration, false);
	ScheduleWindow(SkillWindowBeginTimer, SkillWindowEndTimer, FMath::Min(0.30f, Duration),
		FMath::Min(0.80f, Duration), [this]() { OnSkillWindowBegin(); },
		[this]() { OnSkillWindowEnd(); });
}

void AKianaCharacter::StartCombo()
{
	CancelActionTimers();
	ComboIndex = 1;
	bBranchRequested = false;
	bComboAdvanceConsumed = false;
	bBufferedAttack = false;
	BufferedAttackCount = 0;
	AttackBufferRemaining = 0.f;
	PlayAttackSequence(AS_Attack1);
}

void AKianaCharacter::AdvanceCombo()
{
	if (ComboIndex <= 0 || ComboIndex >= 5) { return; }
	// Replace the current attack's timers before scheduling the next segment.
	// Otherwise an old combo window can fire after the new sequence starts.
	CancelActionTimers();
	GetWorldTimerManager().ClearTimer(AttackBufferTimer);
	if (BufferedAttackCount > 0) { --BufferedAttackCount; }
	bBufferedAttack = BufferedAttackCount > 0;
	AttackBufferRemaining = BufferedAttackCount > 0 ? AttackBufferDuration : 0.f;
	bComboAdvanceConsumed = false;
	++ComboIndex;
	if (ComboIndex > 5)
	{
		FinishAction();
		return;
	}
	PlayAttackSequence(ResolveNextComboSequence());
}

void AKianaCharacter::HandleForward(const FInputActionValue& Value)
{
	bMoveForward = Value.Get<bool>();
	HandleMove(FInputActionValue(FVector2D((bMoveRight ? 1.f : 0.f) - (bMoveLeft ? 1.f : 0.f), (bMoveForward ? 1.f : 0.f) - (bMoveBack ? 1.f : 0.f))));
}
void AKianaCharacter::HandleBack(const FInputActionValue& Value)
{
	bMoveBack = Value.Get<bool>();
	HandleMove(FInputActionValue(FVector2D((bMoveRight ? 1.f : 0.f) - (bMoveLeft ? 1.f : 0.f), (bMoveForward ? 1.f : 0.f) - (bMoveBack ? 1.f : 0.f))));
}
void AKianaCharacter::HandleLeft(const FInputActionValue& Value)
{
	bMoveLeft = Value.Get<bool>();
	HandleMove(FInputActionValue(FVector2D((bMoveRight ? 1.f : 0.f) - (bMoveLeft ? 1.f : 0.f), (bMoveForward ? 1.f : 0.f) - (bMoveBack ? 1.f : 0.f))));
}
void AKianaCharacter::HandleRight(const FInputActionValue& Value)
{
	bMoveRight = Value.Get<bool>();
	HandleMove(FInputActionValue(FVector2D((bMoveRight ? 1.f : 0.f) - (bMoveLeft ? 1.f : 0.f), (bMoveForward ? 1.f : 0.f) - (bMoveBack ? 1.f : 0.f))));
}

UAnimSequence* AKianaCharacter::ResolveNextComboSequence()
{
	switch (ComboIndex)
	{
	case 2: return AS_Attack2;
	case 3: return bBranchRequested ? AS_Attack3_Branch : AS_Attack3;
	case 4: return bBranchRequested ? AS_Attack4_Branch : AS_Attack4;
	case 5: return AS_Attack5;
	default: return nullptr;
	}
}

void AKianaCharacter::PlayAttackSequence(UAnimSequence* Sequence)
{
	if (!Sequence)
	{
		FinishAction();
		return;
	}
	CombatState = ECombatState::Attacking;
	HitActorsThisSwing.Empty();
	PlaySequence(Sequence, false);
	bActionPlaying = true;
	// The supplied Attack2..5 exports contain the earlier swings as a prefix.
	// Enter at the new swing rather than replaying the full export from zero.
	static const float EntryTimes[] = {0.f, 0.f, 0.60f, 1.0f, 1.50f, 2.0f};
	const bool bNormalCombo = ComboIndex >= 1 && ComboIndex <= 5 &&
		(Sequence == AS_Attack1 || Sequence == AS_Attack2 || Sequence == AS_Attack3 ||
		 Sequence == AS_Attack4 || Sequence == AS_Attack5);
	const float Entry = bNormalCombo ? EntryTimes[ComboIndex] : 0.f;
	if (UAnimSingleNodeInstance* Node = GetMesh()->GetSingleNodeInstance())
	{
		Node->SetPosition(Entry, false);
	}
	const float Duration = FMath::Max(0.05f, Sequence->GetPlayLength() - Entry);
	const uint32 Generation = ++ComboGeneration;
	const float EffectiveDuration = Duration / FMath::Max(ComboPlayRate, 0.1f);
	AttackElapsed = 0.f;
	MovementCancelTime = EffectiveDuration * 0.72f;
	GetWorldTimerManager().SetTimer(ActionEndTimer, [this, Generation]()
	{
		if (Generation == ComboGeneration) { FinishAction(); }
	}, EffectiveDuration, false);
	const float SwingDuration = bNormalCombo && ComboIndex < 5 ? (EntryTimes[ComboIndex+1]-Entry)/FMath::Max(ComboPlayRate,0.1f) : EffectiveDuration;
const float AttackStart = SwingDuration * 0.18f;
	const float AttackEnd = SwingDuration * 0.75f;
	// The input window starts before the hit recovery finishes and remains open
	// through the late recovery, so rapid clicks are queued without restarting.
	const float ComboStart = bNormalCombo && ComboIndex < 5
		? FMath::Max(0.05f, (EntryTimes[ComboIndex+1] - Entry) / FMath::Max(ComboPlayRate,0.1f))
		: EffectiveDuration * FMath::Max(0.48f, ComboWindowStart);
	const float ComboEnd = EffectiveDuration * ComboWindowEnd;
 NextComboOpportunity=ComboStart;
	ScheduleWindow(AttackWindowBeginTimer, AttackWindowEndTimer, AttackStart, AttackEnd,
		[this]() { OnAttackWindowBegin(); }, [this]() { OnAttackWindowEnd(); });
	if (ComboIndex > 0 && ComboIndex < 5)
	{
		ScheduleWindow(ComboWindowBeginTimer, ComboWindowEndTimer, ComboStart, ComboEnd,
			[this, Generation]() { if (Generation == ComboGeneration) { OnComboWindowBegin(); } },
			[this, Generation]() { if (Generation == ComboGeneration) { OnComboWindowEnd(); } });
	}
	bComboAdvanceConsumed = false;
}

void AKianaCharacter::PlaySequence(UAnimSequence* Sequence, bool bLooping)
{
	if (!GetMesh() || !Sequence) { return; }
	if (Sequence != AS_RunStart) { RunStartRemaining = 0.f; }
	if (Sequence != AS_RunStopLeft && Sequence != AS_RunStopRight) { RunStopRemaining = 0.f; }
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
		// Extract the animated root from the visible pose without applying it twice.
		// CharacterMovement supplies locomotion and the explicit dodge impulse.
		SingleNode->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		const bool bComboAttack = Sequence == AS_Attack1 || Sequence == AS_Attack2 ||
			Sequence == AS_Attack3 || Sequence == AS_Attack3_Branch || Sequence == AS_Attack4 ||
			Sequence == AS_Attack4_Branch || Sequence == AS_Attack5;
		const float Rate = bComboAttack ? ComboPlayRate : (Sequence == AS_RunStart ? RunStartPlayRate
			: (Sequence == AS_RunStopLeft || Sequence == AS_RunStopRight ? RunStopPlayRate : 1.f));
		SingleNode->SetPlayRate(Rate);
	}
	ActiveSequence = Sequence;
}

void AKianaCharacter::PlayLocomotionAnimation()
{
	if (bActionPlaying || !GetMesh()) { return; }
	const float Speed = GetVelocity().Size2D();
	const bool bHasMovementInput = MoveInputMagnitude > 0.1f;
	if (bHasMovementInput)
	{
		RunStopRemaining = 0.f;
		RunTravelTime = bRunToggled && Speed >= RunStopMinSpeed
			? RunTravelTime + GetWorld()->GetDeltaSeconds() : 0.f;
		LastLocomotionSpeed = Speed;
		if (!bWasMoving && bRunToggled && AS_RunStart && Speed < RunStartMaxSpeed)
		{
			bWasMoving = true;
			PlaySequence(AS_RunStart, false);
			RunStartRemaining = AS_RunStart->GetPlayLength();
			return;
		}
		bWasMoving = true;
		if (bRunToggled && RunStartRemaining > 0.f && ActiveSequence == AS_RunStart) { return; }
	}
	else
	{
		RunStartRemaining = 0.f;
		if (bWasMoving)
		{
			bWasMoving = false;
			const bool bQualifiesForStop = RunTravelTime >= RunStopMinRunTime && LastLocomotionSpeed >= RunStopMinSpeed;
			RunTravelTime = 0.f;
			// Choose the planted foot from the run cycle, not the world movement direction.
			float Phase = 0.f;
			if (UAnimSingleNodeInstance* Node = GetMesh()->GetSingleNodeInstance())
			{
				Phase = ActiveSequence ? Node->GetCurrentTime() / FMath::Max(ActiveSequence->GetPlayLength(), 0.01f) : 0.f;
			}
			UAnimSequence* Stop = Phase < 0.5f ? AS_RunStopLeft : AS_RunStopRight;
			if (bQualifiesForStop && Stop && (ActiveSequence == AS_Run || ActiveSequence == AS_RunStart))
			{
				PlaySequence(Stop, false);
				RunStopRemaining = FMath::Min(Stop->GetPlayLength(), RunStopMaxDuration);
				return;
			}
		}
		if (RunStopRemaining > 0.f)
		{
			if (UAnimSingleNodeInstance* Node = GetMesh()->GetSingleNodeInstance())
			{
				if (Node->GetCurrentTime() >= RunStopMaxDuration) { RunStopRemaining = 0.f; }
			}
			if (RunStopRemaining > 0.f) { return; }
		}
	}
	UAnimSequence* Desired = bHasMovementInput
		? (bRunToggled ? AS_Run : AS_Walk)
		: (AS_IdleAlt && IdleElapsed > 6.f ? AS_IdleAlt : AS_Idle);
	if (Desired && ActiveSequence != Desired)
	{
		PlaySequence(Desired, true);
	}
	if (Desired)
	{
		if (UAnimSingleNodeInstance* SingleNode = GetMesh()->GetSingleNodeInstance())
		{
			const float NominalSpeed = Desired == AS_Walk ? WalkSpeed : RunSpeed;
			const float SpeedScale = bHasMovementInput ? FMath::Clamp(Speed / FMath::Max(NominalSpeed, 1.f), 0.65f, 1.15f) : 1.f;
			SingleNode->SetPlayRate(SpeedScale);
		}
	}
}

void AKianaCharacter::CancelActionTimers()
{
	FTimerManager& Timers = GetWorldTimerManager();
	Timers.ClearTimer(ActionEndTimer);
	Timers.ClearTimer(AttackWindowBeginTimer);
	Timers.ClearTimer(AttackWindowEndTimer);
	Timers.ClearTimer(ComboWindowBeginTimer);
	Timers.ClearTimer(ComboWindowEndTimer);
	Timers.ClearTimer(DodgeIFrameBeginTimer);
	Timers.ClearTimer(DodgeIFrameEndTimer);
	Timers.ClearTimer(SkillWindowBeginTimer);
	Timers.ClearTimer(SkillWindowEndTimer);
	Timers.ClearTimer(AttackBufferTimer);
	AttackBufferRemaining = 0.f;
	bAttackWindowOpen = false;
	bComboWindowOpen = false;
	bDodgeInvuln = false;
}

void AKianaCharacter::FinishAction()
{
	if (CombatState == ECombatState::Dead) { return; }
	if (CombatState == ECombatState::Evading)
	{
		DodgeFollowupRemaining = 1.2f;
		DodgeMoveRemaining = 0.f;
		DodgeDistanceRemaining = 0.f;
		DodgeWorldDirection = FVector::ZeroVector;
		GetCharacterMovement()->StopMovementImmediately();
	}
	CancelActionTimers();
	bActionPlaying = false;
	if (CombatState != ECombatState::Dead)
	{
		CombatState = ECombatState::Idle;
	}
	ResetCombo();
	AttackInputCooldown = 0.12f;
	RunTravelTime = 0.f;
	bWasMoving = MoveInputMagnitude > 0.1f;
	PlayLocomotionAnimation();
}

void AKianaCharacter::ScheduleWindow(FTimerHandle& BeginHandle, FTimerHandle& EndHandle,
	float BeginTime, float EndTime, TFunction<void()> BeginCallback, TFunction<void()> EndCallback)
{
	FTimerManager& Timers = GetWorldTimerManager();
	Timers.SetTimer(BeginHandle, [BeginCallback]() { BeginCallback(); }, FMath::Max(0.f, BeginTime), false);
	Timers.SetTimer(EndHandle, [EndCallback]() { EndCallback(); }, FMath::Max(BeginTime, EndTime), false);
}

void AKianaCharacter::ResetCombo()
{
	MouseComboRequestExpiresAt = -1.0;
	ComboIndex = 0;
	bBranchRequested = false;
	bBufferedAttack = false;
	BufferedAttackCount = 0;
	bComboWindowOpen = false;
	bAttackWindowOpen = false;
	bComboAdvanceConsumed = false;
	HitActorsThisSwing.Empty();
}

void AKianaCharacter::OnComboWindowBegin()
{
	bComboWindowOpen = true;
	if (BufferedAttackCount > 0 || bGamepadAttackHeld)
	{
		if (bComboAdvanceConsumed) { return; }
		bComboAdvanceConsumed = true;
		AdvanceCombo();
	}
}

void AKianaCharacter::OnComboWindowEnd()
{
	bComboWindowOpen = false;
}

void AKianaCharacter::OnAttackWindowBegin()
{
	bAttackWindowOpen = true;
	HitActorsThisSwing.Empty();
}

void AKianaCharacter::OnAttackWindowEnd()
{
	bAttackWindowOpen = false;
}

void AKianaCharacter::OnDodgeIFrameBegin()
{
	bDodgeInvuln = true;
}

void AKianaCharacter::OnDodgeIFrameEnd()
{
	bDodgeInvuln = false;
}

void AKianaCharacter::OnSkillWindowBegin()
{
	PerformSkillBlast();
}

void AKianaCharacter::OnSkillWindowEnd()
{
}

void AKianaCharacter::PerformAttackTrace()
{
	const FVector Origin = GetActorLocation() + GetActorForwardVector() * AttackTraceForward;
	TArray<FOverlapResult> Hits;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AttackTrace), false, this);
	GetWorld()->OverlapMultiByChannel(Hits, Origin, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(AttackTraceRadius), Params);

	for (const FOverlapResult& Hit : Hits)
	{
		AActor* Actor = Hit.GetActor();
		if (!Actor || Actor == this) { continue; }
		if (HitActorsThisSwing.Contains(Actor)) { continue; }
		if (AStrikeJaeger* Enemy = Cast<AStrikeJaeger>(Actor))
		{
			HitActorsThisSwing.Add(Actor);
			const float Damage = AttackDamage * (1.f + 0.15f * (ComboIndex - 1));
			Enemy->ApplyCombatDamage(Damage, this);
		}
	}
}

void AKianaCharacter::PerformSkillBlast()
{
 ACombatFeedback::Spawn(GetWorld(),TEXT("Skill"),GetActorLocation()-FVector(0,0,50),FRotator::ZeroRotator,1.0f);
	TArray<FOverlapResult> Hits;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SkillBlast), false, this);
	GetWorld()->OverlapMultiByChannel(Hits, GetActorLocation(), FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(SkillRadius), Params);
	for (const FOverlapResult& Hit : Hits)
	{
		AActor* Actor = Hit.GetActor();
		if (!Actor || Actor == this) { continue; }
		if (HitActorsThisSwing.Contains(Actor)) { continue; }
		if (AStrikeJaeger* Enemy = Cast<AStrikeJaeger>(Actor))
		{
			HitActorsThisSwing.Add(Actor);
			Enemy->ApplyCombatDamage(SkillDamage, this);
		}
	}
}

void AKianaCharacter::FaceTargetOnce()
{
	AStrikeJaeger* Enemy = GetEnemy();
	if (!Enemy) { return; }
	const FVector ToTarget = (Enemy->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (!ToTarget.IsNearlyZero())
	{
		const FRotator Desired = ToTarget.Rotation();
		const float Delta = FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw, Desired.Yaw);
		const float Step = FMath::Clamp(Delta, -180.f, 180.f);
		SetActorRotation(FRotator(0.f, GetActorRotation().Yaw + Step, 0.f));
		if (AController* C = GetController())
		{
			C->SetControlRotation(FRotator(0.f, GetActorRotation().Yaw, 0.f));
		}
	}
}

void AKianaCharacter::FaceLockedTarget(float DeltaTime)
{
	if (!IsValid(LockedEnemy) || LockedEnemy->IsDead()) { return; }
	const FVector ToTarget = (LockedEnemy->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (ToTarget.IsNearlyZero()) { return; }
	const FRotator Desired(0.f, ToTarget.Rotation().Yaw, 0.f);
	const float TurnStep = DeltaTime > 0.f ? TargetTurnSpeed : 0.f;
	const FRotator NewRotation = DeltaTime > 0.f
		? FMath::RInterpConstantTo(GetActorRotation(), Desired, DeltaTime, TurnStep)
		: GetActorRotation();
	if (CombatState != ECombatState::Evading && CombatState != ECombatState::Hurt)
	{
		SetActorRotation(NewRotation);
	}
	if (AController* C = GetController())
	{
		const FRotator CameraTarget(C->GetControlRotation().Pitch, Desired.Yaw, 0.f);
		const FRotator ControlRotation = DeltaTime > 0.f
			? FMath::RInterpConstantTo(C->GetControlRotation(), CameraTarget, DeltaTime, TurnStep)
			: CameraTarget;
		C->SetControlRotation(ControlRotation);
	}
}

void AKianaCharacter::UpdateMovementSpeed()
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = bRunToggled ? RunSpeed : WalkSpeed;
	}
}

AStrikeJaeger* AKianaCharacter::GetEnemy() const
{
	AStrikeJaeger* Found = nullptr;
	for (TActorIterator<AStrikeJaeger> It(GetWorld()); It; ++It)
	{
		Found = *It;
		break;
	}
	return Found;
}

AStrikeJaeger* AKianaCharacter::FindNearestEnemy() const
{
	AStrikeJaeger* Nearest = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	for (TActorIterator<AStrikeJaeger> It(GetWorld()); It; ++It)
	{
		AStrikeJaeger* Candidate = *It;
		if (!IsValid(Candidate) || Candidate->IsDead()) { continue; }
		const float DistSq = FVector::DistSquared2D(GetActorLocation(), Candidate->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Nearest = Candidate;
		}
	}
	return Nearest;
}

void AKianaCharacter::ToggleTargetLock()
{
	if (bTargetLocked)
	{
		bTargetLocked = false;
		LockedEnemy = nullptr;
		return;
	}
	LockedEnemy = FindNearestEnemy();
	bTargetLocked = IsValid(LockedEnemy);
	UE_LOG(LogTemp, Warning, TEXT("[CombatDiag] Target lock=%s enemy=%s"),
		bTargetLocked ? TEXT("ON") : TEXT("OFF"),
		LockedEnemy ? *LockedEnemy->GetName() : TEXT("None"));
}

float AKianaCharacter::GetHealthPercent() const
{
	return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f;
}

float AKianaCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead || bDodgeInvuln) { return 0.f; }
	DodgeFollowupRemaining = 0.f;
	const float Applied = FMath::Max(0.f, DamageAmount);
	CurrentHealth = FMath::Max(0.f, CurrentHealth - Applied);
 if(Applied>0.f) ACombatFeedback::Hit(this,DamageCauser);
	UCombatHUDWidget::OnCombatDamage.Broadcast(GetActorLocation(), Applied);
	if (CurrentHealth <= 0.f) { Die(); return Applied; }
	UE_LOG(LogTemp, Warning, TEXT("[CombatDiag] PlayerDamage time=%.2f applied=%.2f current=%.2f"), GetWorld()->GetTimeSeconds(), Applied, CurrentHealth);
	const bool bHeavy = Applied >= 20.f;
	if (CombatState != ECombatState::Dead)
	{
		CombatState = ECombatState::Hurt;
		CancelActionTimers();
		ResetCombo();
		UAnimSequence* HitSequence = bHeavy ? AS_Hit_H : AS_Hit_L;
		if (!HitSequence) { FinishAction(); return Applied; }
		PlaySequence(HitSequence, false);
		bActionPlaying = true;
		const float Duration = HitSequence ? HitSequence->GetPlayLength() : 0.8f;
		GetWorldTimerManager().SetTimer(ActionEndTimer, this, &AKianaCharacter::FinishAction, Duration, false);
	}
	if (CurrentHealth <= 0.f)
	{
		Die();
	}
	return Applied;
}

void AKianaCharacter::Die()
{
 if (bIsDead) { return; }
 CurrentHealth = 0.f;
	bIsDead = true;
	CombatState = ECombatState::Dead;
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DisableInput(Cast<APlayerController>(GetController()));
	CancelActionTimers();
	bActionPlaying = true;
	PlaySequence(AS_Die ? AS_Die : AS_Hit_H, false);
}

void AKianaCharacter::FellOutOfWorld(const UDamageType& DamageType)
{
 // Preserve the pawn for the HUD's defeat/retry flow instead of destroying it.
 Die();
}

