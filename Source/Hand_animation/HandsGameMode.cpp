// Fill out your copyright notice in the Description page of Project Settings.

#include "Hand_animation.h"
#include "HandsGameMode.h"
#include "Hands_Character.h"
#include "Kismet/GameplayStatics.h"
#include "CalibrationBox.h"
#include "Engine.h"
#include "InteractionObject.h"

AHandsGameMode::AHandsGameMode()
{
	ExperimentDurationTime = 5.f;
	SpawnedObjectLifeTime = 10.f;
	ObjectModificationLifeTime = 10.f;
	bIsExperimentSynchronous = true;
	bAreDPsActive = true;
	bSpawnObjectsWithTimer = true;
}

void AHandsGameMode::BeginPlay()
{
	Super::BeginPlay();
	ObjectCount = 0;
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
		float TimeInSeconds = ExperimentDurationTime * 60.f;
		GetWorldTimerManager().SetTimer(ExperimentDurationTimerHandle, this, &AHandsGameMode::HasTimeRunOut, TimeInSeconds, false);
		AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
		if (MyCharacter)
		{
			MyCharacter->CalibrateSystem(AxisTranslation);
			MyCharacter->ExperimentSetup(bIsExperimentSynchronous, bAreDPsActive);
		}
		if (bSpawnObjectsWithTimer)
		{
			ObjectIndex.Empty();
			MyCharacter->SpawnObject1();
			bIsObject1Spawned = true;
			GetWorldTimerManager().SetTimer(SpawnedObjectTimerHandle, this, &AHandsGameMode::SpawnNewObject, SpawnedObjectLifeTime, false);
		}
		else if(bIsMeshToChange)
		{
			MyCharacter->SpawnObject1();
			GetWorldTimerManager().SetTimer(ObjectModificationTimerHandle, this, &AHandsGameMode::ChangeMeshObject, ObjectModificationLifeTime, false);
		}
		
	}
		break;
	case EExperimentPlayState::EExperimentFinished:
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Game state is ExperimentFinished")));
		GetWorldTimerManager().ClearTimer(SpawnedObjectTimerHandle);
		AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
		if (MyCharacter)
		{
			if (MyCharacter->SpawnedObject)
			{
				MyCharacter->SpawnedObject->Destroy();
			}
		}
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
	GetWorldTimerManager().ClearTimer(ExperimentDurationTimerHandle);
	SetCurrentState(EExperimentPlayState::EExperimentFinished);
}

void AHandsGameMode::SpawnNewObject()
{	
	if (ObjectCount > 3)
	{
		ObjectIndex.Empty();
		ObjectCount = 0;
	}
	uint32 RandomObjectIndex = FMath::RandRange(1, 4);
	bool bIsObjectIndexRepeated = ObjectIndex.Contains(RandomObjectIndex);
	while (bIsObjectIndexRepeated)
	{
		RandomObjectIndex = FMath::RandRange(1, 4);
		bIsObjectIndexRepeated = ObjectIndex.Contains(RandomObjectIndex);
	}
	ObjectIndex.Emplace(RandomObjectIndex);
	AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter)
	{
		switch (RandomObjectIndex)
		{
		case 1:
			
				MyCharacter->SpawnObject1();
				ObjectCount++;
				GetWorldTimerManager().SetTimer(SpawnedObjectTimerHandle, this, &AHandsGameMode::SpawnNewObject, SpawnedObjectLifeTime, false);
			
			break;
		case 2:
			
				MyCharacter->SpawnObject2();
				ObjectCount++;
				GetWorldTimerManager().SetTimer(SpawnedObjectTimerHandle, this, &AHandsGameMode::SpawnNewObject, SpawnedObjectLifeTime, false);
			
			break;
		case 3:
			
				MyCharacter->SpawnObject3();
				ObjectCount++;
				GetWorldTimerManager().SetTimer(SpawnedObjectTimerHandle, this, &AHandsGameMode::SpawnNewObject, SpawnedObjectLifeTime, false);
			
			break;
		case 4:
			
				MyCharacter->SpawnObject4();
				ObjectCount++;
				GetWorldTimerManager().SetTimer(SpawnedObjectTimerHandle, this, &AHandsGameMode::SpawnNewObject, SpawnedObjectLifeTime, false);
			
			break;
		default:
			break;
		}
	}
	
	/*if (bIsObject1Spawned)
	{
		AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
		if (MyCharacter)
		{
			MyCharacter->SpawnObject2();
			bIsObject1Spawned = false;
			GetWorldTimerManager().SetTimer(SpawnedObjectTimerHandle, this, &AHandsGameMode::SpawnNewObject, SpawnedObjectLifeTime, false);
		}
	}
	else
	{
		GetWorldTimerManager().ClearTimer(SpawnedObjectTimerHandle);
	}*/
}

void AHandsGameMode::ChangeMeshObject()
{
	AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter)
	{
		if (MyCharacter->SpawnedObject)
		{
			MyCharacter->SpawnedObject->ChangeMesh();
			MyCharacter->bAreDPset = false;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("No spawned object when casted frmo AHandsGameMode::ChangeMeshObject()")));
		}
	}
	GetWorldTimerManager().ClearTimer(ObjectModificationTimerHandle);
}