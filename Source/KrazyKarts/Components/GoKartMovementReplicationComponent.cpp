// Copyright © 2021 Nikita Kuznetsov. All Rights Reserved.

#include "GoKartMovementReplicationComponent.h"
#include "Net/UnrealNetwork.h"

UGoKartMovementReplicationComponent::UGoKartMovementReplicationComponent(const FObjectInitializer& ObjectInitializer):
	UActorComponent(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UGoKartMovementReplicationComponent::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = GetOwner()->FindComponentByClass<UGoKartMovementComponent>();
	if (!MovementComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Can not find MovementComponent in %s"), *GetOwner()->GetName())
	}
}

void UGoKartMovementReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// IF I'm client THEN move my pawn and send movement to server
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		UnacknowledgedMoves.Add(MovementComponent->GetLastMove());
		Server_SendMove(MovementComponent->GetLastMove());
	}

	// IF I'm server and controlling pawn THEN move my pawn
	if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		UpdateServerState(MovementComponent->GetLastMove());
	}

	// IF I'm just synchronized with server THEN simulate movement as it was on server
	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		ClientTick(DeltaTime);
	}
}

void UGoKartMovementReplicationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGoKartMovementReplicationComponent, ServerState)
}

void UGoKartMovementReplicationComponent::OnRep_ServerState()
{
	switch (GetOwnerRole())
	{
	case ROLE_AutonomousProxy:
		AutonomousProxy_OnRep_ServerState();
		break;
	case ROLE_SimulatedProxy:
		SimulatedProxy_OnRep_ServerState();
		break;
	default:
		break;
	}
}

void UGoKartMovementReplicationComponent::AutonomousProxy_OnRep_ServerState()
{
	if (!MovementComponent) { return; }

	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);

	ClearUnacknowledgedMoves(ServerState.LastMove);

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}

void UGoKartMovementReplicationComponent::SimulatedProxy_OnRep_ServerState()
{
	if (!MovementComponent) { return; }

	ClientTimeBetweenLastUpdates = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0;
	ClientStartTransform = GetOwner()->GetActorTransform();
	ClientStartVelocity = MovementComponent->GetVelocity();
}

void UGoKartMovementReplicationComponent::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (!MovementComponent) { return; }

	MovementComponent->SimulateMove(Move);
	UpdateServerState(Move);
}

bool UGoKartMovementReplicationComponent::Server_SendMove_Validate(FGoKartMove Move)
{
	// TODO: make better validation
	return true;
}

void UGoKartMovementReplicationComponent::UpdateServerState(const FGoKartMove& Move)
{
	if (!MovementComponent) { return; }

	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

void UGoKartMovementReplicationComponent::ClearUnacknowledgedMoves(const FGoKartMove& LastMove)
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

void UGoKartMovementReplicationComponent::ClientTick(float DeltaTime)
{
	ClientTimeSinceUpdate += DeltaTime;

	// Prevent division big and very small floats to escape errors
	if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER) { return; }
	if (!MovementComponent) { return; }

	const float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates;

	const FHermiteCubicSpline Spline = CreateSpline();
	InterpolateLocation(Spline, LerpRatio);
	InterpolateVelocity(Spline, LerpRatio);
	InterpolateRotation(LerpRatio);
}

float UGoKartMovementReplicationComponent::GetVelocityToDerivative() const
{
	return ClientTimeBetweenLastUpdates * 100;
}

FHermiteCubicSpline UGoKartMovementReplicationComponent::CreateSpline() const
{
	FHermiteCubicSpline Spline;
	Spline.StartLocation = ClientStartTransform.GetLocation();
	Spline.TargetLocation = ServerState.Transform.GetLocation();
	Spline.StartDerivative = ClientStartVelocity * GetVelocityToDerivative();
	Spline.TargetDerivative = ServerState.Velocity * GetVelocityToDerivative();
	return Spline;
}

void UGoKartMovementReplicationComponent::InterpolateLocation(const FHermiteCubicSpline& Spline, float LerpRatio)
{
	const FVector NewLocation = Spline.InterpolateLocation(LerpRatio);
	GetOwner()->SetActorLocation(NewLocation);
}

void UGoKartMovementReplicationComponent::InterpolateVelocity(const FHermiteCubicSpline& Spline, const float LerpRatio)
{
	const FVector NewDerivative = Spline.InterpolateDerivative(LerpRatio);
	const FVector NewVelocity = NewDerivative / GetVelocityToDerivative();
	MovementComponent->SetVelocity(NewVelocity);
}

void UGoKartMovementReplicationComponent::InterpolateRotation(float LerpRatio)
{
	const FQuat TargetRotation = ServerState.Transform.GetRotation();
	const FQuat StartRotation = ClientStartTransform.GetRotation();
	const FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);
	GetOwner()->SetActorRotation(NewRotation);
}
