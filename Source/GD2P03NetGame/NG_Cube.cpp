// Fill out your copyright notice in the Description page of Project Settings.


#include "NG_Cube.h"

// Sets default values
ANG_Cube::ANG_Cube()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>("Cube Mesh");
	RootComponent = CubeMesh;

	bReplicates = true;
}

// Called when the game starts or when spawned
void ANG_Cube::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANG_Cube::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

