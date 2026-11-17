#include "IA/MaidAIMovement.h"

#include "AIController.h"
#include "Characters/MaidCharacter.h"
#include "Engine/World.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"

FVector MeidoAIMovement::ResolveDirection(AMaidCharacter* InMaid, const FVector& InDestination,
	const float InRepathInterval, const float InPointAcceptanceRadius, float& InOutNextRepathTime,
	int32& InOutPathPointIndex, TArray<FVector>& InOutPathPoints)
{
	UWorld* MovementWorld = InMaid ? InMaid->GetWorld() : nullptr;
	if (!MovementWorld)
	{
		return FVector::ZeroVector;
	}
	const float MovementTime = MovementWorld->GetTimeSeconds();
	if (MovementTime >= InOutNextRepathTime)
	{
		InOutPathPoints.Reset();
		InOutPathPointIndex = 1;
		// Throttle failed path queries too; a missing nav mesh must not trigger one per frame.
		InOutNextRepathTime = MovementTime + FMath::Max(0.05f, InRepathInterval);
		UNavigationSystemV1* Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(MovementWorld);
		FNavLocation ProjectedDestination;
		if (Navigation && Navigation->ProjectPointToNavigation(InDestination, ProjectedDestination, FVector(100.f, 100.f, 250.f)))
		{
			if (UNavigationPath* MovementPath = UNavigationSystemV1::FindPathToLocationSynchronously(
				MovementWorld, InMaid->GetActorLocation(), ProjectedDestination.Location, InMaid))
			{
				if (MovementPath->IsValid())
				{
					InOutPathPoints = MovementPath->PathPoints;
				}
			}
		}
	}
	while (InOutPathPoints.IsValidIndex(InOutPathPointIndex))
	{
		const FVector ToPathPoint = InOutPathPoints[InOutPathPointIndex] - InMaid->GetActorLocation();
		if (ToPathPoint.SizeSquared2D() <= FMath::Square(FMath::Max(5.f, InPointAcceptanceRadius)))
		{
			++InOutPathPointIndex;
			continue;
		}
		return ToPathPoint.GetSafeNormal2D();
	}
	return FVector::ZeroVector;
}

void MeidoAIMovement::ApplyInput(AMaidCharacter* InMaid, AAIController* InController,
	const FVector& InWorldInput, const FVector& InFacingLocation)
{
	if (!InMaid || !InController)
	{
		return;
	}
	const FVector FacingDirection = (InFacingLocation - InMaid->GetActorLocation()).GetSafeNormal2D();
	if (!FacingDirection.IsNearlyZero())
	{
		InController->SetControlRotation(FRotator(0.f, FacingDirection.Rotation().Yaw, 0.f));
	}
	const FRotator InputYaw(0.f, InController->GetControlRotation().Yaw, 0.f);
	const FVector ForwardAxis = FRotationMatrix(InputYaw).GetUnitAxis(EAxis::X);
	const FVector RightAxis = FRotationMatrix(InputYaw).GetUnitAxis(EAxis::Y);
	const FVector ClampedInput = InWorldInput.GetClampedToMaxSize(1.f);
	InMaid->DoMove(FVector::DotProduct(ClampedInput, RightAxis), FVector::DotProduct(ClampedInput, ForwardAxis));
}
