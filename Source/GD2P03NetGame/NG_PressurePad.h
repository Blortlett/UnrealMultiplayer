// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NG_PressurePad.generated.h"

class UBoxComponent;
class ANG_ForceField;

/**
 * A stepped-on trigger that drops its linked force field(s) for a fixed time,
 * then automatically raises them again (a timed latch, not a hold). Stepping on
 * the pad while it is already down simply refreshes the timer. The pressed state
 * is replicated so all clients can show feedback (via OnRep).
 */
UCLASS()
class GD2P03NETGAME_API ANG_PressurePad : public AActor
{
	GENERATED_BODY()

public:
	ANG_PressurePad();

protected:
	virtual void BeginPlay() override;

	// Visual plate for the pad.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PressurePad")
	UStaticMeshComponent* PadMesh = nullptr;

	// Volume a player steps into to trigger the pad.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PressurePad")
	UBoxComponent* TriggerZone = nullptr;

	// Force fields this pad lowers. Assign the level instances in the editor.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "PressurePad")
	TArray<ANG_ForceField*> LinkedForceFields;

	// How long (seconds) the linked fields stay down after the pad is stepped on.
	UPROPERTY(EditAnywhere, Category = "PressurePad")
	float FieldDownDuration = 12.0f;

	// Replicated so clients can light up the pad while it is active.
	UPROPERTY(ReplicatedUsing = OnRep_Pressed, BlueprintReadOnly, Category = "PressurePad")
	bool bPressed = false;

	UFUNCTION()
	void OnRep_Pressed();

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Server: raise the linked fields again once the timer elapses.
	void ServerResetPad();

	void SetLinkedFieldsActive(bool bActive);

	FTimerHandle ResetTimer;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Designer hook (Blueprint): change material / play sound when the pad toggles.
	UFUNCTION(BlueprintImplementableEvent, Category = "PressurePad")
	void OnPressedChanged(bool bNowPressed);
};
