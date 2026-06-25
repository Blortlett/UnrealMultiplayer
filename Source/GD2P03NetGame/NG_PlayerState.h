// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NG_PlayerState.generated.h"

/**
 * 
 */
UCLASS()
class GD2P03NETGAME_API ANG_PlayerState : public APlayerState
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(Replicated)
	int Eliminations = 0;

public:

	void GiveElimination();

	UFUNCTION(BlueprintPure)
	int GetEliminations();
};
