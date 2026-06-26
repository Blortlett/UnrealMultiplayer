// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "NG_GameMode_Match.generated.h"

/**
 * 
 */
UCLASS()
class GD2P03NETGAME_API ANG_GameMode_Match : public AGameMode
{
	GENERATED_BODY()
	
public:
	void RespawnPlayer(APlayerController* _playerToRespawn);

	void CheckForWinner();

	// Score a player must reach to win the match (brief minimum: 3).
	// Exposed so the host can configure it before the session begins.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Match")
	int ScoreToWin = 3;

protected:
	virtual void HandleMatchHasEnded() override;

	FTimerHandle MatchRestartDelayTimer;
};
