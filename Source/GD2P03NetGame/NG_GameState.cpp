// Fill out your copyright notice in the Description page of Project Settings.


#include "NG_GameState.h"
#include "NG_PlayerState.h"
#include "GameFramework/GameMode.h"
#include "Net/UnrealNetwork.h"

void ANG_GameState::SetWinner(ANG_PlayerState* _winner)
{
	WinningPlayer = _winner;
}

ANG_PlayerState* ANG_GameState::GetWinner()
{
	return WinningPlayer;
}

bool ANG_GameState::IsWaitingForPlayers() const
{
	// GetMatchState() reads the replicated MatchState, so this is valid on clients too.
	return GetMatchState() == MatchState::WaitingToStart;
}

int32 ANG_GameState::GetNumConnectedPlayers() const
{
	return PlayerArray.Num();
}

void ANG_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANG_GameState, WinningPlayer);
	DOREPLIFETIME(ANG_GameState, MinPlayersToStart);
}
