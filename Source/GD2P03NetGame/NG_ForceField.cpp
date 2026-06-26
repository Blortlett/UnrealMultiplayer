// Fill out your copyright notice in the Description page of Project Settings.


#include "NG_ForceField.h"
#include "Net/UnrealNetwork.h"

ANG_ForceField::ANG_ForceField()
{
	PrimaryActorTick.bCanEverTick = false;

	FieldMesh = CreateDefaultSubobject<UStaticMeshComponent>("Field Mesh");
	RootComponent = FieldMesh;
	// Block pawns by default; ApplyFieldState() relaxes this when the field is down.
	FieldMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	bReplicates = true;
}

void ANG_ForceField::BeginPlay()
{
	Super::BeginPlay();

	// Initialise from the designer's choice (server owns the authoritative state).
	if (HasAuthority())
	{
		bFieldActive = bStartActive;
	}

	// Make visuals + collision match the current state on every machine.
	ApplyFieldState();
}

void ANG_ForceField::SetFieldActive(bool bNewActive)
{
	// Authority only: changing the replicated value pushes it to clients.
	if (!HasAuthority() || bFieldActive == bNewActive)
	{
		return;
	}

	bFieldActive = bNewActive;
	// OnRep does not fire on the server, so apply locally here too.
	OnRep_FieldActive();
}

void ANG_ForceField::OnRep_FieldActive()
{
	ApplyFieldState();
	OnFieldStateChanged(bFieldActive);
}

void ANG_ForceField::ApplyFieldState()
{
	if (!FieldMesh)
	{
		return;
	}

	// Up: visible and blocking. Down: hidden and passable.
	FieldMesh->SetVisibility(bFieldActive);
	FieldMesh->SetCollisionEnabled(bFieldActive ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
}

void ANG_ForceField::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANG_ForceField, bFieldActive);
}
