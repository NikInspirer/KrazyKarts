// Copyright © 2021 Nikita Kuznetsov. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "KrazyKarts/Components/GoKartMovementComponent.h"
#include "KrazyKarts/Components/GoKartMovementReplicationComponent.h"
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

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	
	UPROPERTY(VisibleAnywhere)
	UGoKartMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere)
	UGoKartMovementReplicationComponent* ReplicationComponent;
};
