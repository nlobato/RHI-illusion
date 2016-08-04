// Fill out your copyright notice in the Description page of Project Settings.

#include "Hand_animation.h"
#include "HandsGameMode.h"
#include "Hands_Character.h"
#include "Kismet/GameplayStatics.h"
#include "CalibrationBox.h"
#include "Engine.h"
#include "InteractionObject.h"
#include "Blueprint/UserWidget.h"

AHandsGameMode::AHandsGameMode()
{
	ExperimentDurationTime = 5.f;
	SpawnedObjectLifeTime = 10.f;
	ObjectModificationLifeTime = 10.f;
	MessageToDisplay = 0;
	bIsExperimentForRHIReplication = true;
	bIsSynchronousActive = true;
	bSpawnObjectsWithTimer = false;
	AmountOfChangesInObject = 1;
	bDisplayQuestion = false;
	bDisplayMessage = false;
	SensorsSourceHeight = 123.270798;
}

void AHandsGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	if (HUDWidgetClass != nullptr)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
		}
	}
	TimesObjectHasSpawnedCounter = 0;
	bHasRealSizeObjectIndexBeenSet = false;
	RealSizeObjectIndexCounter = 0;
	SetObjectNewScale();
	SetCurrentState(EExperimentPlayState::EExperimentInitiated);
}

void AHandsGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	{
		/*if (CurrentState == EExperimentPlayState::EExperimentInitiated)
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
		}*/
		if (CurrentState == EExperimentPlayState::EExperimentInProgress && bSpawnObjectsWithTimer)
		{
			if (GetWorldTimerManager().GetTimerRemaining(SpawnedObjectTimerHandle) < 5.f)
			{
				bDisplayMessage = true;
			}			
		}
		else if (CurrentState == EExperimentPlayState::EExperimentFinished)
		{
			AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
			if (MyCharacter)
			{
				if (MyCharacter->bIsDecisionMade)
				{
					DecisionEvaluation(MyCharacter->ObjectChosen);
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
		// Display welcome message
		MessageToDisplay = 1;
		// Enter calibration mode
		GetWorldTimerManager().SetTimer(CalibrationTimerHandle, this, &AHandsGameMode::CalibrateSystem, 5.0f, false);
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
			MyCharacter->SetAlphaValue(1.f);
			if (bIsExperimentForDPAlgorithm)
			{
				MyCharacter->ExperimentSetup(true, bAreDPsActive);
				bSpawnObjectsWithTimer = false;
				if (bIsMeshToChange)
				{
					MyCharacter->SpawnObject1();
					GetWorldTimerManager().SetTimer(ObjectModificationTimerHandle, this, &AHandsGameMode::ChangeMeshObject, ObjectModificationLifeTime, false);
				}
				else if (bIsSizeToChange)
				{			
					//SpawnNewObject();
					MyCharacter->SpawnObject1();
					PointerToObjectSpawnedByCharacter = &(MyCharacter->ObjectToSpawn1);
					ChangeSizeObject();
				}
			}
			else if (bIsExperimentForRHIReplication)
			{
				MyCharacter->ExperimentSetup(bIsSynchronousActive, true);
				ObjectIndex.Empty();
				SpawnNewObject();				
				//MyCharacter->SpawnObject1();
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
			MyCharacter->SetAlphaValue(0.f);
			MyCharacter->bIsExperimentFinished = true;
			RootLocation = MyCharacter->MyMesh->GetSocketLocation(TEXT("spine_02"));
		}
		if (bIsExperimentForDPAlgorithm)
		{
			SpawnObjectsForDecision();
			bDisplayQuestion = true;
			for (FVector i : ObjectSizeChangesArray)
			{
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Scale: %f"), i.X));
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
	if (TimesObjectHasSpawnedCounter > 1)
	{
		ObjectIndex.Empty();
		TimesObjectHasSpawnedCounter = 0;
	}
	uint32 RandomObjectIndex = FMath::RandRange(1, 2);
	bool bIsObjectIndexRepeated = ObjectIndex.Contains(RandomObjectIndex);
	while (bIsObjectIndexRepeated)
	{
		RandomObjectIndex = FMath::RandRange(1, 2);
		bIsObjectIndexRepeated = ObjectIndex.Contains(RandomObjectIndex);
	}
	ObjectIndex.Emplace(RandomObjectIndex);
	AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter)
	{
		switch (RandomObjectIndex)
		{
		case 1:
			
			bDisplayMessage = false;
			MyCharacter->SpawnObject1();
			PointerToObjectSpawnedByCharacter = &(MyCharacter->ObjectToSpawn1);
			TimesObjectHasSpawnedCounter++;
			if (bSpawnObjectsWithTimer)
			{
				GetWorldTimerManager().SetTimer(SpawnedObjectTimerHandle, this, &AHandsGameMode::SpawnNewObject, SpawnedObjectLifeTime, false);
			}			
			break;
		
		case 2:
			bDisplayMessage = false;
			MyCharacter->SpawnObject4();
			PointerToObjectSpawnedByCharacter = &(MyCharacter->ObjectToSpawn4);
			TimesObjectHasSpawnedCounter++;
			if (bSpawnObjectsWithTimer)
			{
				GetWorldTimerManager().SetTimer(SpawnedObjectTimerHandle, this, &AHandsGameMode::SpawnNewObject, SpawnedObjectLifeTime, false);
			}	
			break;
		case 3:
			
				MyCharacter->SpawnObject3();
				PointerToObjectSpawnedByCharacter = &(MyCharacter->ObjectToSpawn3);
				TimesObjectHasSpawnedCounter++;
				if (bSpawnObjectsWithTimer)
				{
					GetWorldTimerManager().SetTimer(SpawnedObjectTimerHandle, this, &AHandsGameMode::SpawnNewObject, SpawnedObjectLifeTime, false);
				}			
			break;
		case 4:
			
				MyCharacter->SpawnObject4();
				PointerToObjectSpawnedByCharacter = &(MyCharacter->ObjectToSpawn4);
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
			FVector NewScale = ObjectSizeChangesArray[RealSizeObjectIndexCounter];
			MyCharacter->SpawnedObject->OurVisibleComponent->SetRelativeScale3D(NewScale);
			MyCharacter->bHasObjectSizeChanged = true;			
		}
		MyCharacter->SpawnedObject->ChangeColor(RealSizeObjectIndexCounter);
		RealSizeObjectIndexCounter++;
	}
	if (RealSizeObjectIndexCounter > 3)
	{
		RealSizeObjectIndexCounter = 0;
	}
	GetWorldTimerManager().SetTimer(ObjectModificationTimerHandle, this, &AHandsGameMode::ChangeSizeObject, ((ExperimentDurationTime * 60.f) / (2 * AmountOfChangesInObject)), false);
}

void AHandsGameMode::SetObjectNewScale()
{
	ObjectSizeChangesArray.Empty();
	float RandomSizeLow = FMath::FRandRange(0.6, 0.85);
	float RandomSizeHigh = FMath::FRandRange(1.15, 1.4);
	float Decision = FMath::RandRange(1, 2);
	if(Decision == 1)
	{
		ObjectSizeChangesArray.Emplace(FVector(RandomSizeLow, RandomSizeLow, RandomSizeLow));
		ObjectSizeChangesArray.Emplace(FVector(RandomSizeHigh, RandomSizeHigh, RandomSizeHigh));
	}
	else
	{
		ObjectSizeChangesArray.Emplace(FVector(RandomSizeHigh, RandomSizeHigh, RandomSizeHigh));
		ObjectSizeChangesArray.Emplace(FVector(RandomSizeLow, RandomSizeLow, RandomSizeLow));
	}
	RandomSizeLow = FMath::FRandRange(0.6, 0.9);
	RandomSizeHigh = FMath::FRandRange(1.1, 1.4);
	if (Decision == 1)
	{
		ObjectSizeChangesArray.Emplace(FVector(RandomSizeLow, RandomSizeLow, RandomSizeLow));		
	}
	else
	{
		ObjectSizeChangesArray.Emplace(FVector(RandomSizeHigh, RandomSizeHigh, RandomSizeHigh));
	}
	RealSizeObjectIndex = FMath::RandRange(0, AmountOfChangesInObject - 1);
	ObjectSizeChangesArray.Insert(FVector(1.f, 1.f, 1.f), RealSizeObjectIndex);
}

void AHandsGameMode::SpawnObjectsForDecision()
{
	float Offset = -30;
	int32 contador = 0;
	for (uint32 i = 0; i <= (AmountOfChangesInObject - 1); i++)
	{
		UWorld* const World = GetWorld();
		if (World)
		{
			// Set the spawn parameters
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;

			// spawn the pickup
			FVector PositionForObject = FVector(45.f, Offset, -15.f) + RootLocation;

			AInteractionObject* const DecisionObject = World->SpawnActor<AInteractionObject>(*PointerToObjectSpawnedByCharacter, PositionForObject, FRotator(0.f, 0.f, 0.f), SpawnParams);
			if (DecisionObject)
			{
				if (i != RealSizeObjectIndex)
				{
					DecisionObject->OurVisibleComponent->SetRelativeScale3D(ObjectSizeChangesArray[i]);
					DecisionObject->ChangeColor(i);
				}
				else
				{
					DecisionObject->ChangeColor(i);
				}
			}
		}
		Offset += 20.f;
		contador++;
	}
}

void AHandsGameMode::DecisionEvaluation(int32 ObjectChosen)
{
	int32 CorrectAnswer = ObjectSizeChangesArray.Find(FVector(1.f, 1.f, 1.f));
	if (CorrectAnswer == ObjectChosen)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Congrats! Correct answer")));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Too bad! You missed :(")));
	}
}

void AHandsGameMode::CalibrateSystem()
{	
	AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter)
	{
		if (bIsShoulderCalibrated)
		{
			FVector LeftHandNavel = MyCharacter->GetLeftHandPosition();		
			AxisTranslation.X = LeftHandNavel.X;
			AxisTranslation.Z = -(SensorsSourceHeight - LeftHandNavel.Z);
			MessageToDisplay = 3;
			GetWorldTimerManager().ClearTimer(CalibrationTimerHandle);
			SetCurrentState(EExperimentPlayState::EExperimentInProgress);
		}
		else
		{
			FVector LeftHandTPose = MyCharacter->GetLeftHandPosition();
			AxisTranslation.Y = LeftHandTPose.Y;
			bIsShoulderCalibrated = true;
			MessageToDisplay = 2;
			GetWorldTimerManager().SetTimer(CalibrationTimerHandle, this, &AHandsGameMode::CalibrateSystem, 5.0f, false);
		}
	}	
}