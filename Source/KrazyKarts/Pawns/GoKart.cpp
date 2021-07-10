// Copyright © 2021 Nikita Kuznetsov. All Rights Reserved.

#include "GoKart.h"

#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

AGoKart::AGoKart()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Mass = 1000.0f;
	MaxDrivingForce = 10000.0f;
	MinTurningRadius = 10;
	DragCoefficient = 16;
	RollingResistanceCoefficient = 0.015f;
}

void AGoKart::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
}

void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Create FGoKartMove based on user inputs and send it to server ONLY if controlled by this peer
	if (IsLocallyControlled())
	{
		const FGoKartMove Move = CreateMove(DeltaTime);
		if (!HasAuthority()) { UnacknowledgedMoves.Add(Move); }
		Server_SendMove(Move);
		SimulateMove(Move);
	}

	DrawDebugString(GetWorld(), FVector(0, 0, 100), UEnum::GetValueAsString(GetLocalRole()), this,
	                FColor::White, DeltaTime);
}

void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}

void AGoKart::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGoKart, ServerState)
}

void AGoKart::OnRep_ServerState()
{
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;

	ClearUnacknowledgedMoves(ServerState.LastMove);

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		SimulateMove(Move);
	}
}

FGoKartMove AGoKart::CreateMove(float DeltaTime) const
{
	FGoKartMove Move;
	Move.DeltaTime = DeltaTime;
	Move.SteeringThrow = SteeringThrow;
	Move.Throttle = Throttle;
	Move.Time = GetWorld()->TimeSeconds;

	return Move;
}

void AGoKart::ClearUnacknowledgedMoves(const FGoKartMove& LastMove)
{
	TArray<FGoKartMove> NewMoves;
	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		if (LastMove.Time < Move.Time)
		{
			NewMoves.Add(Move);
		}
	}
	UnacknowledgedMoves = NewMoves;
}

void AGoKart::SimulateMove(FGoKartMove Move)
{
	FVector Force = GetActorForwardVector() * MaxDrivingForce * Move.Throttle;
	Force += GetAirResistance();
	Force += GetRollingResistance();

	const FVector Acceleration = Force / Mass;
	Velocity += Acceleration * Move.DeltaTime;

	ApplyRotation(Move.SteeringThrow, Move.DeltaTime);
	UpdateLocationFromVelocity(Move.DeltaTime);
}

void AGoKart::MoveForward(float Value)
{
	Throttle = Value;
}

void AGoKart::MoveRight(float Value)
{
	SteeringThrow = Value;
}

void AGoKart::Server_SendMove_Implementation(FGoKartMove Move)
{
	SimulateMove(Move);

	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = Velocity;
}

bool AGoKart::Server_SendMove_Validate(FGoKartMove Move)
{
	// TODO: make better validation
	return true;
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

void AGoKart::ApplyRotation(float InSteeringThrow, float InDeltaTime)
{
	const float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * InDeltaTime;
	const float RotationAngle = DeltaLocation / MinTurningRadius * InSteeringThrow;
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
