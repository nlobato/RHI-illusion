// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "cameradirector.generated.h"

UCLASS()
class HAND_ANIMATION_API Acameradirector : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	Acameradirector();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UPROPERTY(EditAnywhere)
	AActor* CameraOne;
	
};
