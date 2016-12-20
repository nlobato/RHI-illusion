// Fill out your copyright notice in the Description page of Project Settings.

#include "Hand_animation.h"
#include "TableActor.h"


// Sets default values
ATableActor::ATableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	RootComponent = SphereComponent;
	SphereComponent->InitSphereRadius(1.0f);
	// RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	OurVisibleComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OurVisibleComponent"));
	OurVisibleComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ATableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATableActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

