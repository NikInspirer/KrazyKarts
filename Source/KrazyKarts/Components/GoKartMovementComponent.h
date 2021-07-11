// Copyright © 2021 Nikita Kuznetsov. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.generated.h"

USTRUCT()
struct FGoKartMove
{
	GENERATED_BODY()

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float SteeringThrow;

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float Time;
};

/** Simulate physics of GoKart movement */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KRAZYKARTS_API UGoKartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	explicit UGoKartMovementComponent(const FObjectInitializer& ObjectInitializer);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void SimulateMove(const FGoKartMove& Move);

	FORCEINLINE FVector GetVelocity() const { return Velocity; }
	FORCEINLINE FGoKartMove GetLastMove() const { return LastMove; }

	FORCEINLINE void SetVelocity(const FVector& NewVelocity) { Velocity = NewVelocity; }
	FORCEINLINE void SetThrottle(const float NewThrottle) { Throttle = NewThrottle; }
	FORCEINLINE void SetSteeringThrow(const float NewSteeringThrow) { SteeringThrow = NewSteeringThrow; }

protected:
	virtual void BeginPlay() override;

	/** Mass of the car (kg). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Mass;

	/** Higher means more drag (kg/m). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DragCoefficient;

	/** Higher means more rolling resistance (kg/m). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RollingResistanceCoefficient;

	/** The force applied to the car when the throttle is full down (N). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxDrivingForce;

	/** Minimum radius of the car turning circle at full lock (m). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinTurningRadius;

private:
	FGoKartMove CreateMove(float DeltaTime) const;
	
	FVector GetAirResistance() const;
	FVector GetRollingResistance() const;

	void ApplyRotation(float InSteeringThrow, float DeltaTime);
	void UpdateLocationFromVelocity(float DeltaTime);

	FVector Velocity;
	float Throttle;
	float SteeringThrow;

	FGoKartMove LastMove;
};
