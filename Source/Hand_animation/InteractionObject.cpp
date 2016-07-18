// Fill out your copyright notice in the Description page of Project Settings.

#include "Hand_animation.h"
#include "InteractionObject.h"


// Sets default values
AInteractionObject::AInteractionObject()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Create a dummy root component we can attach things to.
	
	USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	RootComponent = SphereComponent;
	SphereComponent->InitSphereRadius(1.0f);
	// RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	OurVisibleComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OurVisibleComponent"));
	OurVisibleComponent->AttachTo(RootComponent);
}

// Called when the game starts or when spawned
void AInteractionObject::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInteractionObject::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// Called to bind functionality to input
void AInteractionObject::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

}

void AInteractionObject::ChangeMesh()
{
	//FString meshActorPath = TEXT("/Game/mano/Character/Basic/mesh_test.mesh_test");
	FString meshActorPath = TEXT("/Game/mano/Character/Basic/pelican_ECA.pelican_ECA");
	//AStaticMeshActor* jaja = Cast<AStaticMeshActor>(StaticLoadObject(AStaticMeshActor::StaticClass(), NULL, *meshActorPath));
	UStaticMesh* jaja = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, *meshActorPath));
	if (jaja == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Casting failed on AInteractionObject::ChangeMesh()")));
	}
	else
	{
		//OurVisibleComponent->SetStaticMesh(jaja->GetStaticMeshComponent()->StaticMesh);
		OurVisibleComponent->SetStaticMesh(jaja);
	}
}

void AInteractionObject::ChangeColor(int32 Color)
{
	ColorIndex = Color;
	ColorChangeEffect();
}
