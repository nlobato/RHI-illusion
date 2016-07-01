// Fill out your copyright notice in the Description page of Project Settings.

#include "Hand_animation.h"
#include "cameradirector.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
Acameradirector::Acameradirector()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void Acameradirector::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void Acameradirector::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	APlayerController* OurPlayerController = UGameplayStatics::GetPlayerController(this, 0);
	OurPlayerController->SetViewTarget(CameraOne);
}

