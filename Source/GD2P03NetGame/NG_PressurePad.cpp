// Fill out your copyright notice in the Description page of Project Settings.


#include "NG_PressurePad.h"
#include "Components/BoxComponent.h"
#include "GD2P03NetGameCharacter.h"
#include "NG_ForceField.h"
#include "Net/UnrealNetwork.h"

ANG_PressurePad::ANG_PressurePad()
{
	PrimaryActorTick.bCanEverTick = false;

	PadMesh = CreateDefaultSubobject<UStaticMeshComponent>("Pad Mesh");
	RootComponent = PadMesh;
	PadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TriggerZone = CreateDefaultSubobject<UBoxComponent>("Trigger Zone");
	TriggerZone->SetupAttachment(RootComponent);
	TriggerZone->SetBoxExtent(FVector(100.f, 100.f, 50.f));
	TriggerZone->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	bReplicates = true;
}

void ANG_PressurePad::BeginPlay()
{
	Super::BeginPlay();

	// Only the server reacts to players stepping on the pad.
	if (HasAuthority())
	{
		TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &ANG_PressurePad::OnTriggerBeginOverlap);
	}
}

void ANG_PressurePad::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Only players trigger the pad.
	if (!Cast<AGD2P03NetGameCharacter>(OtherActor))
	{
		return;
	}

	// Drop the fields and (re)start the re-raise timer. Stepping on the pad while
	// it is already down just refreshes the countdown.
	if (!bPressed)
	{
		bPressed = true;
		OnRep_Pressed();
		SetLinkedFieldsActive(false);
	}

	GetWorldTimerManager().SetTimer(ResetTimer, this, &ANG_PressurePad::ServerResetPad, FieldDownDuration, false);
}

void ANG_PressurePad::ServerResetPad()
{
	bPressed = false;
	OnRep_Pressed();
	SetLinkedFieldsActive(true);
}

void ANG_PressurePad::SetLinkedFieldsActive(bool bActive)
{
	for (ANG_ForceField* Field : LinkedForceFields)
	{
		if (Field)
		{
			Field->SetFieldActive(bActive);
		}
	}
}

void ANG_PressurePad::OnRep_Pressed()
{
	OnPressedChanged(bPressed);
}

void ANG_PressurePad::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANG_PressurePad, bPressed);
}
