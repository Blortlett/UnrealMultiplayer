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

	// Score gained through non-elimination interactions (control point capture).
	// Named CaptureScore to avoid shadowing APlayerState's built-in Score.
	UPROPERTY(Replicated)
	int CaptureScore = 0;

public:

	void GiveElimination();

	UFUNCTION(BlueprintPure)
	int GetEliminations();

	// Server-side: award score and check for a winner
	void AddScore(int _amount);

	UFUNCTION(BlueprintPure)
	int GetCaptureScore();
};
