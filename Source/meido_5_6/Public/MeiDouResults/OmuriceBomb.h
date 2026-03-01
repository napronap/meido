// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OmuriceBomb.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UParticleSystem;
class USoundBase;

UCLASS()
class MEIDO_5_6_API AOmuriceBomb : public AActor
{
	GENERATED_BODY()
	
public:	
	AOmuriceBomb();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, Category="Bomb")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category="Bomb")
	TObjectPtr<UStaticMeshComponent> BombMesh;

	UPROPERTY(EditAnywhere, Category="Bomb|Fall")
	float InitialHeightOffset = 1800.f;

	UPROPERTY(EditAnywhere, Category="Bomb|Fall", meta=(ClampMin="0.0"))
	float FallSpeed = 1800.f;

	UPROPERTY(EditAnywhere, Category="Bomb|Impact", meta=(ClampMin="0.0"))
	float ImpactDamage = 50.f;

	UPROPERTY(EditAnywhere, Category="Bomb|Impact", meta=(ClampMin="0.0"))
	float ImpactRadius = 450.f;

	UPROPERTY(EditAnywhere, Category="Bomb|Impact", meta=(ClampMin="0.0"))
	float DestroyDelay = 0.2f;

	UPROPERTY(EditAnywhere, Category="Bomb|Impact")
	TObjectPtr<UParticleSystem> ImpactVFX = nullptr;

	UPROPERTY(EditAnywhere, Category="Bomb|Impact")
	TObjectPtr<USoundBase> ImpactSFX = nullptr;

	UPROPERTY(EditAnywhere, Category="Bomb|Debug")
	bool bDrawImpactDebug = false;

	bool bHasImpacted = false;

	void HandleImpact();

};
