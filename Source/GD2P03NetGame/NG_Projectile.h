// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NG_Projectile.generated.h"

UCLASS()
class GD2P03NETGAME_API ANG_Projectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANG_Projectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	class UProjectileMovementComponent* ProjectileMovement = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* ProjectileMesh = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class USphereComponent* SphereCollision = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float Damage = 20.f;


	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* _thisHitComp, AActor* _otherActor,
		UPrimitiveComponent* _otherHitComp, FVector _normalImpulse,
		const FHitResult& _hitResult);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
