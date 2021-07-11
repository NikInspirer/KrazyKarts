// Copyright © 2021 Nikita Kuznetsov. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GoKartMovementComponent.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementReplicationComponent.generated.h"

USTRUCT()
struct FGoKartState
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FGoKartMove LastMove;
};

struct FHermiteCubicSpline
{
	FVector StartLocation;
	FVector StartDerivative;
	FVector TargetLocation;
	FVector TargetDerivative;

	FORCEINLINE FVector InterpolateLocation(const float LerpRatio) const
	{
		return FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}

	FORCEINLINE FVector InterpolateDerivative(const float LerpRatio) const
	{
		return FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative,
		                                    LerpRatio);
	}
};

/** Replicates GoKart movement. */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KRAZYKARTS_API UGoKartMovementReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	explicit UGoKartMovementReplicationComponent(const FObjectInitializer& ObjectInitializer);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void SetMeshOffsetRoot(USceneComponent* Root) { MeshOffsetRoot = Root; }

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnRep_ServerState();
	void AutonomousProxy_OnRep_ServerState();
	void SimulatedProxy_OnRep_ServerState();

	/** Send to server GoKart move data. */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove Move);

	void UpdateServerState(const FGoKartMove& Move);
	void ClearUnacknowledgedMoves(const FGoKartMove& LastMove);

	void ClientTick(float DeltaTime);
	float GetVelocityToDerivative() const;
	FHermiteCubicSpline CreateSpline() const;
	void InterpolateLocation(const FHermiteCubicSpline& Spline, float LerpRatio);
	void InterpolateVelocity(const FHermiteCubicSpline& Spline, float LerpRatio);
	void InterpolateRotation(float LerpRatio);

	/** GoKart state from Server. */
	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FGoKartState ServerState;

	UPROPERTY()
	UGoKartMovementComponent* MovementComponent;

	float ClientTimeSinceUpdate;
	float ClientTimeBetweenLastUpdates;
	FTransform ClientStartTransform;
	FVector ClientStartVelocity;

	TArray<FGoKartMove> UnacknowledgedMoves;

	UPROPERTY()
	USceneComponent* MeshOffsetRoot;
};
