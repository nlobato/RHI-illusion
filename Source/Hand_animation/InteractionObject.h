// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "InteractionObject.generated.h"

UCLASS()
class HAND_ANIMATION_API AInteractionObject : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AInteractionObject();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* OurVisibleComponent;

	void ChangeMesh();
	
};
