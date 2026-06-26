// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NG_ForceField.generated.h"

/**
 * A barrier that can be toggled on and off (e.g. around a capture zone).
 * When active the mesh is visible and blocks players; when inactive it is
 * hidden and lets them pass. The active state is replicated so every client
 * sees the field appear / disappear (via OnRep). Toggled by ANG_PressurePad.
 */
UCLASS()
class GD2P03NETGAME_API ANG_ForceField : public AActor
{
	GENERATED_BODY()

public:
	ANG_ForceField();

	// Server-only: turn the field on or off. Drives the replicated state.
	UFUNCTION(BlueprintCallable, Category = "ForceField")
	void SetFieldActive(bool bNewActive);

	// True when the field is up (blocking).
	UFUNCTION(BlueprintPure, Category = "ForceField")
	bool IsFieldActive() const { return bFieldActive; }

protected:
	virtual void BeginPlay() override;

	// The visible barrier. Blocks pawns while the field is active.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ForceField")
	UStaticMeshComponent* FieldMesh = nullptr;

	// Whether the field starts up when the level loads.
	UPROPERTY(EditAnywhere, Category = "ForceField")
	bool bStartActive = true;

	// Replicated on/off state. OnRep applies the visuals + collision on clients.
	UPROPERTY(ReplicatedUsing = OnRep_FieldActive, BlueprintReadOnly, Category = "ForceField")
	bool bFieldActive = true;

	UFUNCTION()
	void OnRep_FieldActive();

	// Apply collision + visibility to match bFieldActive (runs on all machines).
	void ApplyFieldState();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Designer hook (Blueprint): play effects / sound when the field toggles.
	// Runs on every machine because it is driven by the replicated state.
	UFUNCTION(BlueprintImplementableEvent, Category = "ForceField")
	void OnFieldStateChanged(bool bNowActive);
};
