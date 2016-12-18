// Fill out your copyright notice in the Description page of Project Settings.

#include "Hand_animation.h"
#include "Hands_Pawn.h"


// Sets default values
AHands_Pawn::AHands_Pawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AHands_Pawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHands_Pawn::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// Called to bind functionality to input
void AHands_Pawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

}

