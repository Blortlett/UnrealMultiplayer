// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NG_ControlPoint.generated.h"

class UBoxComponent;
class ANG_PlayerState;

/**
 * A capturable control point. While a single player stands inside the capture
 * zone uncontested, that player is awarded score on an interval. The controlling
 * player is replicated so all clients can show visual feedback (via OnRep).
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

	// How often (seconds) the controlling player is awarded score
	UPROPERTY(EditAnywhere, Category = "ControlPoint")
	float ScoreInterval = 2.0f;

	// Score awarded to the controlling player each interval
	UPROPERTY(EditAnywhere, Category = "ControlPoint")
	int ScorePerInterval = 1;

	// The player currently in sole control (null when empty or contested).
	// Replicated so clients can update the point's appearance.
	UPROPERTY(ReplicatedUsing = OnRep_ControllingState, BlueprintReadOnly, Category = "ControlPoint")
	ANG_PlayerState* ControllingState = nullptr;

	UFUNCTION()
	void OnRep_ControllingState();

	// Server-side: runs on a repeating timer to evaluate capture and award score
	void ServerEvaluateCapture();

	FTimerHandle CaptureTimer;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Designer hook (Blueprint): change material / play effects when control changes.
	// Runs on every machine because it is driven by the replicated ControllingState.
	UFUNCTION(BlueprintImplementableEvent, Category = "ControlPoint")
	void OnControllerChanged(ANG_PlayerState* NewController);
};
