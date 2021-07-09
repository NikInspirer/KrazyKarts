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
	//~ END AActor Interface

protected:
	//~ BEGIN AActor Interface
	virtual void BeginPlay() override;
	//~ END AActor Interface

	/** Mass of the car (kg). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart")
	float Mass;

	/** The force applied to the car when the throttle is full down (N). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart")
	float MaxDrivingForce;

	/** The number of degrees rotated per second at full control throw (degree/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kart")
	float MaxDegreesPerSecond;

private:
	/** Velocity of movement in 3D (m/s). */
	FVector Velocity;
	
	/** Current throttle value for movement. */
	float Throttle;

	/** Current steering throw for rotation. */
	float SteeringThrow;

	/** Move kart forward - changes velocity based on specified Value. */
	void MoveForward(float Value);

	/** Move kart right - changes steering throw based on specified Value. */
	void MoveRight(float Value);

	/** Update location of the car based on current Velocity with collision detection. */
	void UpdateLocationFromVelocity(float DeltaTime);
};
