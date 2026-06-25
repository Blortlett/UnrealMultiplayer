// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "NG_GameState.generated.h"

/**
 * 
 */
UCLASS()
class GD2P03NETGAME_API ANG_GameState : public AGameState
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(Replicated)
	class ANG_PlayerState* WinningPlayer = nullptr;

public:

	void SetWinner(class ANG_PlayerState* _winner);

	UFUNCTION(BlueprintPure)
	ANG_PlayerState* GetWinner();
};
