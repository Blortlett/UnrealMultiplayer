// Fill out your copyright notice in the Description page of Project Settings.


#include "NG_ControlPoint.h"
#include "Components/BoxComponent.h"
#include "GD2P03NetGameCharacter.h"
#include "NG_PlayerState.h"
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
		GetWorldTimerManager().SetTimer(CaptureTimer, this, &ANG_ControlPoint::ServerEvaluateCapture, ScoreInterval, true);
	}
}

void ANG_ControlPoint::ServerEvaluateCapture()
{
	// Gather the distinct players currently standing in the capture zone
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

	// The point is only captured when exactly one player is present (uncontested)
	ANG_PlayerState* NewController = nullptr;
	if (PlayersInZone.Num() == 1)
	{
		NewController = *PlayersInZone.CreateIterator();
	}

	// Update the replicated controller and run feedback locally on the server
	if (NewController != ControllingState)
	{
		ControllingState = NewController;
		OnRep_ControllingState();
	}

	// Award score to the sole controller (this also checks for a winner)
	if (ControllingState)
	{
		ControllingState->AddScore(ScorePerInterval);
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
