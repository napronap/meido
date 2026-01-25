// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MEIDO_5_6_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAttackComponent();

	UFUNCTION(BlueprintCallable)
	void StartAttack();

	UFUNCTION(BlueprintCallable)
	void OpenHitWindow(FName InSocket);

	UFUNCTION(BlueprintCallable)
	void CloseHitWindow();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void PerformHitTrace();

	UPROPERTY()
	ACharacter* OwnerCharacter;

	UPROPERTY(EditAnywhere, Category="Attack")
	float Damage = 10.f;

	UPROPERTY(EditAnywhere, Category="Attack")
	FVector BoxExtent = FVector(20.f, 20.f, 20.f);
	
	UPROPERTY(EditAnywhere, Category="Attack")
	float TraceDistance = 60.f;

	UPROPERTY(EditAnywhere, Category="Attack")
	FName HitSocketName = "Attack_Hand_R";

	bool bIsAttacking = false;
	bool bHitWindowOpen = false;

	TSet<AActor*> HitActorsThisAttack;
	
	FName CurrentHitSocket;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
