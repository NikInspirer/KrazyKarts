// Copyright © 2021 Nikita Kuznetsov. All Rights Reserved.

#include "GoKart.h"

#include "DrawDebugHelpers.h"

AGoKart::AGoKart()
{
	PrimaryActorTick.bCanEverTick = true;
	Mass = 1000.0f;
	MaxDrivingForce = 10000.0f;
	MinTurningRadius = 10;
	DragCoefficient = 16;
	RollingResistanceCoefficient = 0.015f;
}

void AGoKart::BeginPlay()
{
	Super::BeginPlay();
}

void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;
	Force += GetAirResistance();

	const FVector Acceleration = Force / Mass;
	Velocity += Acceleration * DeltaTime;

	ApplyRotation(DeltaTime);
	UpdateLocationFromVelocity(DeltaTime);

	DrawDebugString(GetWorld(), FVector(0, 0, 100), UEnum::GetValueAsString(GetLocalRole()), this,
	                FColor::White, DeltaTime);
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
	Server_MoveForward(Value);
}

void AGoKart::MoveRight(float Value)
{
	SteeringThrow = Value;
	Server_MoveRight(Value);
}

void AGoKart::Server_MoveForward_Implementation(float Value)
{
	Throttle = Value;
}

bool AGoKart::Server_MoveForward_Validate(float Value)
{
	return FMath::Abs(Value) <= 1.0f;
}

void AGoKart::Server_MoveRight_Implementation(float Value)
{
	SteeringThrow = Value;
}

bool AGoKart::Server_MoveRight_Validate(float Value)
{
	return FMath::Abs(Value) <= 1.0f;
}

FVector AGoKart::GetAirResistance() const
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector AGoKart::GetRollingResistance() const
{
	const float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100.0f;
	const float NormalForce = Mass * AccelerationDueToGravity;

	return -Velocity.GetSafeNormal() * RollingResistanceCoefficient * NormalForce;
}

void AGoKart::ApplyRotation(float DeltaTime)
{
	const float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
	const float RotationAngle = DeltaLocation / MinTurningRadius * SteeringThrow;
	const FQuat RotationDelta(GetActorUpVector(), RotationAngle);

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
