// Fill out your copyright notice in the Description page of Project Settings.

#include "Hand_animation.h"
#include "HandsGameMode.h"
#include "Hands_Character.h"
#include "Kismet/GameplayStatics.h"
#include "CalibrationBox.h"
#include "Engine.h"

AHandsGameMode::AHandsGameMode()
{
	ExperimentDurationTime = 5.f;
	bIsExperimentSynchronous = true;
	bAreDPsActive = true;
}

void AHandsGameMode::BeginPlay()
{
	Super::BeginPlay();

	SetCurrentState(EExperimentPlayState::EExperimentInitiated);
}

void AHandsGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	{
		if (CurrentState == EExperimentPlayState::EExperimentInitiated)
		{

			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACalibrationBox::StaticClass(), FoundActors);
			for (auto Actor : FoundActors)
			{
				ACalibrationBox* CalibrationBox = Cast<ACalibrationBox>(Actor);
				if (CalibrationBox)
				{
					if (CalibrationBox->GetSystemCalibrationState())
					{
						AxisTranslation = CalibrationBox->GetAxisTranslation();
						SetCurrentState(EExperimentPlayState::EExperimentInProgress);

					}
				}
			}
		}
	}
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
	case EExperimentPlayState::EExperimentInitiated:
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Game state is ExperimentInitiated")));
	}
		break;
	case EExperimentPlayState::EExperimentInProgress:
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Game state is ExperimentInProgress")));
		AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
		if (MyCharacter)
		{
			MyCharacter->CalibrateSystem(AxisTranslation);
			MyCharacter->ExperimentSetup(bIsExperimentSynchronous, bAreDPsActive);
		}
		float TimeInSeconds = ExperimentDurationTime * 60.f;
		GetWorldTimerManager().SetTimer(ExperimentDurationTimerHandle, this, &AHandsGameMode::HasTimeRunOut, TimeInSeconds, false);
	}
		break;
	case EExperimentPlayState::EExperimentFinished:
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Game state is ExperimentFinished")));
	}
		break;

	case EExperimentPlayState::EUnknown:
	{

	}
		break;
	}
}

void AHandsGameMode::HasTimeRunOut()
{
	SetCurrentState(EExperimentPlayState::EExperimentFinished);
}