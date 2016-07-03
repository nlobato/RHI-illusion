// Fill out your copyright notice in the Description page of Project Settings.

#include "Hand_animation.h"
#include "HandsGameMode.h"
#include "Hands_Character.h"
#include "Kismet/GameplayStatics.h"

AHandsGameMode::AHandsGameMode()
{

}

void AHandsGameMode::BeginPlay()
{
	Super::BeginPlay();

	SetCurrentState(EExperimentPlayState::EStudyInitiated);
}

void AHandsGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter)
	{

	}*/
}

void AHandsGameMode::SetCurrentState(EExperimentPlayState NewState)
{
	// set current state
	CurrentState = NewState;
	// handle current state
	HandleNewState(CurrentState);
}

EExperimentPlayState AHandsGameMode::GetCurrentState() const
{
	return CurrentState;
}

void AHandsGameMode::HandleNewState(EExperimentPlayState NewState)
{
	switch (NewState)
	{
	case EExperimentPlayState::EStudyInitiated:
	{
	}
		break;
		
	case EExperimentPlayState::ESynchronous:
	{

	}
		break;

	case EExperimentPlayState::EAsynchronous:
	{

	}
		break;

	case EExperimentPlayState::EStudyFinished:
	{

	}
		break;

	case EExperimentPlayState::EUnknown:
	{

	}
		break;
	}
}

