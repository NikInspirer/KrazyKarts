// Copyright © 2021 Nikita Kuznetsov. All Rights Reserved.

#include "GoKart.h"

AGoKart::AGoKart()
{
	PrimaryActorTick.bCanEverTick = true;
	Mass = 1000.0f;
	MaxDrivingForce = 10000.0f;
}

void AGoKart::BeginPlay()
{
	Super::BeginPlay();
}

void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;
	const FVector Acceleration = Force / Mass;

	Velocity += Acceleration * DeltaTime;

	const FVector WorldOffset = Velocity * 100 * DeltaTime;
	AddActorWorldOffset(WorldOffset);
}

void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
}

void AGoKart::MoveForward(float Value)
{
	Throttle = Value;
}
