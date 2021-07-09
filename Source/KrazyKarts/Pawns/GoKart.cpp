// Copyright © 2021 Nikita Kuznetsov. All Rights Reserved.

#include "GoKart.h"

AGoKart::AGoKart()
{
	PrimaryActorTick.bCanEverTick = true;
	Mass = 1000.0f;
	MaxDrivingForce = 10000.0f;
	MaxDegreesPerSecond = 90;
	DragCoefficient = 16;
}

void AGoKart::BeginPlay()
{
	Super::BeginPlay();
}

void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;
	Force += GetResistance();

	const FVector Acceleration = Force / Mass;
	Velocity += Acceleration * DeltaTime;

	ApplyRotation(DeltaTime);
	UpdateLocationFromVelocity(DeltaTime);
}

void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}

void AGoKart::MoveForward(float Value)
{
	Throttle = Value;
}

void AGoKart::MoveRight(float Value)
{
	SteeringThrow = Value;
}

FVector AGoKart::GetResistance() const
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

void AGoKart::ApplyRotation(float DeltaTime)
{
	const float RotationAngle = MaxDegreesPerSecond * SteeringThrow * DeltaTime;
	const FQuat RotationDelta(GetActorUpVector(), FMath::DegreesToRadians(RotationAngle));
	Velocity = RotationDelta.RotateVector(Velocity);
	AddActorWorldRotation(RotationDelta);
}

void AGoKart::UpdateLocationFromVelocity(float DeltaTime)
{
	const FVector WorldOffset = Velocity * 100 * DeltaTime;

	FHitResult Hit;
	AddActorWorldOffset(WorldOffset, true, &Hit);

	// If collision was detected (like hitting a wall)
	if (Hit.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}
