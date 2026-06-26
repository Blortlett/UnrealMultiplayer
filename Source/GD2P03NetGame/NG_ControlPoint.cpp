// Fill out your copyright notice in the Description page of Project Settings.


#include "NG_ControlPoint.h"
#include "Components/BoxComponent.h"
#include "GD2P03NetGameCharacter.h"
#include "NG_PlayerState.h"
#include "NG_ForceField.h"
#include "Net/UnrealNetwork.h"

ANG_ControlPoint::ANG_ControlPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	PointMesh = CreateDefaultSubobject<UStaticMeshComponent>("Point Mesh");
	RootComponent = PointMesh;
	PointMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CaptureZone = CreateDefaultSubobject<UBoxComponent>("Capture Zone");
	CaptureZone->SetupAttachment(RootComponent);
	CaptureZone->SetBoxExtent(FVector(200.f, 200.f, 200.f));
	CaptureZone->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	bReplicates = true;
}

void ANG_ControlPoint::BeginPlay()
{
	Super::BeginPlay();

	// Only the server evaluates capture and awards score
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(CaptureTimer, this, &ANG_ControlPoint::ServerEvaluateCapture, EvaluationInterval, true);
	}
}

void ANG_ControlPoint::ServerEvaluateCapture()
{
	// --- Track the gating field's down-cycle (one score per field toggle) ---
	// When no field is assigned, scoring is ungated (the field is treated as down).
	const bool bHasGate = (GatingForceField != nullptr);
	const bool bFieldActive = bHasGate ? GatingForceField->IsFieldActive() : false;

	// The moment the field drops (active -> inactive) opens a fresh scoring window.
	if (bHasGate && bPrevFieldActive && !bFieldActive)
	{
		bScoredThisWindow = false;
		HeldTime = 0.f;
	}
	bPrevFieldActive = bFieldActive;

	// --- Find the sole, uncontested occupant of the zone ---
	TArray<AActor*> OverlappingActors;
	CaptureZone->GetOverlappingActors(OverlappingActors, AGD2P03NetGameCharacter::StaticClass());

	TSet<ANG_PlayerState*> PlayersInZone;
	for (AActor* Actor : OverlappingActors)
	{
		if (AGD2P03NetGameCharacter* Character = Cast<AGD2P03NetGameCharacter>(Actor))
		{
			if (ANG_PlayerState* PlayerState = Character->GetPlayerState<ANG_PlayerState>())
			{
				PlayersInZone.Add(PlayerState);
			}
		}
	}

	// The point is only held when exactly one player is present (uncontested)
	ANG_PlayerState* NewController = nullptr;
	if (PlayersInZone.Num() == 1)
	{
		NewController = *PlayersInZone.CreateIterator();
	}

	// Update the replicated controller and run feedback locally on the server.
	// A change of holder (including becoming empty/contested) restarts the hold.
	if (NewController != ControllingState)
	{
		ControllingState = NewController;
		OnRep_ControllingState();
		HeldTime = 0.f;
	}

	// --- Accumulate hold time; award a single point once per down-cycle ---
	// Scoring is open only while the field is down and no point has been scored
	// yet during this window.
	const bool bScoringOpen = !bFieldActive && !bScoredThisWindow;
	if (ControllingState && bScoringOpen)
	{
		HeldTime += EvaluationInterval;
		if (HeldTime >= RequiredHoldTime)
		{
			ControllingState->AddScore(ScorePerCapture);
			HeldTime = 0.f;

			// Lock out further scoring until the field cycles again. (When there
			// is no gating field this stays open so the point keeps working.)
			if (bHasGate)
			{
				bScoredThisWindow = true;
			}
		}
	}
	else
	{
		HeldTime = 0.f;
	}
}

void ANG_ControlPoint::OnRep_ControllingState()
{
	OnControllerChanged(ControllingState);
}

void ANG_ControlPoint::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANG_ControlPoint, ControllingState);
}
