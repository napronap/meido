// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/PlayerMaidCharacter.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ActorComponents/DashComponent.h"
#include "ActorComponents/LockOnComponent.h"
#include "GameModes/DemoGameMode.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"

APlayerMaidCharacter::APlayerMaidCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	// player-only gameplay components (pointers for Dash/MeiDou live in AMaidCharacter for now)
	DashComponent = CreateDefaultSubobject<UDashComponent>(TEXT("DashComponent"));
	MeiDouComponent = CreateDefaultSubobject<UMeiDouComponent>(TEXT("MeiDouComponent"));
	LockOnComponent = CreateDefaultSubobject<ULockOnComponent>(TEXT("LockOnComponent"));

	// Player should remain in scene on death for lose flow/camera
	// (different from enemy maid that uses this to despawn)
	DeathLifeSpanSeconds = 0.f;
}

void APlayerMaidCharacter::BeginPlay()
{
	Super::BeginPlay();

	// the game always starts in the main menu
	if (bApplyMenuCameraPoseOnBeginPlay)
	{
		ApplyMenuCameraPose();
	}
}

void APlayerMaidCharacter::ResetForFlowRestart()
{
	Super::ResetForFlowRestart();

	if (LockOnComponent->IsLockedOn())
	{
		LockOnComponent->ClearLockOn();
	}

	bMenuCameraTransitionActive = false;
	bWinSequenceActive = false;
	bLoseSequenceActive = false;

	if (GetUpMontage)
	{
		PlayAnimMontage(GetUpMontage);
	}
}

void APlayerMaidCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateMenuCameraTransition(DeltaTime);
	UpdateWinSequenceTransition(DeltaTime);
	UpdateLoseSequenceTransition(DeltaTime);
}

void APlayerMaidCharacter::ApplyMenuCameraPose()
{
	if (!CameraBoom)
	{
		return;
	}

	CameraBoom->TargetArmLength = FMath::Max(0.f, MenuArmLength);
	FVector SocketOffset = CameraBoom->SocketOffset;
	SocketOffset.Y = MenuHorizontalOffset;
	CameraBoom->SocketOffset = SocketOffset;

	AController* LocalController = Controller;
	if (!LocalController)
	{
		LocalController = GetController();
	}

	if (!LocalController)
	{
		return;
	}

	const FRotator ActorRot = GetActorRotation();
	const FRotator MenuRot(MenuPitch, ActorRot.Yaw + MenuYawOffset, 0.f);
	LocalController->SetControlRotation(MenuRot);
}

void APlayerMaidCharacter::StartMenuCameraTransitionToGameplay()
{
	AController* LocalController = Controller;
	if (!LocalController)
	{
		LocalController = GetController();
	}

	if (!LocalController || !CameraBoom || bMenuCameraTransitionActive)
	{
		return;
	}

	bMenuCameraTransitionActive = true;
	MenuCameraTransitionElapsed = 0.f;
	MenuCameraTransitionStartArmLength = CameraBoom->TargetArmLength;
	MenuCameraTransitionStartSocketOffsetY = CameraBoom->SocketOffset.Y;
	MenuCameraTransitionTargetSocketOffsetY = GameplayHorizontalOffset;
	MenuCameraTransitionStartRotation = LocalController->GetControlRotation();

	const FRotator ActorRot = GetActorRotation();
	MenuCameraTransitionTargetRotation = FRotator(
		GameplayPitch,
		ActorRot.Yaw + GameplayYawOffset,
		0.f
	);
}

void APlayerMaidCharacter::UpdateMenuCameraTransition(const float DeltaTime)
{
	if (!bMenuCameraTransitionActive)
	{
		return;
	}

	AController* LocalController = Controller;
	if (!LocalController)
	{
		LocalController = GetController();
	}

	if (!LocalController || !CameraBoom)
	{
		bMenuCameraTransitionActive = false;
		return;
	}

	const float Duration = FMath::Max(0.01f, MenuToGameplayTransitionDuration);
	MenuCameraTransitionElapsed += FMath::Max(0.f, DeltaTime);
	const float Alpha = FMath::Clamp(MenuCameraTransitionElapsed / Duration, 0.f, 1.f);
	const float EasedAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, MenuToGameplayTransitionExponent);

	const float NewArmLength = FMath::Lerp(MenuCameraTransitionStartArmLength, GameplayArmLength, EasedAlpha);
	CameraBoom->TargetArmLength = NewArmLength;
	FVector SocketOffset = CameraBoom->SocketOffset;
	SocketOffset.Y = FMath::Lerp(MenuCameraTransitionStartSocketOffsetY, MenuCameraTransitionTargetSocketOffsetY, EasedAlpha);
	CameraBoom->SocketOffset = SocketOffset;

	const FRotator NewRotation = FMath::Lerp(MenuCameraTransitionStartRotation, MenuCameraTransitionTargetRotation, EasedAlpha);
	LocalController->SetControlRotation(NewRotation);

	if (Alpha < 1.f)
	{
		return;
	}

	bMenuCameraTransitionActive = false;
	GetWorld()->GetAuthGameMode<ADemoGameMode>()->FinishTransition();
}

void APlayerMaidCharacter::StartWinSequenceCameraTransition()
{
	AController* LocalController = Controller;
	if (!LocalController)
	{
		LocalController = GetController();
	}

	if (!LocalController || !CameraBoom || bWinSequenceActive)
	{
		return;
	}

	if (LockOnComponent->IsLockedOn())
	{
		LockOnComponent->ClearLockOn();
	}

	if (WinMontage)
	{
		PlayAnimMontage(WinMontage);
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetMorphTarget(TEXT("Fcl_MTH_Fun"), 0.3f);
		MeshComp->SetMorphTarget(TEXT("Fcl_MTH_Joy"), 0.6f);
		MeshComp->SetMorphTarget(TEXT("Fcl_EYE_Joy"), 1.f);
	}
	
	bWinSequenceActive = true;
	WinSequenceElapsed = 0.f;
	WinSequenceStartArmLength = CameraBoom->TargetArmLength;
	WinSequenceStartSocketOffsetY = CameraBoom->SocketOffset.Y;
	WinSequenceStartRotation = LocalController->GetControlRotation();
	WinSequenceTargetArmLength = FMath::Max(0.f, MenuArmLength);
	WinSequenceTargetSocketOffsetY = MenuHorizontalOffset;

	const FRotator ActorRot = GetActorRotation();
	WinSequenceTargetRotation = FRotator(
		MenuPitch,
		ActorRot.Yaw + MenuYawOffset,
		0.f
	);
}

void APlayerMaidCharacter::StartLoseSequenceCameraTransition()
{
	AController* LocalController = Controller;
	if (!LocalController)
	{
		LocalController = GetController();
	}

	if (!LocalController || !CameraBoom || bLoseSequenceActive)
	{
		return;
	}

	if (LockOnComponent->IsLockedOn())
	{
		LockOnComponent->ClearLockOn();
	}

	bLoseSequenceActive = true;
	LoseSequenceElapsed = 0.f;
	LoseSequenceStartArmLength = CameraBoom->TargetArmLength;
	LoseSequenceStartSocketOffsetY = CameraBoom->SocketOffset.Y;
	LoseSequenceStartRotation = LocalController->GetControlRotation();
	LoseSequenceTargetArmLength = FMath::Max(0.f, LoseArmLength);
	LoseSequenceTargetSocketOffsetY = LoseHorizontalOffset;

	const FRotator ActorRot = GetActorRotation();
	LoseSequenceTargetRotation = FRotator(
		LosePitch,
		ActorRot.Yaw + LoseYawOffset,
		0.f
	);
}

void APlayerMaidCharacter::UpdateWinSequenceTransition(const float DeltaTime)
{
	if (!bWinSequenceActive)
	{
		return;
	}

	AController* LocalController = Controller;
	if (!LocalController)
	{
		LocalController = GetController();
	}

	if (!LocalController || !CameraBoom)
	{
		bWinSequenceActive = false;
		GetWorld()->GetAuthGameMode<ADemoGameMode>()->FinishTransition();
		return;
	}

	const float Duration = FMath::Max(0.01f, MenuToGameplayTransitionDuration);
	WinSequenceElapsed += FMath::Max(0.f, DeltaTime);
	const float Alpha = FMath::Clamp(WinSequenceElapsed / Duration, 0.f, 1.f);
	const float EasedAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, MenuToGameplayTransitionExponent);

	CameraBoom->TargetArmLength = FMath::Lerp(WinSequenceStartArmLength, WinSequenceTargetArmLength, EasedAlpha);
	FVector SocketOffset = CameraBoom->SocketOffset;
	SocketOffset.Y = FMath::Lerp(WinSequenceStartSocketOffsetY, WinSequenceTargetSocketOffsetY, EasedAlpha);
	CameraBoom->SocketOffset = SocketOffset;

	const FRotator NewRotation = FMath::Lerp(WinSequenceStartRotation, WinSequenceTargetRotation, EasedAlpha);
	LocalController->SetControlRotation(NewRotation);

	if (Alpha < 1.f)
	{
		return;
	}

	bWinSequenceActive = false;
	GetWorld()->GetAuthGameMode<ADemoGameMode>()->FinishTransition();
}

void APlayerMaidCharacter::UpdateLoseSequenceTransition(const float DeltaTime)
{
	if (!bLoseSequenceActive)
	{
		return;
	}

	AController* LocalController = Controller;
	if (!LocalController)
	{
		LocalController = GetController();
	}

	if (!LocalController || !CameraBoom)
	{
		bLoseSequenceActive = false;
		GetWorld()->GetAuthGameMode<ADemoGameMode>()->FinishTransition();
		return;
	}

	const float Duration = FMath::Max(0.01f, LoseTransitionDuration);
	LoseSequenceElapsed += FMath::Max(0.f, DeltaTime);
	const float Alpha = FMath::Clamp(LoseSequenceElapsed / Duration, 0.f, 1.f);
	const float EasedAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, LoseTransitionExponent);

	CameraBoom->TargetArmLength = FMath::Lerp(LoseSequenceStartArmLength, LoseSequenceTargetArmLength, EasedAlpha);
	FVector SocketOffset = CameraBoom->SocketOffset;
	SocketOffset.Y = FMath::Lerp(LoseSequenceStartSocketOffsetY, LoseSequenceTargetSocketOffsetY, EasedAlpha);
	CameraBoom->SocketOffset = SocketOffset;

	const FRotator NewRotation = FMath::Lerp(LoseSequenceStartRotation, LoseSequenceTargetRotation, EasedAlpha);
	LocalController->SetControlRotation(NewRotation);

	if (Alpha < 1.f)
	{
		return;
	}

	bLoseSequenceActive = false;
	GetWorld()->GetAuthGameMode<ADemoGameMode>()->FinishTransition();
}


void APlayerMaidCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	CachedMoveInput = MovementVector;

	DoMove(MovementVector.X, MovementVector.Y);
}

void APlayerMaidCharacter::MoveReleased()
{
	CachedMoveInput = FVector2D::ZeroVector;
}

void APlayerMaidCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	// While lockon is active we keep yaw camera control driven by target tracking
	// Player can still adjust pitch manually
	const float YawInput = LockOnComponent->IsLockedOn() ? 0.f : LookAxisVector.X;
	DoLook(YawInput, LookAxisVector.Y);
}

void APlayerMaidCharacter::JumpPressed()
{
	Jump();
}

void APlayerMaidCharacter::JumpReleased()
{
	StopJumping();
}

void APlayerMaidCharacter::DashPressed()
{
	DoDash(CachedMoveInput, LockOnComponent->IsLockedOn());
}

void APlayerMaidCharacter::ComboAttackPressed()
{
	DoStartComboAttack();
}

void APlayerMaidCharacter::InputMeiDouM()
{
	RegisterMeiDouInput(EMeiDouInput::EMDI_Moe);
}

void APlayerMaidCharacter::InputMeiDouK()
{
	RegisterMeiDouInput(EMeiDouInput::EMDI_Kyun);
}

void APlayerMaidCharacter::InputMeiDouN()
{
	RegisterMeiDouInput(EMeiDouInput::EMDI_Nyan);
}


void APlayerMaidCharacter::ToggleLockOn()
{
	if (LockOnComponent->IsLockedOn())
	{
		LockOnComponent->ClearLockOn();
		return;
	}

	LockOnComponent->TryLockOn();
}

void APlayerMaidCharacter::OnLockOnSwitch(const FInputActionValue& Value)
{
	// bound on Started so it fires once per key press or stick push until it's released
	LockOnComponent->SwitchTarget(Value.Get<float>() > 0.f ? 1 : -1);
}

void APlayerMaidCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// movement related actions
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &APlayerMaidCharacter::Move);
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Completed, this, &APlayerMaidCharacter::MoveReleased);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerMaidCharacter::Look);

		// combat related actions
		EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started, this,
		                                   &APlayerMaidCharacter::ComboAttackPressed);

		// override inherited jump actions
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerMaidCharacter::JumpPressed);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayerMaidCharacter::JumpReleased);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &APlayerMaidCharacter::DashPressed);

		// mei dou related actions
		EnhancedInputComponent->BindAction(MeiDouMAction, ETriggerEvent::Started, this, &APlayerMaidCharacter::InputMeiDouM);
		EnhancedInputComponent->BindAction(MeiDouKAction, ETriggerEvent::Started, this, &APlayerMaidCharacter::InputMeiDouK);
		EnhancedInputComponent->BindAction(MeiDouNAction, ETriggerEvent::Started, this, &APlayerMaidCharacter::InputMeiDouN);

		// lock on actions
		EnhancedInputComponent->BindAction(LockOnInputAction, ETriggerEvent::Started, this,
		                                   &APlayerMaidCharacter::ToggleLockOn);
		EnhancedInputComponent->BindAction(ChangeLockOnInputAction, ETriggerEvent::Started, this,
		                                   &APlayerMaidCharacter::OnLockOnSwitch);
	}
}
