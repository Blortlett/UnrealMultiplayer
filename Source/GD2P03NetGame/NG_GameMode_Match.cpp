// Fill out your copyright notice in the Description page of Project Settings.


#include "NG_GameMode_Match.h"
#include "NG_GameState.h"
#include "NG_PlayerState.h"

void ANG_GameMode_Match::RespawnPlayer(APlayerController* _playerToRespawn)
{
	if (IsMatchInProgress())
	{
		RestartPlayer(_playerToRespawn);
	}
}

void ANG_GameMode_Match::CheckForWinner()
{
	// Check if any player has enough eliminations to win
	if (ANG_GameState* NGGameState = GetGameState<ANG_GameState>())
	{
		for (auto iter : NGGameState->PlayerArray)
		{
			if (ANG_PlayerState* PlayerState = Cast<ANG_PlayerState>(iter))
			{
				if (PlayerState->GetEliminations() >= 5)
				{
					NGGameState->SetWinner(PlayerState);

					EndMatch();

					break;
				}
			}
		}
	}
}

void ANG_GameMode_Match::HandleMatchHasEnded()
{
	GetWorldTimerManager().SetTimer(MatchRestartDelayTimer, this, &AGameMode::RestartGame, 5.0f, false);
}
