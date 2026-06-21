// Fill out your copyright notice in the Description page of Project Settings.


#include "NG_Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "GD2P03NetGameCharacter.h"

// Sets default values
ANG_Projectile::ANG_Projectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("Projectile Movement");

	SphereCollision = CreateDefaultSubobject<USphereComponent>("Sphere Collision");
	SphereCollision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	SphereCollision->OnComponentHit.AddDynamic(this, &ThisClass::OnProjectileHit);
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
	

	SphereCollision->IgnoreActorWhenMoving(GetOwner(), true);

	if (AGD2P03NetGameCharacter* OwnerCharacter = GetOwner<AGD2P03NetGameCharacter>())
	{
		OwnerCharacter->MoveIgnoreActorAdd(this);
	}
}

void ANG_Projectile::OnProjectileHit(UPrimitiveComponent* _thisHitComp, AActor* _otherActor, UPrimitiveComponent* _otherHitComp, FVector _normalImpulse, const FHitResult& _hitResult)
{
	if (HasAuthority())
	{
		if (AGD2P03NetGameCharacter* HitCharacter = Cast<AGD2P03NetGameCharacter>(_otherActor))
		{
			HitCharacter->NG_TakeDamage(Damage);
		}
		
		Destroy();
	}
}

// Called every frame
void ANG_Projectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

