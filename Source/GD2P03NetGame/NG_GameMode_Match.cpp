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
	// Check if any player has reached the score required to win.
	// Score is gained through the control point (a non-elimination interaction).
	if (ANG_GameState* NGGameState = GetGameState<ANG_GameState>())
	{
		for (auto iter : NGGameState->PlayerArray)
		{
			if (ANG_PlayerState* PlayerState = Cast<ANG_PlayerState>(iter))
			{
				if (PlayerState->GetCaptureScore() >= ScoreToWin)
				{
					NGGameState->SetWinner(PlayerState);

					EndMatch();

					break;
				}
			}
		}
	}
}

bool ANG_GameMode_Match::ReadyToStartMatch_Implementation()
{
	// Only start once the lobby has filled to the minimum. Until then the level is
	// loaded and players can move around, but the match stays in WaitingToStart.
	return GetMatchState() == MatchState::WaitingToStart && NumPlayers >= MinPlayersToStart;
}

void ANG_GameMode_Match::InitGameState()
{
	Super::InitGameState();

	if (ANG_GameState* NGGameState = GetGameState<ANG_GameState>())
	{
		NGGameState->MinPlayersToStart = MinPlayersToStart;
	}
}

void ANG_GameMode_Match::HandleMatchHasEnded()
{
	GetWorldTimerManager().SetTimer(MatchRestartDelayTimer, this, &AGameMode::RestartGame, 5.0f, false);
}
