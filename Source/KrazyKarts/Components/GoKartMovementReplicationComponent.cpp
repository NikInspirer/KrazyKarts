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
		MovementComponent->SimulateMove(ServerState.LastMove);
	}
}

void UGoKartMovementReplicationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGoKartMovementReplicationComponent, ServerState)
}

void UGoKartMovementReplicationComponent::OnRep_ServerState()
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

