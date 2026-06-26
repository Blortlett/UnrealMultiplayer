// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NG_ControlPoint.generated.h"

class UBoxComponent;
class ANG_PlayerState;
class ANG_ForceField;

/**
 * A capturable control point. A single uncontested player must hold the zone for
 * RequiredHoldTime continuous seconds to score one point. Scoring is gated by the
 * linked force field: only ONE point can be scored per field down-cycle. After a
 * point is scored the player cannot score again until the field raises and is
 * dropped once more (by the pressure pad). The controlling player is replicated so
 * all clients can show visual feedback (via OnRep).
 */
UCLASS()
class GD2P03NETGAME_API ANG_ControlPoint : public AActor
{
	GENERATED_BODY()

public:
	ANG_ControlPoint();

protected:
	virtual void BeginPlay() override;

	// Trigger volume players stand in to capture the point
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ControlPoint")
	UBoxComponent* CaptureZone = nullptr;

	// Visual mesh for the point
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ControlPoint")
	UStaticMeshComponent* PointMesh = nullptr;

	// How often (seconds) the server re-evaluates the zone. Smaller = smoother
	// hold timing. Also the granularity at which hold time accumulates.
	UPROPERTY(EditAnywhere, Category = "ControlPoint")
	float EvaluationInterval = 0.1f;

	// Continuous seconds a sole player must hold the zone to score a point.
	UPROPERTY(EditAnywhere, Category = "ControlPoint")
	float RequiredHoldTime = 5.0f;

	// Score awarded when a capture completes.
	UPROPERTY(EditAnywhere, Category = "ControlPoint")
	int ScorePerCapture = 1;

	// The force field whose down-cycle gates scoring. Exactly one point may be
	// scored each time this field is dropped. Assign the level instance in the
	// editor (the same field the pressure pad lowers).
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "ControlPoint")
	ANG_ForceField* GatingForceField = nullptr;

	// The player currently in sole control (null when empty or contested).
	// Replicated so clients can update the point's appearance.
	UPROPERTY(ReplicatedUsing = OnRep_ControllingState, BlueprintReadOnly, Category = "ControlPoint")
	ANG_PlayerState* ControllingState = nullptr;

	UFUNCTION()
	void OnRep_ControllingState();

	// Server-side: runs on a repeating timer to evaluate capture and award score
	void ServerEvaluateCapture();

	FTimerHandle CaptureTimer;

	// --- Server-only scoring state (not replicated) ---

	// Continuous seconds the current sole occupant has held the zone.
	float HeldTime = 0.f;

	// True once a point has been scored during the current field down-cycle.
	// Blocks further scoring until the field raises and is dropped again.
	bool bScoredThisWindow = false;

	// Last observed field-active state, used to detect the field dropping
	// (active -> inactive), which opens a fresh scoring window.
	bool bPrevFieldActive = true;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Designer hook (Blueprint): change material / play effects when control changes.
	// Runs on every machine because it is driven by the replicated ControllingState.
	UFUNCTION(BlueprintImplementableEvent, Category = "ControlPoint")
	void OnControllerChanged(ANG_PlayerState* NewController);
};
