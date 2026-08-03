// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/AttackComponent.h"
#include "ActorComponents/CharacterStateComponent.h"
#include "ActorComponents/HealthComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Audio/CombatAudioData.h"
#include "Characters/MaidCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Interfaces/CombatTeamSource.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Sound/SoundBase.h"
#include "Types/StateTypes.h"
#include "Utils/CombatFeedbackSubsystem.h"

UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	CurrentHitSockets.Reset();
	if (HitSocketName != NAME_None)
	{
		CurrentHitSockets.Add(HitSocketName);
	}
}

void UAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CloseHitWindow();
	Super::EndPlay(EndPlayReason);
}

AMaidCharacter* UAttackComponent::GetMaidOwner() const
{
	return Cast<AMaidCharacter>(GetOwner());
}

UCharacterStateComponent* UAttackComponent::ResolveStateComponent() const
{
	if (const AActor* OwnerActor = GetOwner())
	{
		return OwnerActor->FindComponentByClass<UCharacterStateComponent>();
	}

	return nullptr;
}

ECombatTeam UAttackComponent::ResolveCombatTeam(const AActor* Actor)
{
	if (!Actor || !Actor->GetClass()->ImplementsInterface(UCombatTeamSource::StaticClass()))
	{
		return ECombatTeam::Neutral;
	}

	return ICombatTeamSource::Execute_GetCombatTeam(Actor);
}

ECombatTeam UAttackComponent::GetOwnerCombatTeam() const
{
	return ResolveCombatTeam(GetOwner());
}

// --- Hit windows ---

void UAttackComponent::StartAttack(const EHitStopType InHitStopType)
{
	CurrentHitStopType = InHitStopType;
	HitActorsThisWindow.Reset();
}

void UAttackComponent::OpenHitWindow(const FName InSocket)
{
	TArray<FName> Sockets;
	if (InSocket != NAME_None)
	{
		Sockets.Add(InSocket);
	}

	OpenHitWindowSockets(Sockets);
}

void UAttackComponent::OpenHitWindowSockets(const TArray<FName>& InSockets)
{
	CurrentHitSockets.Reset();
	for (const FName SocketName : InSockets)
	{
		if (SocketName == NAME_None || CurrentHitSockets.Contains(SocketName))
		{
			continue;
		}

		CurrentHitSockets.Add(SocketName);
	}

	if (CurrentHitSockets.Num() <= 0 && HitSocketName != NAME_None)
	{
		CurrentHitSockets.Add(HitSocketName);
	}

	HitActorsThisWindow.Reset();
	bHitWindowOpen = true;

	if (CombatAudio)
	{
		PlayCombatSfx(CombatAudio->MeleeSwing);
	}
}

void UAttackComponent::CloseHitWindow()
{
	bHitWindowOpen = false;
}

void UAttackComponent::PlayCombatSfx(USoundBase* Sound) const
{
	if (!Sound || !OwnerCharacter)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(World, Sound, OwnerCharacter->GetActorLocation());
}

// --- Combo chain ---

bool UAttackComponent::IsInActiveComboAttack() const
{
	const UCharacterStateComponent* State = ResolveStateComponent();
	if (!State || !State->IsAttacking())
	{
		return false;
	}

	return State->GetAttackState() != EAttackState::WhiffRecover;
}

void UAttackComponent::RequestComboAttack()
{
	if (IsInActiveComboAttack())
	{
		// One buffered press = permission for next section (not a queue).
		bComboInputBuffered = true;
		return;
	}

	StartComboChain();
}

void UAttackComponent::StartComboChain()
{
	AMaidCharacter* Maid = GetMaidOwner();
	if (!Maid || !ComboAttackMontage)
	{
		if (Maid)
		{
			Maid->ApplyGameplayStateIdle();
		}
		return;
	}

	StartAttack(EHitStopType::Light);
	Maid->ApplyGameplayStateAttacking();

	ComboStepIndex = 0;
	bComboInputBuffered = false;

	USkeletalMeshComponent* Mesh = Maid->GetMesh();
	UAnimInstance* AnimInstance = Mesh ? Mesh->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		Maid->ApplyGameplayStateIdle();
		return;
	}

	const float MontageLength = AnimInstance->Montage_Play(
		ComboAttackMontage,
		1.0f,
		EMontagePlayReturnType::MontageLength,
		0.0f,
		true
	);

	if (MontageLength > 0.0f)
	{
		AnimInstance->Montage_SetEndDelegate(Maid->OnAttackMontageEnded, ComboAttackMontage);
	}
	else
	{
		Maid->ApplyGameplayStateIdle();
	}
}

void UAttackComponent::NotifyCheckCombo()
{
	if (!IsInActiveComboAttack())
	{
		return;
	}

	AMaidCharacter* Maid = GetMaidOwner();
	if (!Maid)
	{
		return;
	}

	if (!bComboInputBuffered)
	{
		Maid->ApplyGameplayStateWhiffRecover();
		return;
	}

	bComboInputBuffered = false;
	++ComboStepIndex;

	if (ComboStepIndex < ComboSectionNames.Num())
	{
		if (UAnimInstance* AnimInstance = Maid->GetMesh() ? Maid->GetMesh()->GetAnimInstance() : nullptr)
		{
			AnimInstance->Montage_JumpToSection(ComboSectionNames[ComboStepIndex], ComboAttackMontage);
		}
	}
}

void UAttackComponent::NotifyRecoveryEnd()
{
	AMaidCharacter* Maid = GetMaidOwner();
	if (!Maid || !ComboAttackMontage)
	{
		return;
	}

	if (UAnimInstance* AnimInstance = Maid->GetMesh() ? Maid->GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInstance->Montage_Stop(RecoveryBlendOutTime, ComboAttackMontage);
	}
}

void UAttackComponent::ResetComboRuntime()
{
	ComboStepIndex = 0;
	bComboInputBuffered = false;
}

// --- Trace / damage ---

void UAttackComponent::PerformHitTrace()
{
	if (!OwnerCharacter)
	{
		return;
	}

	USkeletalMeshComponent* OwnerMesh = OwnerCharacter->GetMesh();
	if (!OwnerMesh || CurrentHitSockets.Num() <= 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FCollisionShape Box = FCollisionShape::MakeBox(BoxExtent);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AttackTrace), false, OwnerCharacter);
	QueryParams.AddIgnoredActor(OwnerCharacter);

	for (const FName SocketName : CurrentHitSockets)
	{
		if (SocketName == NAME_None || !OwnerMesh->DoesSocketExist(SocketName))
		{
			continue;
		}

		const FVector Start = OwnerMesh->GetSocketLocation(SocketName);
		const FVector End = Start + OwnerCharacter->GetActorForwardVector() * TraceDistance;

		FHitResult Hit;
		const bool bHit = World->SweepSingleByChannel(
			Hit,
			Start,
			End,
			FQuat::Identity,
			ECC_Pawn,
			Box,
			QueryParams
		);

		if (bDrawDebugHitTrace)
		{
			DrawDebugBox(
				World,
				(Start + End) * 0.5f,
				BoxExtent,
				bHit ? FColor::Green : FColor::Red,
				false,
				0.1f
			);
		}

		if (!bHit || !Hit.GetActor() || HitActorsThisWindow.Contains(Hit.GetActor()))
		{
			continue;
		}

		AActor* HitActor = Hit.GetActor();
		if (ShouldIgnoreHitActor(HitActor))
		{
			continue;
		}

		ApplyDamageToHit(HitActor);
		ApplyHitStopForHit(HitActor);
		HitActorsThisWindow.Add(HitActor);
	}
}

bool UAttackComponent::ShouldIgnoreHitActor(const AActor* HitActor) const
{
	if (!HitActor || !OwnerCharacter)
	{
		return true;
	}

	if (HitActor == OwnerCharacter)
	{
		return true;
	}

	if (const UHealthComponent* TargetHealth = HitActor->FindComponentByClass<UHealthComponent>())
	{
		if (TargetHealth->IsDead())
		{
			return true;
		}
	}

	if (bAllowFriendlyFire)
	{
		return false;
	}

	const ECombatTeam OwnerTeam = ResolveCombatTeam(OwnerCharacter);
	if (OwnerTeam == ECombatTeam::Neutral)
	{
		return false;
	}

	const ECombatTeam TargetTeam = ResolveCombatTeam(HitActor);
	return TargetTeam == OwnerTeam;
}

float UAttackComponent::GetHitStopDuration(const EHitStopType HitStopType) const
{
	switch (HitStopType)
	{
	case EHitStopType::Heavy:
		return HeavyHitStopDuration;
	case EHitStopType::Light:
	default:
		return LightHitStopDuration;
	}
}

void UAttackComponent::ApplyHitStopForHit(AActor* HitActor)
{
	UWorld* World = GetWorld();
	if (!World || !OwnerCharacter || !HitActor)
	{
		return;
	}

	const float Duration = GetHitStopDuration(CurrentHitStopType);
	if (Duration <= 0.f)
	{
		return;
	}

	if (UCombatFeedbackSubsystem* Feedback = World->GetSubsystem<UCombatFeedbackSubsystem>())
	{
		Feedback->ApplyHitStopPair(
			OwnerCharacter,
			HitActor,
			Duration,
			HitStopTimeDilation
		);
	}
}

void UAttackComponent::ApplyDamageToHit(AActor* HitActor)
{
	if (!OwnerCharacter || !HitActor)
	{
		return;
	}

	UGameplayStatics::ApplyDamage(
		HitActor,
		Damage,
		OwnerCharacter->GetController(),
		OwnerCharacter,
		nullptr
	);

	if (CombatAudio)
	{
		PlayCombatSfx(CombatAudio->MeleeHitConfirm);
	}
}

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
