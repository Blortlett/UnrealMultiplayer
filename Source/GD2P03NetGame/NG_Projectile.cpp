// Fill out your copyright notice in the Description page of Project Settings.


#include "NG_Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
ANG_Projectile::ANG_Projectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("Projectile Movement");
	SphereCollision = CreateDefaultSubobject<USphereComponent>("Sphere Collision");
	RootComponent = SphereCollision;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>("Projectile Mesh");
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetupAttachment(RootComponent);

	bReplicates = true;
}

// Called when the game starts or when spawned
void ANG_Projectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANG_Projectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

