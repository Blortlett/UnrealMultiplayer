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

	// Set by the GameMode in InitGameState; replicated so clients can show "(x/y)".
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
	int32 MinPlayersToStart = 2;

	void SetWinner(class ANG_PlayerState* _winner);

	UFUNCTION(BlueprintPure)
	ANG_PlayerState* GetWinner();

	// True while the level is loaded but the match has not started (lobby fill).
	UFUNCTION(BlueprintPure, Category = "Match")
	bool IsWaitingForPlayers() const;

	// Current connected player count (replicated PlayerArray) — for the lobby overlay.
	UFUNCTION(BlueprintPure, Category = "Match")
	int32 GetNumConnectedPlayers() const;
};
