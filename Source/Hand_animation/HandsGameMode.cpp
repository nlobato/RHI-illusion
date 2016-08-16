// Fill out your copyright notice in the Description page of Project Settings.

#include "Hand_animation.h"
#include "HandsGameMode.h"
#include "Hands_Character.h"
#include "Kismet/GameplayStatics.h"
#include "CalibrationBox.h"
#include "Engine.h"
#include "InteractionObject.h"
#include "Blueprint/UserWidget.h"
#include "Misc/CoreMisc.h"
#include "Misc/Char.h"
#include "Public/StaticMeshResources.h"

AHandsGameMode::AHandsGameMode()
{
	RHIExperimentDurationTime = 5.f;
	IllusionExperimentDurationTime = 5.f;
	VirtualObjectChangesDurationTime = 2.f;
	SpawnedObjectLifeTime = 10.f;
	//MessageToDisplay = 0;
	bIsExperimentForRHIReplication = true;
	bIsSynchronousActive = true;
	bSpawnObjectsWithTimer = false;
	AmountOfChangesInObject = 1;
	bDisplayQuestion = false;
	bDisplayMessage = false;
	bHasAnswerBeenGiven = false;
	SensorsSourceHeight = 123.270798;
	bIsOriginalMesh = true;
}

void AHandsGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	TimesObjectHasSpawnedCounter = 0;
	bHasRealSizeObjectIndexBeenSet = false;
	bIsSystemCalibrated = false;
	bIsShoulderCalibrated = false;
	RealSizeObjectIndexCounter = 0;
	SetObjectNewScale();
	
	InitializeArrays();

	SetCurrentState(EExperimentPlayState::EExperimentInitiated);
}



void AHandsGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	{
		if (CurrentState == EExperimentPlayState::ERHIExperimentInProgress && bSpawnObjectsWithTimer)
		{
			if (GetWorldTimerManager().GetTimerRemaining(SpawnedObjectTimerHandle) <= 5.f)
			{
				if (!MessagesTimerHandle.IsValid())
				{
					MessageToDisplay = EMessages::ERHINewObject;
					GetWorldTimerManager().SetTimer(MessagesTimerHandle, this, &AHandsGameMode::ToggleMessage, 5.0, false);
				}
			}			
		}
		else if (CurrentState == EExperimentPlayState::EDPExperimentInProgress && !ObjectModificationTimerHandle.IsValid())
		{
			if (GetWorldTimerManager().GetTimerRemaining(ExperimentDurationTimerHandle) < 5.f)
			{
				MessageToDisplay = EMessages::ERHINewObject;
				GetWorldTimerManager().SetTimer(MessagesTimerHandle, this, &AHandsGameMode::ToggleMessage, 5.0, false);
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
					MessageToDisplay = EMessages::EExperimentFinishedMessage;
					GetWorldTimerManager().SetTimer(MessagesTimerHandle, this, &AHandsGameMode::ToggleMessage, 5.0, false);
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

EMessages AHandsGameMode::GetCurrentMessage() const
{
	return MessageToDisplay;
}

void AHandsGameMode::HandleNewState(EExperimentPlayState NewState)
{
	switch (NewState)
	{
	case EExperimentPlayState::EExperimentInitiated:
	{
		// Display welcome message
		MessageToDisplay = EMessages::EWelcomeMessage;
		GetWorldTimerManager().SetTimer(MessagesTimerHandle, this, &AHandsGameMode::ToggleMessage, 5.0, false);
	}
		break;
	case EExperimentPlayState::ERHIExperimentInProgress:
	{
		float TimeInSeconds = RHIExperimentDurationTime * 60.f;
		GetWorldTimerManager().SetTimer(ExperimentDurationTimerHandle, this, &AHandsGameMode::HasTimeRunOut, TimeInSeconds, false);
		AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
		if (MyCharacter)
		{
			MyCharacter->CalibrateSystem(AxisTranslation);
			MyCharacter->SetAlphaValue(1.f);
			MessageToDisplay = EMessages::ERHIExperimentInstructions;
			GetWorldTimerManager().SetTimer(MessagesTimerHandle, this, &AHandsGameMode::ToggleMessage, 7.0, false);
			MyCharacter->ExperimentSetup(bIsSynchronousActive, false);
			ObjectIndex.Empty();
			SpawnNewObject();				
			//MyCharacter->SpawnObject4();					
		}	
	}
		break;

	case EExperimentPlayState::EDPExperimentInProgress:
	{
		float TimeInSeconds = IllusionExperimentDurationTime * 60.f;
		GetWorldTimerManager().SetTimer(ExperimentDurationTimerHandle, this, &AHandsGameMode::DPExperimentFirstPartOver, TimeInSeconds, false);
		AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
		if (MyCharacter)
		{
			MyCharacter->CalibrateSystem(AxisTranslation);
			MyCharacter->SetAlphaValue(1.f);
			MessageToDisplay = EMessages::EDPAlgorithmInstructions;
			GetWorldTimerManager().SetTimer(MessagesTimerHandle, this, &AHandsGameMode::ToggleMessage, 7.0, false);
			MyCharacter->ExperimentSetup(true, bAreDPsActive);
			bSpawnObjectsWithTimer = false;		
			//SpawnNewObject();
			MyCharacter->SpawnObject1();	
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
			MessageToDisplay = EMessages::EDPAlgorithmQuestion;
			GetWorldTimerManager().SetTimer(MessagesTimerHandle, this, &AHandsGameMode::ToggleMessage, 5.0, false);					
		}
		else
		{
			MessageToDisplay = EMessages::EExperimentFinishedMessage;
			GetWorldTimerManager().SetTimer(MessagesTimerHandle, this, &AHandsGameMode::ToggleMessage, 5.0, false);
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

void AHandsGameMode::DPExperimentFirstPartOver()
{
	float TimeInSeconds = VirtualObjectChangesDurationTime * 60.f;
	GetWorldTimerManager().SetTimer(ExperimentDurationTimerHandle, this, &AHandsGameMode::HasTimeRunOut, TimeInSeconds, false);
	AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter)
	{
		// Object 4 is the default for DP algorithm
		MyCharacter->SpawnObject4();
		if (bIsMeshToChange)
		{	
			ChangeMeshObject();
		}
		else if (bIsSizeToChange)
		{
			PointerToObjectSpawnedByCharacter = &(MyCharacter->ObjectToSpawn4);
			ChangeSizeObject();
		}
	}
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
			if (bIsOriginalMesh)
			{
				//MyCharacter->SpawnedObject->ChangeMesh();
				OriginalMesh = MyCharacter->SpawnedObject->OurVisibleComponent->StaticMesh;
				MyCharacter->SpawnedObject->OurVisibleComponent->SetStaticMesh(SecondMesh);
				MyCharacter->bAreDPset = false;
				MyCharacter->CurrentMeshIdentificator = 2;
				bIsOriginalMesh = false;
			}
			else
			{
				MyCharacter->SpawnedObject->OurVisibleComponent->SetStaticMesh(OriginalMesh);
				MyCharacter->bAreDPset = false;
				MyCharacter->CurrentMeshIdentificator = 1;
				bIsOriginalMesh = true;
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("No spawned object when casted from AHandsGameMode::ChangeMeshObject()")));
		}
	}
	GetWorldTimerManager().SetTimer(ObjectModificationTimerHandle, this, &AHandsGameMode::ChangeMeshObject, ((VirtualObjectChangesDurationTime * 60.f) / (2 * AmountOfChangesInObject)), false);
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
	GetWorldTimerManager().SetTimer(ObjectModificationTimerHandle, this, &AHandsGameMode::ChangeSizeObject, ((VirtualObjectChangesDurationTime * 60.f) / (2 * AmountOfChangesInObject)), false);
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
	if (!bHasAnswerBeenGiven)
	{

		int32 CorrectAnswer = ObjectSizeChangesArray.Find(FVector(1.f, 1.f, 1.f));
		FString ParticipantNumber = FString::Printf(TEXT("Participant No. %d"), ParticipantCounter);
		FString Answer = FString::Printf(TEXT("Object chosen: %d"), (ObjectChosen + 1));
		FString CorrectAnswerString = FString::Printf(TEXT("Correct answer: %d"), (CorrectAnswer + 1));
		FString Scale = FString::Printf(TEXT("Scale of object selected: %f\n"), ObjectSizeChangesArray[ObjectChosen].X);
		

		FString TextToSave = ParticipantNumber + " " + Answer + " " + CorrectAnswerString + " " + Scale;

		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
		{
			FString AbsoluteFilePath = SaveDirectory + "/" + FileName;
			if (!PlatformFile.FileExists(*AbsoluteFilePath))
			{
				FFileHelper::SaveStringToFile(TextToSave, *AbsoluteFilePath);
			}
			else
			{
				IFileHandle* handle = PlatformFile.OpenWrite(*AbsoluteFilePath, true);
				if (handle)
				{
					handle->Write((const uint8*)TCHAR_TO_ANSI(*TextToSave), TextToSave.Len());					
				}
				delete handle;
			}			
			bHasAnswerBeenGiven = true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Could not find directory"));
		}
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
			MessageToDisplay = EMessages::ECalibrationReady;
			bIsSystemCalibrated = true;
			GetWorldTimerManager().SetTimer(MessagesTimerHandle, this, &AHandsGameMode::ToggleMessage, 2.0f, false);
			if (bIsExperimentForDPAlgorithm)
			{
				SetCurrentState(EExperimentPlayState::EDPExperimentInProgress);
			}
			else
			{
				SetCurrentState(EExperimentPlayState::ERHIExperimentInProgress);
			}
		}
		else
		{
			FVector LeftHandTPose = MyCharacter->GetLeftHandPosition();
			AxisTranslation.Y = LeftHandTPose.Y;
			bIsShoulderCalibrated = true;
			MessageToDisplay = EMessages::ECalibrationInstructions2;
			GetWorldTimerManager().SetTimer(CalibrationTimerHandle, this, &AHandsGameMode::CalibrateSystem, 5.0f, false);
		}
	}	
}

void AHandsGameMode::ReadTextFile(FString AbsolutePathToFile, TArray<FVector>& TargetCoordinatesArray)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	TArray<FString> ArrayForTextFile;

	if (PlatformFile.CreateDirectoryTree(*LoadDirectory))
	{
		if (PlatformFile.FileExists(*AbsolutePathToFile))
		{
			FFileHelper::LoadANSITextFileToStrings(*AbsolutePathToFile, NULL, ArrayForTextFile);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Could not find file"));
		}		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find directory"));
	}

	TargetCoordinatesArray.Empty();
	TargetCoordinatesArray.Reserve(ArrayForTextFile.Num());
	for (FString& EachString : ArrayForTextFile)
	{
		FString StringVector;
		float ComponentX;
		float ComponentY;
		float ComponentZ;
		int32 WhitespaceCounter = 0;
		for (int32 i = 0; i < EachString.Len(); i++)
		{
			if (!EachString.IsValidIndex(i))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index while creating coordinates array"));
			}
			if (!FChar::IsWhitespace(EachString[i]))
			{
				StringVector += EachString[i];
			}
			else
			{
				switch (WhitespaceCounter)
				{
				case 0:
					ComponentX = FCString::Atof(*StringVector);
					StringVector.Empty();
					WhitespaceCounter++;
					break;
				case 1:
					ComponentY = FCString::Atof(*StringVector);
					StringVector.Empty();
					break;
				default:
					break;
				}
			}
		}
		ComponentZ = FCString::Atof(*StringVector);
		TargetCoordinatesArray.Emplace(FVector(ComponentX, ComponentY, ComponentZ));
	}
	
	/*int32 One_Index = FCString::Atoi(*DenseCorrespondenceIndices[123]);
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("One vertex index: %d"), One_Index));
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Array size: %d"), DenseCorrespondenceIndices.Num()));
	AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter)
	{
		TArray<int32> ReturnIndices;
		for (int32 i = 0; i < DenseCorrespondenceIndices.Num(); i++)
		{
			MyCharacter->DenseCorrespondenceIndices.Emplace(FCString::Atoi(*DenseCorrespondenceIndices[i]));
		}
	}*/
}

void AHandsGameMode::ToggleMessage()
{
	if (bIsSystemCalibrated)
	{
		GetWorldTimerManager().ClearTimer(MessagesTimerHandle);
		MessageToDisplay = EMessages::EUnknown;		
	}
	else
	{
		GetWorldTimerManager().ClearTimer(MessagesTimerHandle);
		MessageToDisplay = EMessages::ECalibrationInstructions1;
		GetWorldTimerManager().SetTimer(CalibrationTimerHandle, this, &AHandsGameMode::CalibrateSystem, 5.0f, false);
	}
}

void AHandsGameMode::InitializeArrays()
{
	AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter)
	{
		PtrDenseCorrespondenceCoordinates = &MyCharacter->DenseCorrespondenceCoordinates;
		PtrOriginalMeshVerticesCoordinatesFromObjFile = &MyCharacter->OriginalMeshVerticesCoordinatesFromObjFile;
		PtrSecondMeshVerticesCoordinatesFromObjFile = &MyCharacter->SecondMeshVerticesCoordinatesFromObjFile;

		PtrOriginalMeshVerticesCoordinatesFromUE4Asset = &MyCharacter->OriginalMeshVerticesCoordinatesFromUE4Asset;
		PtrOriginalMeshVerticesNormalsFromUE4Asset = &MyCharacter->OriginalMeshVerticesNormalsFromUE4Asset;
		PtrOriginalMeshVerticesTangentsFromUE4Asset = &MyCharacter->OriginalMeshVerticesTangentsFromUE4Asset;
		PtrOriginalMeshVerticesBinormalsFromUE4Asset = &MyCharacter->OriginalMeshVerticesBinormalsFromUE4Asset;

		PtrSecondMeshVerticesCoordinatesFromUE4Asset = &MyCharacter->SecondMeshVerticesCoordinatesFromUE4Asset;
		PtrSecondMeshVerticesNormalsFromUE4Asset = &MyCharacter->SecondMeshVerticesNormalsFromUE4Asset;
		PtrSecondMeshVerticesTangentsFromUE4Asset = &MyCharacter->SecondMeshVerticesTangentsFromUE4Asset;
		PtrSecondMeshVerticesBinormalsFromUE4Asset = &MyCharacter->SecondMeshVerticesBinormalsFromUE4Asset;

		FString VerticesCorrespondaceFilePath = LoadDirectory + "/" + VerticesCorrespondanceFileName;
		ReadTextFile(VerticesCorrespondaceFilePath, *PtrDenseCorrespondenceCoordinates);
		//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("One vector x: %f y: %f z: %f"), (*PtrDenseCorrespondenceCoordinates)[154].X, (*PtrDenseCorrespondenceCoordinates)[154].Y, (*PtrDenseCorrespondenceCoordinates)[154].Z));
		//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("Array size: %d"), DenseCorrespondenceCoordinates.Num()));

		FString OriginalMeshFilePath = LoadDirectory + "/" + OriginalMeshVerticesCoordinatesFromObjFileName;
		ReadTextFile(OriginalMeshFilePath, *PtrOriginalMeshVerticesCoordinatesFromObjFile);

		FString SecondMeshFilePath = LoadDirectory + "/" + SecondMeshVerticesCoordinatesFromObjFileName;
		ReadTextFile(SecondMeshFilePath, *PtrSecondMeshVerticesCoordinatesFromObjFile);

		AInteractionObject* InteractionObjectForMeshChange = MyCharacter->ObjectToSpawn4.GetDefaultObject();
		if (InteractionObjectForMeshChange)
		{
			AccessMeshVertices(InteractionObjectForMeshChange->OurVisibleComponent->StaticMesh, *PtrOriginalMeshVerticesCoordinatesFromUE4Asset, *PtrOriginalMeshVerticesNormalsFromUE4Asset, *PtrOriginalMeshVerticesTangentsFromUE4Asset, *PtrOriginalMeshVerticesBinormalsFromUE4Asset);
			//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("One vector x: %f y: %f z: %f"), Array_Prueba[5].X, Array_Prueba[5].Y, Array_Prueba[5].Z));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Error while casting to interaction object"));
		}

		if (SecondMesh != nullptr || SecondMesh->RenderData != nullptr)
		{
			AccessMeshVertices(SecondMesh, *PtrSecondMeshVerticesCoordinatesFromUE4Asset, *PtrSecondMeshVerticesNormalsFromUE4Asset, *PtrSecondMeshVerticesTangentsFromUE4Asset, *PtrSecondMeshVerticesBinormalsFromUE4Asset);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Error while trying to acceess second mesh"));
		}

		MapIndicesFromObjToUE4Asset(*PtrOriginalMeshVerticesCoordinatesFromUE4Asset, *PtrOriginalMeshVerticesCoordinatesFromObjFile);
	}
}

void AHandsGameMode::AccessMeshVertices(UStaticMesh* MyMesh, TArray<FVector>& TargetArray, TArray<FVector>& NormalsArray, TArray<FVector>& TangentsArray, TArray<FVector>& BinormalsArray)
{
	TargetArray.Empty();
	NormalsArray.Empty();
	TangentsArray.Empty();
	BinormalsArray.Empty();

	if (MyMesh == nullptr || MyMesh->RenderData == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Error while accesing the static mesh")));
		return;
	}
	FStaticMeshLODResources& LODModel = MyMesh->RenderData->LODResources[0];
	FPositionVertexBuffer& PositionVertexBuffer = LODModel.PositionVertexBuffer;
	if (PositionVertexBuffer.GetNumVertices() == 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Error with the number of vertices")));
		return;
	}
	FIndexArrayView Indices = LODModel.IndexBuffer.GetArrayView();
	uint32 NumIndices = Indices.Num();
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Num indices: %d"), NumIndices));
	for (uint32 i = 0; i < NumIndices; i++)
	{
		FVector Coordinates = PositionVertexBuffer.VertexPosition(Indices[i]);
		TargetArray.Emplace(Coordinates);
		NormalsArray.Emplace(LODModel.VertexBuffer.VertexTangentZ(Indices[i]));
		TangentsArray.Emplace(LODModel.VertexBuffer.VertexTangentX(Indices[i]));
		BinormalsArray.Emplace(LODModel.VertexBuffer.VertexTangentY(Indices[i]));
	}
}

void AHandsGameMode::MapIndicesFromObjToUE4Asset(TArray<FVector>& ArrayFromAsset, TArray<FVector>& ArrayFromObj)
{
	AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter)
	{
		TArray<FArrayForStoringIndices>& MyHugeArray = MyCharacter->MappingBetweenMeshes;
		MyHugeArray.Empty();
		MyHugeArray.Reserve(ArrayFromObj.Num());
		for (int32 i = 0; i < ArrayFromObj.Num(); i++)
		{
			MyHugeArray.Add(FArrayForStoringIndices());
			if (MyHugeArray.IsValidIndex(i))
			{
				MyHugeArray[i].IndicesArray.Empty();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Error while trying to initialize array values"));
				return;
			}
		}
		/*PtrMappingBetweenMeshes->EmptyIndicesArray();
		PtrMappingBetweenMeshes->ReserveMemory(ArrayFromObj.Num());*/
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Num Indices: %d"), ArrayFromAsset.Num()));
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Index 186 x: %f y: %f z: %f"), ArrayFromAsset[186].X, ArrayFromAsset[186].Y, ArrayFromAsset[186].Z));
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Index 187 x: %f y: %f z: %f"), ArrayFromAsset[187].X, ArrayFromAsset[187].Y, ArrayFromAsset[187].Z));
		int32 index_one_one = ArrayFromObj.Find(ArrayFromAsset[187]);
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Index: %d"), index_one_one));
		for (int32 i = 0; i < ArrayFromAsset.Num(); i++)
		{
			if (ArrayFromAsset.IsValidIndex(i))
			{
				int32 index_one = ArrayFromObj.Find(ArrayFromAsset[i]);
				if (MyHugeArray.IsValidIndex(index_one))
				{
					MyHugeArray[index_one].IndicesArray.Emplace(i);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Error while trying to acceess index %d at iteration %d of the int32 array"), index_one, i);
					return;
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Error while trying to acceess the indices"));
				return;
			}
			//IndicesResult.Emplace(ArrayFromAsset.Find(ArrayFromObj[i]));
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Indices: %d"), ArrayFromAsset.Find(ArrayFromObj[i])));
		}
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Num Indices in super array: %d"), MyHugeArray.Num()));
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Num Indices in index 0 of super array: %d"), MyHugeArray[0].IndicesArray.Num()));
		for (int32 i = 0; i < MyHugeArray[0].IndicesArray.Num(); i++)
		{
			if (MyHugeArray[0].IndicesArray.IsValidIndex(i))
			{
				if ((*PtrOriginalMeshVerticesCoordinatesFromUE4Asset).IsValidIndex(MyHugeArray[0].IndicesArray[i]))
				{
					FVector Coordinates = (*PtrOriginalMeshVerticesCoordinatesFromUE4Asset)[MyHugeArray[0].IndicesArray[i]];
					GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Coordinates x: %f y: %f z: %f"), Coordinates.X, Coordinates.Y, Coordinates.Z));
				}
				//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Num Indices in index 0 of super array: %d"), MyHugeArray[0].IndicesArray[i]));
			}
		}
	}
}