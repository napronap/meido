// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllers/MaidPlayerController.h"
#include "ActorComponents/LockOnComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Characters/PlayerMaidCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameModes/DemoGameMode.h"
#include "Interfaces/Targetable.h"

void AMaidPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())->AddMappingContext(DefaultIMC, 0);

	UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(InputComponent);

	EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMaidPlayerController::Move);
	EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &AMaidPlayerController::StopMove);
	EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMaidPlayerController::Look);
	EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &AMaidPlayerController::Jump);
	EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMaidPlayerController::StopJumping);
	EnhancedInput->BindAction(DashAction, ETriggerEvent::Started, this, &AMaidPlayerController::Dash);

	EnhancedInput->BindAction(AttackAction, ETriggerEvent::Started, this, &AMaidPlayerController::Attack);
	EnhancedInput->BindAction(MeiDouMoeAction, ETriggerEvent::Started, this, &AMaidPlayerController::MeiDouMoe);
	EnhancedInput->BindAction(MeiDouKyunAction, ETriggerEvent::Started, this, &AMaidPlayerController::MeiDouKyun);
	EnhancedInput->BindAction(MeiDouNyanAction, ETriggerEvent::Started, this, &AMaidPlayerController::MeiDouNyan);

	EnhancedInput->BindAction(LockOnAction, ETriggerEvent::Started, this, &AMaidPlayerController::ToggleLockOn);
	// Started fires once per key press/stick push until it's released
	EnhancedInput->BindAction(SwitchLockOnAction, ETriggerEvent::Started, this, &AMaidPlayerController::SwitchLockOnTarget);

	EnhancedInput->BindAction(PauseAction, ETriggerEvent::Started, this, &AMaidPlayerController::TogglePause);
	EnhancedInput->BindAction(DebugToggleAIAction, ETriggerEvent::Started, this, &AMaidPlayerController::ToggleDebugEnemyAI);
}

void AMaidPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	PlayerMaid = CastChecked<APlayerMaidCharacter>(InPawn);
}

void AMaidPlayerController::UpdateRotation(const float DeltaTime)
{
	if (AActor* Target = PlayerMaid->GetLockOnComponent()->GetCurrentTarget())
	{
		const FVector ToTarget = ITargetable::Execute_GetTargetLocation(Target) - PlayerCameraManager->GetCameraLocation();
		const FRotator CurrentRotation = GetControlRotation();
		const FRotator DesiredRotation(CurrentRotation.Pitch, ToTarget.Rotation().Yaw, CurrentRotation.Roll);

		SetControlRotation(FMath::RInterpTo(CurrentRotation, DesiredRotation, DeltaTime, LockOnYawInterpSpeed));
	}

	Super::UpdateRotation(DeltaTime);
}

void AMaidPlayerController::Move(const FInputActionValue& Value)
{
	MoveInput = Value.Get<FVector2D>();
	PlayerMaid->DoMove(MoveInput.X, MoveInput.Y);
}

void AMaidPlayerController::StopMove()
{
	MoveInput = FVector2D::ZeroVector;
}

void AMaidPlayerController::Look(const FInputActionValue& Value)
{
	const FVector2D LookInput = Value.Get<FVector2D>();

	// locked on the yaw follows the target (UpdateRotation), the player only moves the pitch
	if (!PlayerMaid->GetLockOnComponent()->IsLockedOn())
	{
		AddYawInput(LookInput.X);
	}
	AddPitchInput(LookInput.Y);
}

void AMaidPlayerController::Jump()
{
	PlayerMaid->Jump();
}

void AMaidPlayerController::StopJumping()
{
	PlayerMaid->StopJumping();
}

void AMaidPlayerController::Dash()
{
	PlayerMaid->DoDash(MoveInput, PlayerMaid->GetLockOnComponent()->IsLockedOn());
}

void AMaidPlayerController::Attack()
{
	PlayerMaid->DoStartComboAttack();
}

void AMaidPlayerController::MeiDouMoe()
{
	PlayerMaid->RegisterMeiDouInput(EMeiDouInput::EMDI_Moe);
}

void AMaidPlayerController::MeiDouKyun()
{
	PlayerMaid->RegisterMeiDouInput(EMeiDouInput::EMDI_Kyun);
}

void AMaidPlayerController::MeiDouNyan()
{
	PlayerMaid->RegisterMeiDouInput(EMeiDouInput::EMDI_Nyan);
}

void AMaidPlayerController::ToggleLockOn()
{
	ULockOnComponent* LockOnComponent = PlayerMaid->GetLockOnComponent();
	if (LockOnComponent->IsLockedOn())
	{
		LockOnComponent->ClearLockOn();
	}
	else
	{
		LockOnComponent->TryLockOn();
	}
}

void AMaidPlayerController::SwitchLockOnTarget(const FInputActionValue& Value)
{
	PlayerMaid->GetLockOnComponent()->SwitchTarget(Value.Get<float>() > 0.f ? 1 : -1);
}

void AMaidPlayerController::TogglePause()
{
	GetWorld()->GetAuthGameMode<ADemoGameMode>()->TogglePause();
}

void AMaidPlayerController::ToggleDebugEnemyAI()
{
	GetWorld()->GetAuthGameMode<ADemoGameMode>()->ToggleDebugEnemyAI();
}
