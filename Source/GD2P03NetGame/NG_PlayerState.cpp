// Fill out your copyright notice in the Description page of Project Settings.


#include "NG_PlayerState.h"
#include "NG_GameMode_Match.h"
#include "Net/UnrealNetwork.h"

void ANG_PlayerState::GiveElimination()
{
	Eliminations++;

	if (ANG_GameMode_Match* GameMode = GetWorld()->GetAuthGameMode<ANG_GameMode_Match>())
	{
		if (GameMode->IsMatchInProgress())
		{
			GameMode->CheckForWinner();
		}
	}
}

int ANG_PlayerState::GetEliminations()
{
	return Eliminations;
}

void ANG_PlayerState::AddScore(int _amount)
{
	CaptureScore += _amount;

	if (ANG_GameMode_Match* GameMode = GetWorld()->GetAuthGameMode<ANG_GameMode_Match>())
	{
		if (GameMode->IsMatchInProgress())
		{
			GameMode->CheckForWinner();
		}
	}
}

int ANG_PlayerState::GetCaptureScore()
{
	return CaptureScore;
}

void ANG_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANG_PlayerState, Eliminations);
	DOREPLIFETIME(ANG_PlayerState, CaptureScore);
}