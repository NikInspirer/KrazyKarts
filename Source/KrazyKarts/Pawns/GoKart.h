// Copyright © 2021 Nikita Kuznetsov. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	AGoKart();

	//~ BEGIN AActor Interface
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ END AActor Interface

protected:
	//~ BEGIN AActor Interface
	virtual void BeginPlay() override;
	//~ END AActor Interface

	/** Mass of the car (kg). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart")
	float Mass;

	/** Higher means more drag (kg/m). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart")
	float DragCoefficient;

	/** Higher means more rolling resistance (kg/m). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart")
	float RollingResistanceCoefficient;

	/** The force applied to the car when the throttle is full down (N). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart")
	float MaxDrivingForce;

	/** Minimum radius of the car turning circle at full lock (m). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart")
	float MinTurningRadius;

private:
	/** Replicated Pawn Location value. */
	UPROPERTY(Replicated)
	FVector ReplicatedLocation;

	/** Replicated Pawn Rotation value. */
	UPROPERTY(Replicated)
	FRotator ReplicatedRotation;
	
	/** Velocity of movement in 3D (m/s). */
	FVector Velocity;
	
	/** Current throttle value for movement. */
	float Throttle;

	/** Current steering throw for rotation. */
	float SteeringThrow;

	/** Client MoveForward call version. */
	void MoveForward(float Value);
	/** Client MoveRight call version. */
	void MoveRight(float Value);

	/** Move kart forward - changes velocity based on specified Value. */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveForward(float Value);

	/** Move kart right - changes steering throw based on specified Value. */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveRight(float Value);

	/** Calculate air resistance to car movement. */
	FVector GetAirResistance() const;

	/** Calculate rolling resistance to car movement. */
	FVector GetRollingResistance() const;

	/** Apply rotation to current Velocity. */
	void ApplyRotation(float DeltaTime);
	
	/** Update location of the car based on current Velocity with collision detection. */
	void UpdateLocationFromVelocity(float DeltaTime);
};
