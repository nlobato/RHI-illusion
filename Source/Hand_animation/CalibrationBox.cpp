// Fill out your copyright notice in the Description page of Project Settings.

#include "Hand_animation.h"
#include "CalibrationBox.h"


// Sets default values
ACalibrationBox::ACalibrationBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	DefaultComponent = CreateAbstractDefaultSubobject<USceneComponent>(TEXT("DefaultComponent"));
	RootComponent = DefaultComponent;

	// Create the box component
	CalibrationVolume = CreateAbstractDefaultSubobject<UBoxComponent>(TEXT("CalibrationVolume"));
	CalibrationVolume->AttachTo(RootComponent);

}

// Called when the game starts or when spawned
void ACalibrationBox::BeginPlay()
{
	Super::BeginPlay();
	bIsSystemCalibrated = false;
}

// Called every frame
void ACalibrationBox::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

FVector ACalibrationBox::GetAxisTranslation()
{
	return AxisTranslation;
}

FVector ACalibrationBox::GetMeshScale()
{
	return MeshScale;
}

bool ACalibrationBox::GetSystemCalibrationState()
{
	return bIsSystemCalibrated;
}