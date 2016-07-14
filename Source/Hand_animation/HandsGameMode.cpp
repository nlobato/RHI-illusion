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
	bIsExperimentForRHIReplication = true;
	bIsSynchronousActive = true;
	bSpawnObjectsWithTimer = false;
	AmountOfChangesInObject = 1;
}

void AHandsGameMode::BeginPlay()
{
	Super::BeginPlay();
	TimesObjectHasSpawnedCounter = 0;
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
			if (bIsExperimentForDPAlgorithm)
			{
				MyCharacter->ExperimentSetup(true, bAreDPsActive);
				if (bIsMeshToChange)
				{
					MyCharacter->SpawnObject1();
					GetWorldTimerManager().SetTimer(ObjectModificationTimerHandle, this, &AHandsGameMode::ChangeMeshObject, ObjectModificationLifeTime, false);
				}
				else if (bIsSizeToChange)
				{
					MyCharacter->SpawnObject1();
					ChangeSizeObject();
				}
			}
			else if (bIsExperimentForRHIReplication)
			{
				MyCharacter->ExperimentSetup(bIsSynchronousActive, true);
				ObjectIndex.Empty();
				SpawnNewObject();				
			}
			
		}	
	}
		break;
	case EExperimentPlayState::EExperimentFinished:
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Game state is ExperimentFinished")));
		GetWorldTimerManager().ClearTimer(SpawnedObjectTimerHandle);
		GetWorldTimerManager().ClearTimer(ObjectModificationTimerHandle);
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
	if (TimesObjectHasSpawnedCounter > 3)
	{
		ObjectIndex.Empty();
		TimesObjectHasSpawnedCounter = 0;
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
				TimesObjectHasSpawnedCounter++;
				if (bSpawnObjectsWithTimer)
				{
					GetWorldTimerManager().SetTimer(SpawnedObjectTimerHandle, this, &AHandsGameMode::SpawnNewObject, SpawnedObjectLifeTime, false);
				}
			
			break;
		case 2:
			
				MyCharacter->SpawnObject2();
				TimesObjectHasSpawnedCounter++;
				if (bSpawnObjectsWithTimer)
				{
					GetWorldTimerManager().SetTimer(SpawnedObjectTimerHandle, this, &AHandsGameMode::SpawnNewObject, SpawnedObjectLifeTime, false);
				}			
			break;
		case 3:
			
				MyCharacter->SpawnObject3();
				TimesObjectHasSpawnedCounter++;
				if (bSpawnObjectsWithTimer)
				{
					GetWorldTimerManager().SetTimer(SpawnedObjectTimerHandle, this, &AHandsGameMode::SpawnNewObject, SpawnedObjectLifeTime, false);
				}			
			break;
		case 4:
			
				MyCharacter->SpawnObject4();
				TimesObjectHasSpawnedCounter++;
				if (bSpawnObjectsWithTimer)
				{
					GetWorldTimerManager().SetTimer(SpawnedObjectTimerHandle, this, &AHandsGameMode::SpawnNewObject, SpawnedObjectLifeTime, false);
				}			
			break;
		default:
			break;
		}
	}	
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

void AHandsGameMode::ChangeSizeObject()
{
	AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter)
	{
		if (MyCharacter->SpawnedObject)
		{
			float RandomSize = FMath::FRandRange(0.5, 2.0);
			FVector NewScale(RandomSize, RandomSize, RandomSize);
			MyCharacter->SpawnedObject->OurVisibleComponent->SetRelativeScale3D(NewScale);
			MyCharacter->bHasObjectSizeChanged = true;
		}
	}
	GetWorldTimerManager().SetTimer(ObjectModificationTimerHandle, this, &AHandsGameMode::ChangeSizeObject, ((ExperimentDurationTime * 60.f) / AmountOfChangesInObject), false);
}