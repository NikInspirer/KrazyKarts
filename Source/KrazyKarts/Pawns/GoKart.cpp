// Copyright © 2021 Nikita Kuznetsov. All Rights Reserved.

#include "GoKart.h"

AGoKart::AGoKart()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGoKart::BeginPlay()
{
	Super::BeginPlay();
}

void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	Velocity = GetActorForwardVector() * Value * 20;
}
