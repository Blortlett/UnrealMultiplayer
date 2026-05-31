// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NG_Cube.generated.h"

UCLASS()
class GD2P03NETGAME_API ANG_Cube : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANG_Cube();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Cube")
	UStaticMeshComponent* CubeMesh = nullptr;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
