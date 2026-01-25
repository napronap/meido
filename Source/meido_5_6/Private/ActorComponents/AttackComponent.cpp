// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/AttackComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UAttackComponent::UAttackComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void UAttackComponent::StartAttack()
{
	if (bIsAttacking) return;

	bIsAttacking = true;
	HitActorsThisAttack.Reset();
}

void UAttackComponent::OpenHitWindow(FName InSocket)
{
	CurrentHitSocket = InSocket;
	bHitWindowOpen = true;
}

void UAttackComponent::CloseHitWindow()
{
	bHitWindowOpen = false;
	bIsAttacking = false;
}

void UAttackComponent::PerformHitTrace()
{
	if (!OwnerCharacter) return;

	const FVector Start = OwnerCharacter->GetMesh()->GetSocketLocation(CurrentHitSocket);

	const FVector End = Start + OwnerCharacter->GetActorForwardVector() * TraceDistance;

	FCollisionShape Box = FCollisionShape::MakeBox(BoxExtent);

	FHitResult Hit;

	bool bHit = GetWorld()->SweepSingleByChannel(
		Hit,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		Box
	);

	DrawDebugBox(
		GetWorld(),
		(Start + End) * .5f,
		BoxExtent,
		FColor::Red,
		false,
		.1f
	);

	if (bHit && Hit.GetActor())
	{
		if (!HitActorsThisAttack.Contains(Hit.GetActor()))
		{
			UGameplayStatics::ApplyDamage(
				Hit.GetActor(),
				Damage,
				OwnerCharacter->GetController(),
				OwnerCharacter,
				nullptr
			);
		}
	}
}


// Called every frame
void UAttackComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bHitWindowOpen)
	{
		PerformHitTrace();
	}
}
