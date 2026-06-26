// Copyright Epic Games, Inc. All Rights Reserved.

#include "GD2P03NetGameCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GD2P03NetGame.h"
#include "NG_Cube.h"
#include "NG_Projectile.h"
#include "Net/UnrealNetwork.h"
#include "GD2P03NetGamePlayerController.h"
#include "NG_PlayerState.h"

AGD2P03NetGameCharacter::AGD2P03NetGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AGD2P03NetGameCharacter::Tick(float _DeltaTime)
{
	Super::Tick(_DeltaTime);

	if (HasAuthority())
	{
		ControlPitch = FRotator::NormalizeAxis(GetControlRotation().Pitch);
	}
}

float AGD2P03NetGameCharacter::GetReplicatedPitch()
{
	if (IsLocallyControlled())
	{
		return FRotator::NormalizeAxis(GetControlRotation().Pitch);
	}

	return ControlPitch;
}

void AGD2P03NetGameCharacter::NG_TakeDamage(float _damage, ANG_PlayerState* _playerThatDealthDamage)
{
	// Runs on the server (called from the projectile's authoritative hit)
	Health -= _damage;

	// Tell everyone to play impermanent hit feedback, and the victim to flash their HUD
	MulticastOnHit(GetActorLocation());
	ClientOnTookDamage();

	if (Health <= 0.f)
	{
		Health = 0.f;
		if (_playerThatDealthDamage)
		{
			_playerThatDealthDamage->GiveElimination();
		}
		Die();
	}
}

void AGD2P03NetGameCharacter::MulticastOnHit_Implementation(FVector _hitLocation)
{
	BP_OnHit(_hitLocation);
}

void AGD2P03NetGameCharacter::ClientOnTookDamage_Implementation()
{
	BP_OnTookDamageLocal();
}

void AGD2P03NetGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGD2P03NetGameCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AGD2P03NetGameCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGD2P03NetGameCharacter::Look);

		// Interacting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AGD2P03NetGameCharacter::Interact);

		// Attacking
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AGD2P03NetGameCharacter::Attack);
	}
	else
	{
		UE_LOG(LogGD2P03NetGame, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AGD2P03NetGameCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AGD2P03NetGameCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AGD2P03NetGameCharacter::Interact(const FInputActionValue& Value)
{
	ServerSpawnCube();
}

void AGD2P03NetGameCharacter::Attack(const FInputActionValue& Value)
{
	ServerAttack();
}

void AGD2P03NetGameCharacter::Die()
{
	GetController<AGD2P03NetGamePlayerController>()->RespawnAfterDelay();
	Destroy();
}

void AGD2P03NetGameCharacter::ServerAttack_Implementation()
{
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(GetMesh()->GetBoneLocation("hand_l"));
	SpawnTransform.SetRotation(GetControlRotation().Quaternion());

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	GetWorld()->SpawnActor<ANG_Projectile>(ProjectileClass, SpawnTransform, SpawnParams);
}

void AGD2P03NetGameCharacter::ServerSpawnCube_Implementation()
{
	if (CubesRemaining <= 0) return;
	CubesRemaining--;

	// Calculate spawn pos
	FTransform SpawnTransform(GetActorLocation() + GetActorForwardVector() * 100.0f);

	// Spawn cube
	ANG_Cube* SpawnedCube = GetWorld()->SpawnActor<ANG_Cube>(CubeClass, SpawnTransform);
}

void AGD2P03NetGameCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AGD2P03NetGameCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AGD2P03NetGameCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AGD2P03NetGameCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AGD2P03NetGameCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGD2P03NetGameCharacter, CubesRemaining);
	DOREPLIFETIME(AGD2P03NetGameCharacter, ControlPitch);
	DOREPLIFETIME(AGD2P03NetGameCharacter, Health);
}