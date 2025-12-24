// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MaidCharacter.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AMaidCharacter::AMaidCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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
	
}

// Called when the game starts or when spawned
void AMaidCharacter::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void AMaidCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMaidCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	
	const FRotator ControlRotation = GetControlRotation();

	// yaw (horizontal) rotation, create a rotator that represents the controller's rotation
	// only using yaw because we don't want the character to lay down or else
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

	// given the yaw rotation, give me the X unit in the matrix (forward)
	// X is forward in UE!!
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	// scale that forward by mov vector Y because we have Y axis representing fw/bw directions
	AddMovementInput(ForwardDirection, MovementVector.Y);

	// same thing, given the yaw rotation give me the Y, since Y is right in UE
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	// and scale by our input X, which represents X (horizontal) movement
	AddMovementInput(RightDirection, MovementVector.X);	
}

void AMaidCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

// void AMaidCharacter::Jump()
// {
// }


// Called to bind functionality to input
void AMaidCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &AMaidCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMaidCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
	}

}

