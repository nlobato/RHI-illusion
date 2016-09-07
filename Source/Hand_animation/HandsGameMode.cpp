// Fill out your copyright notice in the Description page of Project Settings.

#include "Hand_animation.h"
#include "HandsGameMode.h"
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
	UE_LOG(LogTemp, Warning, TEXT("About to enter initialization of arrays"));
	
	if (bIsSizeToChange || bIsMeshToChange)
	{
		InitializeArrays();
	}

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
				MyCharacter->bHasObjectMeshChanged = true;
				bIsOriginalMesh = false;
			}
			else
			{
				MyCharacter->SpawnedObject->OurVisibleComponent->SetStaticMesh(OriginalMesh);
				MyCharacter->bAreDPset = false;
				MyCharacter->CurrentMeshIdentificator = 1;
				MyCharacter->bHasObjectMeshChanged = false;
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

void AHandsGameMode::InitializeArrays()
{
	UE_LOG(LogTemp, Warning, TEXT("We are in"));
	AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter)
	{
		
		PtrDenseCorrespondenceIndices = &MyCharacter->DenseCorrespondenceIndices;
		//PtrDenseCorrespondenceCoordinates = &MyCharacter->DenseCorrespondenceCoordinates;
		//PtrOriginalMeshVerticesCoordinatesFromObjFile = &MyCharacter->OriginalMeshVerticesCoordinatesFromObjFile;
		//PtrSecondMeshVerticesCoordinatesFromObjFile = &MyCharacter->SecondMeshVerticesCoordinatesFromObjFile;

		//PtrMapped1stMeshCorrespondences = &MyCharacter->Mapped1stMeshCorrespondences;
		//PtrMapped2ndMeshCorrespondences = &MyCharacter->Mapped2ndMeshCorrespondences;
		PtrMesh2Mesh1Correspondences = &MyCharacter->Mesh2Mesh1Correspondences;

		PtrOriginalMeshVertices = &MyCharacter->OriginalMeshVertices;
		PtrOriginalMeshNormals = &MyCharacter->OriginalMeshNormals;
		PtrOriginalMeshTangents = &MyCharacter->OriginalMeshTangents;
		PtrOriginalMeshBinormals = &MyCharacter->OriginalMeshBinormals;
		PtrOriginalMeshIndices = &MyCharacter->OriginalMeshIndices;

		PtrSecondMeshVertices = &MyCharacter->SecondMeshVertices;
		PtrSecondMeshNormals = &MyCharacter->SecondMeshNormals;
		PtrSecondMeshTangents = &MyCharacter->SecondMeshTangents;
		PtrSecondMeshBinormals = &MyCharacter->SecondMeshBinormals;
		PtrSecondMeshIndices = &MyCharacter->SecondMeshIndices;

		UE_LOG(LogTemp, Warning, TEXT("Pointers created succesfully"));
		
		if (bIsSizeToChange)
		{
			AInteractionObject* InteractionObjectForMeshChange = MyCharacter->ObjectToSpawn4.GetDefaultObject();
			UStaticMesh* OneMesh = MyCharacter->ObjectToSpawn4->GetDefaultObject<AInteractionObject>()->OurVisibleComponent->StaticMesh;
			
			if (InteractionObjectForMeshChange)
			{
				AccessMeshVertices(OneMesh, OriginalMeshVerticesCoordinatesFromObjFile, *PtrOriginalMeshVertices, *PtrOriginalMeshNormals, *PtrOriginalMeshTangents, *PtrOriginalMeshBinormals);
				
				UE_LOG(LogTemp, Warning, TEXT("Succesfully accesed the vertices of Original mesh"));
			}
			
			if (SecondMesh != nullptr || SecondMesh->RenderData != nullptr)
			{
				AccessMeshVertices(SecondMesh, SecondMeshVerticesCoordinatesFromObjFile, *PtrSecondMeshVertices, *PtrSecondMeshNormals, *PtrSecondMeshTangents, *PtrSecondMeshBinormals);
				
				UE_LOG(LogTemp, Warning, TEXT("Succesfully accesed the vertices of second mesh"));
			}			
		}
		else
		{
			TArray<int32> OriginalMeshAsset2ObjIndicesMap;
			TArray<int32> SecondMeshAsset2ObjIndicesMap;
			TArray<int32> OriginalMeshTangentsIndicesMap;

			TArray<FVector> OriginalMeshVerticesFromUE4Asset;
			TArray<FVector> OriginalMeshNormalsFromUE4Asset;
			TArray<FVector> OriginalMeshTangentsFrom;
			TArray<FVector> OriginalMeshBlendedMapBinormals;

			TArray<FVector> SecondMeshVerticesFromUE4Asset;
			TArray<FVector> SecondMeshNormalsFromUE4Asset;
			TArray<FVector> SecondMeshTangentsFrom;
			TArray<FVector> SecondMeshBlendedMapBinormals;
			
			TArray<int32> BlendedIntrinsicMapsTrianglesMap;
			TArray<FVector> BlendedIntrinsicMapsBarycentricCoordinates;

			// Read the text file with the dense correspondence indices
			/*FString DenseCorrespondaceIndicesFilePath = LoadDirectory + "/" + DenseCorrespondanceIndicesFileName;
			ReadTextFile(DenseCorrespondaceIndicesFilePath, *PtrDenseCorrespondenceIndices);*/

			// Read the Blended Intrinsic Maps text file 
			FString BlendedIntrinsicMapsFilePath = LoadDirectory + "/" + BlendedIntrinsicMapsFileName;
			ReadTextFile(BlendedIntrinsicMapsFilePath, BlendedIntrinsicMapsTrianglesMap, BlendedIntrinsicMapsBarycentricCoordinates);
			UE_LOG(LogTemp, Warning, TEXT("BlendedIntrinsicMaps text file read succesfully"));
			int32 test_index = 1234;
			if (BlendedIntrinsicMapsBarycentricCoordinates.IsValidIndex(test_index))
			{
				UE_LOG(LogTemp, Warning, TEXT("Index %d barycentric coordinates are x: %f y: %f z: %f"), test_index, BlendedIntrinsicMapsBarycentricCoordinates[test_index].X, BlendedIntrinsicMapsBarycentricCoordinates[test_index].Y, BlendedIntrinsicMapsBarycentricCoordinates[test_index].Z);
			}

			// Read the text file with the OBJ coordinates and triangle indices for the first mesh
			FString OriginalMeshFilePath = LoadDirectory + "/" + OriginalMeshVerticesCoordinatesFromObjFileName;
			ReadTextFile(OriginalMeshFilePath, OriginalMeshVerticesCoordinatesFromObjFile);
			//UE_LOG(LogTemp, Warning, TEXT("Obj[1201] x: %f y: %f z: %f"), OriginalMeshVerticesCoordinatesFromObjFile[1201].X, OriginalMeshVerticesCoordinatesFromObjFile[1201].Y, OriginalMeshVerticesCoordinatesFromObjFile[1201].Z);

			UE_LOG(LogTemp, Warning, TEXT("1st obj text file read succesfully"));

			// Read the text file with the OBJ coordinates and triangle indices for the 2nd mesh
			FString SecondMeshFilePath = LoadDirectory + "/" + SecondMeshVerticesCoordinatesFromObjFileName;
			ReadTextFile(SecondMeshFilePath, SecondMeshVerticesCoordinatesFromObjFile, SecondMeshTriangleIndicesFromObjFile);
			UE_LOG(LogTemp, Warning, TEXT("Triangle Indices Array size: %d"), SecondMeshTriangleIndicesFromObjFile.Num());
			/*int32 test_index = 2699;
			if (SecondMeshTriangleIndicesFromObjFile.IsValidIndex(test_index))
			{
			UE_LOG(LogTemp, Warning, TEXT("Triangle %d vertices are 1st: %d 2nd: %d 3rd: %d"), test_index, int32(SecondMeshTriangleIndicesFromObjFile[test_index].X), int32(SecondMeshTriangleIndicesFromObjFile[test_index].Y), int32(SecondMeshTriangleIndicesFromObjFile[test_index].Z));
			}*/

			UE_LOG(LogTemp, Warning, TEXT("2nd obj text file read succesfully"));

			AInteractionObject* InteractionObjectForMeshChange = MyCharacter->ObjectToSpawn4.GetDefaultObject();
			UStaticMesh* OneMesh = MyCharacter->ObjectToSpawn4->GetDefaultObject<AInteractionObject>()->OurVisibleComponent->StaticMesh;
			//PointerToObjectSpawnedByCharacter = &MyCharacter->ObjectToSpawn4;
			//if (PointerToObjectSpawnedByCharacter)
			if (InteractionObjectForMeshChange)
			{
				AccessMeshVertices(OneMesh, OriginalMeshVerticesCoordinatesFromObjFile, OriginalMeshVerticesFromUE4Asset, OriginalMeshNormalsFromUE4Asset, OriginalMeshAsset2ObjIndicesMap);
				
				UE_LOG(LogTemp, Warning, TEXT("Succesfully accesed the vertices of Original mesh"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("InteractionObjectForMeshChange is invalid. Located at AHandsGameMode::InitializeArrays()"));
			}


			if (SecondMesh != nullptr || SecondMesh->RenderData != nullptr)
			{			
				AccessMeshVertices(SecondMesh, SecondMeshVerticesCoordinatesFromObjFile, SecondMeshVerticesFromUE4Asset, SecondMeshNormalsFromUE4Asset, SecondMeshAsset2ObjIndicesMap);
				
				UE_LOG(LogTemp, Warning, TEXT("Succesfully accesed the vertices of second mesh"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("SecondMesh or SecondMesh->RenderData are invalid. Located at AHandsGameMode::InitializeArrays()"));
			}

			//MappingTriangles(SecondMesh, SecondMeshVerticesCoordinatesFromObjFile, SecondMeshVerticesFromUE4Asset, SecondMeshNormalsFromUE4Asset, SecondMeshTriangleIndicesFromObjFile, MapTriangleIndices);

			MappingTriangles(SecondMeshAsset2ObjIndicesMap, SecondMeshTriangleIndicesFromObjFile, SecondMeshMapTriangleIndices);

			OriginalMeshTangentComputation(OriginalMeshVerticesFromUE4Asset, OriginalMeshNormalsFromUE4Asset, *PtrOriginalMeshVertices, *PtrOriginalMeshNormals, *PtrOriginalMeshTangents, *PtrOriginalMeshBinormals, OriginalMeshTangentsIndicesMap);

			SecondMeshTangentComputation(*PtrSecondMeshVertices, *PtrSecondMeshNormals, *PtrSecondMeshTangents, *PtrSecondMeshBinormals, BlendedIntrinsicMapsTrianglesMap, 
				BlendedIntrinsicMapsBarycentricCoordinates, SecondMeshMapTriangleIndices, SecondMeshVerticesFromUE4Asset, SecondMeshNormalsFromUE4Asset, 
				OriginalMeshTangentsIndicesMap, OriginalMeshAsset2ObjIndicesMap);

			//Map2ndMeshCorrespondences(*PtrSecondMeshVerticesCoordinatesFromUE4Asset, SecondMeshVerticesCoordinatesFromObjFile, Mapped2ndMeshCorrespondences);

			//UE_LOG(LogTemp, Warning, TEXT("Succesfully mapped the correspondances of second mesh"));

			//MappingBetweenMeshes(*PtrOriginalMeshVerticesCoordinatesFromUE4Asset, OriginalMeshVerticesCoordinatesFromObjFile, *PtrMesh2Mesh1Correspondences);
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
			UE_LOG(LogTemp, Warning, TEXT("Could not find file at AHandsGameMode::ReadTextFile(TargetCoordinatessArray)"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find directory at AHandsGameMode::ReadTextFile(TargetCoordinatessArray)"));
	}

	TargetCoordinatesArray.Empty();
	TargetCoordinatesArray.Reserve(ArrayForTextFile.Num() / 3);
	for (FString& EachString : ArrayForTextFile)
	{
		FString StringVector;

		float ComponentX = 0;
		float ComponentY = 0;
		float ComponentZ = 0;
		bool bHasXValueBeenSet = false;
		for (int32 i = 0; i < EachString.Len(); i++)
		{
			if (!EachString.IsValidIndex(i))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index while creating coordinates array"));
				return;
			}
			if (!FChar::IsWhitespace(EachString[i]))
			{
				StringVector += EachString[i];
			}
			else
			{
				if (!bHasXValueBeenSet)
				{
					ComponentX = FCString::Atof(*StringVector);
					StringVector.Empty();
					bHasXValueBeenSet = true;
				}
				else
				{
					ComponentY = FCString::Atof(*StringVector);
					StringVector.Empty();
				}
			}
		}
		ComponentZ = FCString::Atof(*StringVector);
		TargetCoordinatesArray.Emplace(FVector(ComponentX, ComponentY, ComponentZ));

	}
}

void AHandsGameMode::ReadTextFile(FString AbsolutePathToFile, TArray<FVector>& TargetCoordinatesArray, TArray<FVector>& TargetTriangleIndices)
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
			UE_LOG(LogTemp, Warning, TEXT("Could not find file at AHandsGameMode::ReadTextFile(TargetCoordinatessArray)"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find directory at AHandsGameMode::ReadTextFile(TargetCoordinatessArray)"));
	}

	TargetCoordinatesArray.Empty();
	TargetCoordinatesArray.Reserve(ArrayForTextFile.Num() / 3);
	TargetTriangleIndices.Empty();
	TargetTriangleIndices.Reserve(ArrayForTextFile.Num() * (2 / 3));
	for (FString& EachString : ArrayForTextFile)
	{
		FString StringVector;

		if (EachString[0] == TCHAR('f'))
		{
			int32 FirstVertex;
			int32 SecondVertex;
			int32 ThirdVertex;
			bool bHasXValueBeenSet = false;
			bool bHasYValueBeenSet = false;
			bool bHasDashBeenFound = false;
			for (int32 i = 2; i < EachString.Len(); i++)
			{
				if (!EachString.IsValidIndex(i))
				{
					UE_LOG(LogTemp, Warning, TEXT("Invalid index while creating coordinates array"));
					return;
				}
				if (!FChar::IsWhitespace(EachString[i]))
				{
					if (EachString[i] == TCHAR('/') || bHasDashBeenFound == true)
					{
						bHasDashBeenFound = true;
					}
					else
					{
						StringVector += EachString[i];
					}
				}
				else
				{
					if (!bHasXValueBeenSet)
					{
						FirstVertex = FCString::Atoi(*StringVector);
						//UE_LOG(LogTemp, Warning, TEXT("First Vertex %d"), FirstVertex);
						StringVector.Empty();
						bHasXValueBeenSet = true;
						bHasDashBeenFound = false;
					}
					else if (!bHasYValueBeenSet)
					{
						SecondVertex = FCString::Atoi(*StringVector);
						StringVector.Empty();
						bHasYValueBeenSet = true;
						bHasDashBeenFound = false;
					}
					else
					{
						ThirdVertex = FCString::Atoi(*StringVector);
					}
				}
			}
			TargetTriangleIndices.Emplace(FVector(FirstVertex, SecondVertex, ThirdVertex));
		}
		else
		{

			float ComponentX = 0;
			float ComponentY = 0;
			float ComponentZ = 0;
			bool bHasXValueBeenSet = false;
			for (int32 i = 0; i < EachString.Len(); i++)
			{
				if (!EachString.IsValidIndex(i))
				{
					UE_LOG(LogTemp, Warning, TEXT("Invalid index while creating coordinates array"));
					return;
				}
				if (!FChar::IsWhitespace(EachString[i]))
				{
					StringVector += EachString[i];
				}
				else
				{
					if (!bHasXValueBeenSet)
					{
						ComponentX = FCString::Atof(*StringVector);
						StringVector.Empty();
						bHasXValueBeenSet = true;
					}
					else
					{
						ComponentY = FCString::Atof(*StringVector);
						StringVector.Empty();
					}
				}
			}
			ComponentZ = FCString::Atof(*StringVector);
			TargetCoordinatesArray.Emplace(FVector(ComponentX, ComponentY, ComponentZ));
		}
	}
}

void AHandsGameMode::ReadTextFile(FString AbsolutePathToFile, TArray<int32>& TargetIndicesArray)
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
			UE_LOG(LogTemp, Warning, TEXT("Could not find file at AHandsGameMode::ReadTextFile(TargetIndicesArray)"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find directory at  AHandsGameMode::ReadTextFile(TargetIndicesArray)"));
	}

	TargetIndicesArray.Empty();
	TargetIndicesArray.Reserve(ArrayForTextFile.Num());
	for (FString& EachString : ArrayForTextFile)
	{
		TargetIndicesArray.Emplace(FCString::Atod(*EachString));
	}
}

void AHandsGameMode::ReadTextFile(FString AbsolutePathToFile, TArray<int32>& TargetIndicesArray, TArray<FVector>& TargetCoordinatesArray)
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
			UE_LOG(LogTemp, Warning, TEXT("Could not find file %s at AHandsGameMode::ReadTextFile()"), *AbsolutePathToFile);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find directory %s at AHandsGameMode::ReadTextFile()"), *LoadDirectory);
	}

	TargetCoordinatesArray.Empty();
	TargetIndicesArray.Empty();
	TargetCoordinatesArray.Reserve(ArrayForTextFile.Num());
	TargetIndicesArray.Reserve(ArrayForTextFile.Num());

	for (FString& EachString : ArrayForTextFile)
	{
		//FString& EachString = ArrayForTextFile[i];
		FString StringVector;

		float ComponentX = 0;
		float ComponentY = 0;
		float ComponentZ = 0;
		int32 TriangleIndex = 0;
		bool bHasXValueBeenSet = false;
		bool bHasIndexBeenSet = false;
		for (int32 i = 0; i < EachString.Len(); i++)
		{
			if (!EachString.IsValidIndex(i))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index while creating coordinates array"));
				return;
			}
			if (!FChar::IsWhitespace(EachString[i]))
			{
				StringVector += EachString[i];
			}
			else
			{
				if (!bHasIndexBeenSet)
				{
					TriangleIndex = FCString::Atod(*StringVector);
					StringVector.Empty();
					bHasIndexBeenSet = true;
				}
				else if (!bHasXValueBeenSet)
				{
					ComponentX = FCString::Atof(*StringVector);
					StringVector.Empty();
					bHasXValueBeenSet = true;
				}
				else
				{
					ComponentY = FCString::Atof(*StringVector);
					StringVector.Empty();
				}
			}
		}
		ComponentZ = FCString::Atof(*StringVector);
		TargetCoordinatesArray.Emplace(FVector(ComponentX, ComponentY, ComponentZ));
		TargetIndicesArray.Emplace(TriangleIndex);
	}
}

void AHandsGameMode::AccessMeshVertices(UStaticMesh* MyMesh, TArray<FVector>& ArrayFromObj, TArray<FVector>& TargetVerticesArray, TArray<FVector>& TargetNormalsArray, TArray<FVector>& TargetTangentsArray, TArray<FVector>& TargetBinormalsArray)
{
	TargetVerticesArray.Empty();
	TargetNormalsArray.Empty();
	TargetTangentsArray.Empty();
	TargetBinormalsArray.Empty();
	
	if (MyMesh == nullptr || MyMesh->RenderData == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("MyMesh or MyMesh->RenderData are invalid. Located at AHandsGameMode::AccessMeshVertices()"));
		return;
	}
	FStaticMeshLODResources& LODModel = MyMesh->RenderData->LODResources[0];
	FPositionVertexBuffer& PositionVertexBuffer = LODModel.PositionVertexBuffer;
	if (PositionVertexBuffer.GetNumVertices() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("PositionVertexBuffer.GetNumVertices() == 0. Located at AHandsGameMode::AccessMeshVertices()"));
		return;
	}
	FIndexArrayView Indices = LODModel.IndexBuffer.GetArrayView();
	TArray<uint32> IndicesFromBuffer;
	LODModel.IndexBuffer.GetCopy(IndicesFromBuffer);
	int32 NumIndices = Indices.Num();
	
	TargetVerticesArray.Reserve(NumIndices);
	TargetNormalsArray.Reserve(NumIndices);
	TargetTangentsArray.Reserve(NumIndices);
	TargetBinormalsArray.Reserve(NumIndices);
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Num indices: %d"), NumIndices));
	for (int32 i = 0; i < NumIndices; i++)
	{
		FVector Coordinates = PositionVertexBuffer.VertexPosition(Indices[i]);
		TargetVerticesArray.Emplace(Coordinates);
		TargetNormalsArray.Emplace(LODModel.VertexBuffer.VertexTangentZ(Indices[i]));
		TargetTangentsArray.Emplace(LODModel.VertexBuffer.VertexTangentX(Indices[i]));
		TargetBinormalsArray.Emplace(LODModel.VertexBuffer.VertexTangentY(Indices[i]));		
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Test array size %d"), TargetVerticesArray.Num());
}

void AHandsGameMode::AccessMeshVertices(UStaticMesh* MyMesh, TArray<FVector>& ArrayFromObj, TArray<FVector>& TargetVerticesArray, TArray<FVector>& TargetNormalsArray, TArray<int32>& TargetIndicesMapArray)
{
	TargetVerticesArray.Empty();
	TargetNormalsArray.Empty();
	TargetIndicesMapArray.Empty();

	if (MyMesh == nullptr || MyMesh->RenderData == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("MyMesh or MyMesh->RenderData are invalid. Located at AHandsGameMode::AccessMeshVertices()"));
		return;
	}
	FStaticMeshLODResources& LODModel = MyMesh->RenderData->LODResources[0];
	FPositionVertexBuffer& PositionVertexBuffer = LODModel.PositionVertexBuffer;
	if (PositionVertexBuffer.GetNumVertices() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("PositionVertexBuffer.GetNumVertices() == 0. Located at AHandsGameMode::AccessMeshVertices()"));
		return;
	}
	FIndexArrayView Indices = LODModel.IndexBuffer.GetArrayView();
	TArray<uint32> IndicesFromBuffer;
	LODModel.IndexBuffer.GetCopy(IndicesFromBuffer);
	int32 NumIndices = Indices.Num();

	int32 test_index = 147;

	TargetVerticesArray.Reserve(NumIndices);
	TargetNormalsArray.Reserve(NumIndices);
	int32 contar = 0;
	for (int32 i = 0; i < NumIndices; i++)
	{
		TargetNormalsArray.Emplace(LODModel.VertexBuffer.VertexTangentZ(Indices[i]));
		
		FVector Coordinates = PositionVertexBuffer.VertexPosition(Indices[i]);
		//TargetVerticesArray.Emplace(Coordinates);
		int32 Index;
		if (!ArrayFromObj.Contains(Coordinates))
		{			
			FVector CoordinatesCorrection = FVector(Coordinates.X, Coordinates.Y * (-1), Coordinates.Z);
			if (!ArrayFromObj.Contains(CoordinatesCorrection))
			{
				UE_LOG(LogTemp, Warning, TEXT("Asset coordinates at Indices[%d] = %d, x: %f y: %f z: %f not found on Obj. At AHandsGameMode::AccessMeshVertices()"), i, Indices[i], Coordinates.X, Coordinates.Y, Coordinates.Z);
				return;
			}
			//UE_LOG(LogTemp, Warning, TEXT("Coordinates at Indices[%d] = %d were corrected with Coordinates.Y = Coordinates.Y * (-1)"), i, Indices[i]);
			Index = ArrayFromObj.Find(CoordinatesCorrection);			
			TargetIndicesMapArray.Emplace(Index);
			TargetVerticesArray.Emplace(CoordinatesCorrection);
			contar++;
		}
		else
		{
			Index = ArrayFromObj.Find(Coordinates);
			TargetIndicesMapArray.Emplace(Index);
			TargetVerticesArray.Emplace(Coordinates);
		}

		if (Index == test_index)
		{
			UE_LOG(LogTemp, Warning, TEXT("Asset coordinates at Indices[%d] = %d, x: %f y: %f z: %f"), i, Indices[i], TargetVerticesArray.Last().X, TargetVerticesArray.Last().Y, TargetVerticesArray.Last().Z);
			UE_LOG(LogTemp, Warning, TEXT("Asset normals at Indices[%d] = %d, x: %f y: %f z: %f"), i, Indices[i], TargetNormalsArray.Last().GetSafeNormal().X, TargetNormalsArray.Last().GetSafeNormal().Y, TargetNormalsArray.Last().GetSafeNormal().Z);
			UE_LOG(LogTemp, Warning, TEXT("Mapping of asset index %d is obj index %d"), i, TargetIndicesMapArray.Last());
		}

	}

	UE_LOG(LogTemp, Warning, TEXT("VerticesArray size: %d. Total number of corrections: %d"), TargetVerticesArray.Num(), contar);
}

void AHandsGameMode::Map2ndMeshCorrespondences(TArray<FVector>& ArrayFromAsset, TArray<FVector>& ArrayFromObj, TArray<int32>& MappingAssetToObj)
{
	MappingAssetToObj.Empty();
	MappingAssetToObj.Reserve(ArrayFromAsset.Num());
		
	int32 contador = 0;
	for (int32 i = 0; i < ArrayFromAsset.Num(); i++)
	{
		
		if (!ArrayFromAsset.IsValidIndex(i))
		{				
			UE_LOG(LogTemp, Warning, TEXT("Error while trying to acceess index %d of ArrayFromAsset at AHandsGameMode::MapIndicesFromObjToUE4Asset()"), i);
			return;				
		}
		if (ArrayFromObj.Find(ArrayFromAsset[i]) == INDEX_NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("Asset coordinates at index %d, x: %f y: %f z: %f not found on Obj. At AHandsGameMode::MapIndicesFromObjToUE4Asset()"), i, ArrayFromAsset[i].X, ArrayFromAsset[i].Y, ArrayFromAsset[i].Z);
			contador++;
		}
		int32 index_one = ArrayFromObj.Find(ArrayFromAsset[i]);
		int32 CorrespondenceIndex = (*PtrDenseCorrespondenceIndices)[index_one];
		MappingAssetToObj.Emplace(CorrespondenceIndex);
		/*if (CorrespondenceIndex == 948)
		{
			UE_LOG(LogTemp, Warning, TEXT("Coordinates on Asset[%d] were found on Obj[%d], dense correspondence index is %d"), i, index_one, MappingAssetToObj[i]);
		}*/
	}

	//for (int32 i = 0; i < DenseCorrespondenceIndices.Num(); i++)
	//{
	//	if (DenseCorrespondenceIndices[i] == 948)
	//	{
	//		for (int32 j = 0; j < IndexTest[i].IndicesArray.Num(); j++)
	//		{
	//			UE_LOG(LogTemp, Warning, TEXT("2nd access: Coordinates on Asset[%d] were found on Obj[%d], dense correspondence index is %d"), IndexTest[i].IndicesArray[j], i, DenseCorrespondenceIndices[i]);
	//		}
	//	}
	//}

	UE_LOG(LogTemp, Warning, TEXT("Non-valid indices %d"), contador);		
}

void AHandsGameMode::MappingBetweenMeshes(TArray<FVector>& ArrayFromAsset, TArray<FVector>& ArrayFromObj, TArray<FArrayForStoringIndices>& MappedCorrespondences)
{
	int32 contador = 0;
	for (int32 i = 0; i < ArrayFromAsset.Num(); i++)
	{
		if (!ArrayFromAsset.IsValidIndex(i))
		{
			UE_LOG(LogTemp, Warning, TEXT("Error while trying to acceess index %d of ArrayFromAsset at AHandsGameMode::MappingBetweenMeshes()"), i);
			return;
		}
		if (ArrayFromObj.Find(ArrayFromAsset[i]) == INDEX_NONE)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Asset coordinates at index %d, x: %f y: %f z: %f not found on Obj. At AHandsGameMode::MappingBetweenMeshes()"), i, ArrayFromAsset[i].X, ArrayFromAsset[i].Y, ArrayFromAsset[i].Z);
			ArrayFromAsset[i].Y *= -1;
			contador++;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Coordinates not found %d on 1st pass"), contador);
	

	TArray<FArrayForStoringIndices> IndexTest;
	//TArray<int32> IndexTest;
	IndexTest.Empty();
	for (int32 i = 0; i < ArrayFromObj.Num(); i++)
	{
		FArrayForStoringIndices Empty_array;
		Empty_array.IndicesArray.Empty();
		IndexTest.Add(Empty_array);
	}

	TArray<int32> MappingAssetToObject;
	MappingAssetToObject.Empty();
	MappingAssetToObject.Reserve(ArrayFromAsset.Num());
	contador = 0;
	for (int32 i = 0; i < ArrayFromAsset.Num(); i++)
	{
		if (!ArrayFromAsset.IsValidIndex(i))
		{
			UE_LOG(LogTemp, Warning, TEXT("Error while trying to acceess index %d of ArrayFromAsset at AHandsGameMode::MappingBetweenMeshes()"), i);
			return;
		}
		if (ArrayFromObj.Find(ArrayFromAsset[i]) == INDEX_NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("Asset coordinates at index %d, x: %f y: %f z: %f not found on Obj on 2nd pass. At AHandsGameMode::MappingBetweenMeshes()"), i, ArrayFromAsset[i].X, ArrayFromAsset[i].Y, ArrayFromAsset[i].Z);
			contador++;
		}
		int32 index_one = ArrayFromObj.Find(ArrayFromAsset[i]);
		MappingAssetToObject.Emplace(index_one);
		/*if (index_one == 948)
		{
			UE_LOG(LogTemp, Warning, TEXT("Coordinates on OriginalAsset[%d] were found on OriginalObj[%d]"), i, index_one);
		}*/

		if (IndexTest.IsValidIndex(index_one))
		{
			IndexTest[index_one].IndicesArray.Add(i);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Error while trying to acceess index %d of IndexTest at AHandsGameMode::MapIndicesFromObjToUE4Asset()"), index_one);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Coordinates not found %d on 2nd pass"), contador);	

	MappedCorrespondences.Empty();
	for (int32 i = 0; i < Mapped2ndMeshCorrespondences.Num(); i++)
	{
		FArrayForStoringIndices Empty_array;
		Empty_array.IndicesArray.Empty();
		MappedCorrespondences.Add(Empty_array);
	}

	contador = 0;
	for (int32 i = 0; i < Mapped2ndMeshCorrespondences.Num(); i++)
	{
		if (!Mapped2ndMeshCorrespondences.IsValidIndex(i) || !MappedCorrespondences.IsValidIndex(i))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid index &d at MappedCorrespondences or PtrMapped2ndMeshCorrespondences. At AHandsGameMode::MappingBetweenMeshes()"), i);
			return;
		}
		for (int32 j = 0; j < MappingAssetToObject.Num(); j++)
		{
			if (!MappingAssetToObject.IsValidIndex(j))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index &d on MappingAssetToObject. At AHandsGameMode::MappingBetweenMeshes()"), j);
				return;
			}
			
			int32 CorrespondenceIndex = MappingAssetToObject[j];
			
			/*if (CorrespondenceIndex == 948 && CorrespondenceIndex == Mapped2ndMeshCorrespondences[i])
			{
				UE_LOG(LogTemp, Warning, TEXT("Coordinates on SecondAsset[%d] correspond to OriginalAsset[%d]"), i, j);
				contador++;
			}*/
			
			if (CorrespondenceIndex == Mapped2ndMeshCorrespondences[i])
			{
				MappedCorrespondences[i].IndicesArray.Add(j);
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Contador %d"), contador);
	
	/*for (int32 i = 0; i < Mapped2ndMeshCorrespondences.Num(); i++)
	{
		if (948 == Mapped2ndMeshCorrespondences[i])
		{
			for (int32 j = 0; j < MappedCorrespondences[i].IndicesArray.Num(); j++)
			{
				UE_LOG(LogTemp, Warning, TEXT("Coordinates on SecondAsset[%d] correspond to OriginalAsset[%d]"), i, MappedCorrespondences[i].IndicesArray[j]);
			}
		}
	}*/

}

void AHandsGameMode::MappingTriangles(UStaticMesh* MyMesh, TArray<FVector>& ArrayFromObj, TArray<FVector>& VerticesCoordinates, TArray<FVector>& VerticesNormals, TArray<FVector>& TrianglesIndicesFromObj, TArray<FVector>& TargetTriangleIndicesMap)
{
	TArray<int32> ObjIndices;
	
	TargetTriangleIndicesMap.Empty();
	TArray<FArrayForStoringIndices> Map2ndMeshAssetObjIndices;
	Map2ndMeshAssetObjIndices.Empty();
	for (int32 i = 0; i < ArrayFromObj.Num(); i++)
	{
		FArrayForStoringIndices Empty_array;
		Empty_array.IndicesArray.Empty();
		Map2ndMeshAssetObjIndices.Add(Empty_array);
	}

	int32 NumIndices = VerticesCoordinates.Num();
	for (int32 i = 0; i < NumIndices; i++)
	{
		FVector Coordinates = VerticesCoordinates[i];
		int32 Index;
		if (!ArrayFromObj.Contains(Coordinates))
		{
			float Evaluation = Coordinates.Y * -1;
			Index = ArrayFromObj.Find(FVector(Coordinates.X, Evaluation, Coordinates.Z));
			if (!Map2ndMeshAssetObjIndices.IsValidIndex(Index))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index &d at Map2ndMeshAssetObjIndices. At AHandsGameMode::MappingTriangles()"), Index);
				return;
			}
			Map2ndMeshAssetObjIndices[Index].IndicesArray.Emplace(i);
			ObjIndices.Emplace(Index);
		}
		else
		{
			Index = ArrayFromObj.Find(Coordinates);
			if (!Map2ndMeshAssetObjIndices.IsValidIndex(Index))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index &d at Map2ndMeshAssetObjIndices. At AHandsGameMode::MappingTriangles()"), Index);
				return;
			}
			Map2ndMeshAssetObjIndices[Index].IndicesArray.Emplace(i);
			ObjIndices.Emplace(Index);
		}
	}

	for (int32 i = 0; i < TrianglesIndicesFromObj.Num(); i++)
	{
		int32 FirstIndex = TrianglesIndicesFromObj[i].X - 1;
		int32 SecondIndex = TrianglesIndicesFromObj[i].Y - 1;
		int32 ThirdIndex = TrianglesIndicesFromObj[i].Z - 1;

		TArray<FVector> Normals1stIndex;
		TArray<FVector> Normals2ndIndex;
		TArray<FVector> Normals3rdIndex;

		if (!Map2ndMeshAssetObjIndices.IsValidIndex(FirstIndex))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid FirstIndex &d at Map2ndMeshAssetObjIndices. At AHandsGameMode::MappingTriangles()"), FirstIndex);
			return;
		}
		int32 NumIndicesArray = Map2ndMeshAssetObjIndices[FirstIndex].IndicesArray.Num();
		for (int32 j = 0; j < NumIndicesArray; j++)
		{
			int32 MoreIndices = Map2ndMeshAssetObjIndices[FirstIndex].IndicesArray[j];
			if (!VerticesNormals.IsValidIndex(MoreIndices))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index &d at Normals1stIndex. At AHandsGameMode::MappingTriangles()"), FirstIndex);
				return;
			}
			Normals1stIndex.Emplace(VerticesNormals[MoreIndices]);
		}

		if (!Map2ndMeshAssetObjIndices.IsValidIndex(SecondIndex))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid SecondIndex &d at Map2ndMeshAssetObjIndices. At AHandsGameMode::MappingTriangles()"), FirstIndex);
			return;
		}
		NumIndicesArray = Map2ndMeshAssetObjIndices[SecondIndex].IndicesArray.Num();
		for (int32 j = 0; j < NumIndicesArray; j++)
		{
			int32 MoreIndices = Map2ndMeshAssetObjIndices[SecondIndex].IndicesArray[j];
			if (!VerticesNormals.IsValidIndex(MoreIndices))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index &d at Normals2ndIndex. At AHandsGameMode::MappingTriangles()"), FirstIndex);
				return;
			}
			Normals2ndIndex.Emplace(VerticesNormals[MoreIndices]);
		}

		if (!Map2ndMeshAssetObjIndices.IsValidIndex(FirstIndex))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid ThirdIndex &d at Map2ndMeshAssetObjIndices. At AHandsGameMode::MappingTriangles()"), FirstIndex);
			return;
		}
		NumIndicesArray = Map2ndMeshAssetObjIndices[ThirdIndex].IndicesArray.Num();
		for (int32 j = 0; j < NumIndicesArray; j++)
		{
			int32 MoreIndices = Map2ndMeshAssetObjIndices[ThirdIndex].IndicesArray[j];
			if (!VerticesNormals.IsValidIndex(MoreIndices))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index &d at Normals3rdIndex. At AHandsGameMode::MappingTriangles()"), FirstIndex);
				return;
			}
			Normals3rdIndex.Emplace(VerticesNormals[MoreIndices]);
		}

		FVector StoreIndices;
		for (int32 j = 0; j < Normals1stIndex.Num(); j++)
		{
			for (int32 k = 0; k < Normals2ndIndex.Num(); k++)
			{
				for (int32 l = 0; l < Normals3rdIndex.Num(); l++)
				{
					if (Normals1stIndex[j] == Normals2ndIndex[k] && Normals1stIndex[j] == Normals3rdIndex[l])
					{
						StoreIndices.X = Map2ndMeshAssetObjIndices[FirstIndex].IndicesArray[j];
						StoreIndices.Y = Map2ndMeshAssetObjIndices[SecondIndex].IndicesArray[k];
						StoreIndices.Z = Map2ndMeshAssetObjIndices[ThirdIndex].IndicesArray[l];
					}
				}
			}
		}

		TargetTriangleIndicesMap.Emplace(StoreIndices);
	}
	
	/*int32 test_index = 1426;
	if (TargetTriangleIndicesMap.IsValidIndex(test_index))
	{
		UE_LOG(LogTemp, Warning, TEXT("Triangle %d vertices are 1st: %d 2nd: %d 3rd: %d"), test_index, int32(TargetTriangleIndicesMap[test_index].X), int32(TargetTriangleIndicesMap[test_index].Y), int32(TargetTriangleIndicesMap[test_index].Z));		
	}
	if (VerticesCoordinates.IsValidIndex(int32(TargetTriangleIndicesMap[test_index].X)) && ObjIndices.IsValidIndex(int32(TargetTriangleIndicesMap[test_index].X)))
	{
		UE_LOG(LogTemp, Warning, TEXT("Coordinates at Vertex 1 Triangle %d, x: %f y: %f z: %f. Obj vertex: %d"), test_index, VerticesCoordinates[int32(TargetTriangleIndicesMap[test_index].X)].X, VerticesCoordinates[int32(TargetTriangleIndicesMap[test_index].X)].Y, VerticesCoordinates[int32(TargetTriangleIndicesMap[test_index].X)].Z, ObjIndices[int32(TargetTriangleIndicesMap[test_index].X)]);
	}
	if (VerticesCoordinates.IsValidIndex(int32(TargetTriangleIndicesMap[test_index].Y)) && ObjIndices.IsValidIndex(int32(TargetTriangleIndicesMap[test_index].Y)))
	{
		UE_LOG(LogTemp, Warning, TEXT("Coordinates at Vertex 2 Triangle %d, x: %f y: %f z: %f. Obj vertex: %d"), test_index, VerticesCoordinates[int32(TargetTriangleIndicesMap[test_index].Y)].X, VerticesCoordinates[int32(TargetTriangleIndicesMap[test_index].Y)].Y, VerticesCoordinates[int32(TargetTriangleIndicesMap[test_index].Y)].Z, ObjIndices[int32(TargetTriangleIndicesMap[test_index].Y)]);
	}
	if (VerticesCoordinates.IsValidIndex(int32(TargetTriangleIndicesMap[test_index].Z)) && ObjIndices.IsValidIndex(int32(TargetTriangleIndicesMap[test_index].Z)))
	{
		UE_LOG(LogTemp, Warning, TEXT("Coordinates at Vertex 3 Triangle %d, x: %f y: %f z: %f. Obj vertex: %d"), test_index, VerticesCoordinates[int32(TargetTriangleIndicesMap[test_index].Z)].X, VerticesCoordinates[int32(TargetTriangleIndicesMap[test_index].Z)].Y, VerticesCoordinates[int32(TargetTriangleIndicesMap[test_index].Z)].Z, ObjIndices[int32(TargetTriangleIndicesMap[test_index].Z)]);
	}

	test_index = 1424;
	if (TargetTriangleIndicesMap.IsValidIndex(test_index))
	{
		UE_LOG(LogTemp, Warning, TEXT("Triangle %d vertices are 1st: %d 2nd: %d 3rd: %d"), test_index, int32(TargetTriangleIndicesMap[test_index].X), int32(TargetTriangleIndicesMap[test_index].Y), int32(TargetTriangleIndicesMap[test_index].Z));
	}
	if (VerticesCoordinates.IsValidIndex(int32(TargetTriangleIndicesMap[test_index].X)) && ObjIndices.IsValidIndex(int32(TargetTriangleIndicesMap[test_index].X)))
	{
		UE_LOG(LogTemp, Warning, TEXT("Coordinates at Vertex 1 Triangle %d, x: %f y: %f z: %f. Obj vertex: %d"), test_index, VerticesCoordinates[int32(TargetTriangleIndicesMap[test_index].X)].X, VerticesCoordinates[int32(TargetTriangleIndicesMap[test_index].X)].Y, VerticesCoordinates[int32(TargetTriangleIndicesMap[test_index].X)].Z, ObjIndices[int32(TargetTriangleIndicesMap[test_index].X)]);
	}
	if (VerticesCoordinates.IsValidIndex(int32(TargetTriangleIndicesMap[test_index].Y)) && ObjIndices.IsValidIndex(int32(TargetTriangleIndicesMap[test_index].Y)))
	{
		UE_LOG(LogTemp, Warning, TEXT("Coordinates at Vertex 2 Triangle %d, x: %f y: %f z: %f. Obj vertex: %d"), test_index, VerticesCoordinates[int32(TargetTriangleIndicesMap[test_index].Y)].X, VerticesCoordinates[int32(TargetTriangleIndicesMap[test_index].Y)].Y, VerticesCoordinates[int32(TargetTriangleIndicesMap[test_index].Y)].Z, ObjIndices[int32(TargetTriangleIndicesMap[test_index].Y)]);
	}
	if (VerticesCoordinates.IsValidIndex(int32(TargetTriangleIndicesMap[test_index].Z)) && ObjIndices.IsValidIndex(int32(TargetTriangleIndicesMap[test_index].Z)))
	{
		UE_LOG(LogTemp, Warning, TEXT("Coordinates at Vertex 3 Triangle %d, x: %f y: %f z: %f. Obj vertex: %d"), test_index, VerticesCoordinates[int32(TargetTriangleIndicesMap[test_index].Z)].X, VerticesCoordinates[int32(TargetTriangleIndicesMap[test_index].Z)].Y, VerticesCoordinates[int32(TargetTriangleIndicesMap[test_index].Z)].Z, ObjIndices[int32(TargetTriangleIndicesMap[test_index].Z)]);
	}*/
}

void AHandsGameMode::MappingTriangles(TArray<int32>& Asset2ObjIndicesMap, TArray<FVector>& TrianglesIndicesFromObj, TArray<FVector>& TargetTriangleIndicesMap)
{

	int32 test_index = 248;
	int32 limit = TrianglesIndicesFromObj.Num();
	for (int32 i = 0; i < limit; i++)
	{
		int32 FirstIndex = TrianglesIndicesFromObj[i].X - 1;
		int32 SecondIndex = TrianglesIndicesFromObj[i].Y - 1;
		int32 ThirdIndex = TrianglesIndicesFromObj[i].Z - 1;

		FVector StoreIndices;

		if (!Asset2ObjIndicesMap.IsValidIndex(FirstIndex))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid index %d for Asset2ObjIndices. At AHandsGameMode::MappingTriangles()"), FirstIndex);
			return;
		}
		StoreIndices.X = Asset2ObjIndicesMap.Find(FirstIndex);
		if (i == test_index)
		{
			UE_LOG(LogTemp, Warning, TEXT("Triangle %d first index from obj is %d, which corresponds to asset index %d"), i, FirstIndex, int32(StoreIndices.X));
		}


		if (!Asset2ObjIndicesMap.IsValidIndex(SecondIndex))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid index %d for Asset2ObjIndices. At AHandsGameMode::MappingTriangles()"), SecondIndex);
			return;
		}
		StoreIndices.Y = Asset2ObjIndicesMap.Find(SecondIndex);
		if (i == test_index)
		{
			UE_LOG(LogTemp, Warning, TEXT("Triangle %d second index from obj is %d, which corresponds to asset index %d"), i, SecondIndex, int32(StoreIndices.Y));
		}

		if (!Asset2ObjIndicesMap.IsValidIndex(ThirdIndex))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid index %d for Asset2ObjIndices. At AHandsGameMode::MappingTriangles()"), ThirdIndex);
			return;
		}
		StoreIndices.Z = Asset2ObjIndicesMap.Find(ThirdIndex);
		if (i == test_index)
		{
			UE_LOG(LogTemp, Warning, TEXT("Triangle %d third index from obj is %d, which corresponds to asset index %d"), i, ThirdIndex, int32(StoreIndices.Z));
		}

		TargetTriangleIndicesMap.Emplace(StoreIndices);
	}

}

void AHandsGameMode::OriginalMeshTangentComputation(TArray<FVector>& VerticesArray, TArray<FVector>& NormalsArray, TArray<FVector>& TargetVerticesArray, TArray<FVector>& TargetNormalsArray, TArray<FVector>& TargetTangentsArray, TArray<FVector>& TargetBinormalsArray, TArray<int32>& TargetTangentIndicesArray)
{
	TargetVerticesArray.Empty();
	TargetNormalsArray.Empty();
	TargetTangentsArray.Empty();
	TargetBinormalsArray.Empty();
	TargetTangentIndicesArray.Empty();
	
	TArray<FVector> Coordinates;
	Coordinates.Empty();
	
	int32 contador = 0;
	int32 limit = VerticesArray.Num() / 3;
	TargetVerticesArray.Reserve(limit);
	TargetNormalsArray.Reserve(limit);
	TargetTangentsArray.Reserve(limit);
	TargetBinormalsArray.Reserve(limit);
	TargetTangentIndicesArray.Reserve(limit * 2);
	Coordinates.Reserve(limit);
	for (int32 i = 0; i < limit; i++)
	{
		int32 Index0 = i * 3;
		int32 Index1 = Index0 + 1;
		int32 Index2 = Index0 + 2;

		FVector ProposedTangent;
		FVector NewTangent;
		int32 IndexForNormalsArray = 0;
		bool bNewTangentSet = false;

		if (!VerticesArray.IsValidIndex(Index0) || !VerticesArray.IsValidIndex(Index1) || !VerticesArray.IsValidIndex(Index2))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Index for Vertices Array, Index0: %d Index1: %d Index: %d. At AHandsGameMode::TangentComputation()"), Index0, Index1, Index2);
			return;
		}
		if (!TargetVerticesArray.Contains(VerticesArray[Index0]))
		{
			ProposedTangent = (VerticesArray[Index1] - VerticesArray[Index0]).GetSafeNormal();
			TargetVerticesArray.Emplace(VerticesArray[Index0]);
			IndexForNormalsArray = Index0;
			TargetTangentIndicesArray.Emplace(Index0);
			TargetTangentIndicesArray.Emplace(Index1);
			bNewTangentSet = true;
		}
		else if (!TargetVerticesArray.Contains(VerticesArray[Index1]))
		{
			ProposedTangent = (VerticesArray[Index2] - VerticesArray[Index1]).GetSafeNormal();
			TargetVerticesArray.Emplace(VerticesArray[Index1]);
			IndexForNormalsArray = Index1;
			TargetTangentIndicesArray.Emplace(Index1);
			TargetTangentIndicesArray.Emplace(Index2);
			bNewTangentSet = true;
		}
		else if (!TargetVerticesArray.Contains(VerticesArray[Index2]))
		{
			ProposedTangent = (VerticesArray[Index0] - VerticesArray[Index2]).GetSafeNormal();
			TargetVerticesArray.Emplace(VerticesArray[Index2]);
			IndexForNormalsArray = Index2;
			TargetTangentIndicesArray.Emplace(Index2);
			TargetTangentIndicesArray.Emplace(Index0);
			bNewTangentSet = true;
		}

		if (bNewTangentSet)
		{
			FVector Tangent;
			if (!NormalsArray.IsValidIndex(IndexForNormalsArray))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid &d for NormalsArray. At AHandsGameMode::TangentComputation()"), IndexForNormalsArray);
				return;
			}
			TargetNormalsArray.Emplace(NormalsArray[IndexForNormalsArray].GetSafeNormal());
			FVector SafeNormal = NormalsArray[IndexForNormalsArray].GetSafeNormal();
			bool bIsOrthogonal = FVector::Orthogonal(ProposedTangent, SafeNormal);
			
			if (!bIsOrthogonal)
			{
				FPlane VertexPlane(FVector(0.f,0.f,0.f), SafeNormal);
				Tangent = FVector::PointPlaneProject(NewTangent, VertexPlane);
				bIsOrthogonal = FVector::Orthogonal(Tangent.GetSafeNormal(), SafeNormal);
			}		
			
			FVector Binormal = FVector::CrossProduct(SafeNormal, Tangent);
			/*if (i < 1000 && i >= 900)
			{
						
				if (bIsOrthogonal)
				{
					UE_LOG(LogTemp, Warning, TEXT("Binormal and Normal at index %d are orthogonals"), IndexForNormalsArray);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Binormal and Normals at index %d are not orthogonals"), IndexForNormalsArray);
				}
			}
			contador++;*/
			TargetTangentsArray.Emplace(Tangent.GetSafeNormal());
			TargetBinormalsArray.Emplace(Binormal.GetSafeNormal());

		}
	}
	Coordinates.Empty();
	UE_LOG(LogTemp, Warning, TEXT("Size of Vertices Array: %d. Size of Normals Array: %d. Size of Tangents Array: %d. Size of binormals Array: %d. Size of TangentIndices array: %d"), TargetVerticesArray.Num(), TargetNormalsArray.Num(), TargetTangentsArray.Num(), TargetBinormalsArray.Num(), TargetTangentIndicesArray.Num());
}

void AHandsGameMode::SecondMeshTangentComputation(TArray<FVector>& TargetPointArray, TArray<FVector>& TargetNormalsArray, TArray<FVector>& TargetTangentsArray, TArray<FVector>& TargetBinormalsArray, 
	TArray<int32>& BlendedMapVertexTriangleMap, TArray<FVector>& BarycentricCoordinatesArray, TArray<FVector>& SecondMeshTriangleIndices, TArray<FVector>& VerticesFrom2ndAsset, TArray<FVector>& NormalsFrom2ndAsset, TArray<int32>& OriginalMeshTangentsIndices, 
	TArray<int32>& MeshIndicesMap)
{
	TargetPointArray.Empty();
	TargetNormalsArray.Empty();
	TargetTangentsArray.Empty();
	TargetBinormalsArray.Empty();

	/*BlendedIntrinsicMapsTrianglesMap;
	BlendedIntrinsicMapsBarycentricCoordinates; 
	MapTriangleIndices; 
	SecondMeshVerticesFromUE4Asset;
	SecondMeshNormalsFromUE4Asset;
	OriginalMeshTangentsIndicesMap;
	OriginalMeshAsset2ObjIndicesMap*/

	int32 TestIndex = 0;

	// Run this for each one of the tangents calculated for the 1st mesh
	for (int32 i = 0; i < OriginalMeshTangentsIndices.Num() / 2; i++)
	{
		// Check if we are accessing a valid tangent index
		if (!OriginalMeshTangentsIndices.IsValidIndex(i * 2))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Index %d for MeshTangentsIndices. At AHandsGameMode::SecondMeshTangentComputation()"), i * 2);
			return;
		}
		// Check if said index is valid for the mapped indices of the original mesh
		if (!MeshIndicesMap.IsValidIndex(OriginalMeshTangentsIndices[i * 2]))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Index %d for MeshIndicesMap. At AHandsGameMode::SecondMeshTangentComputation()"), OriginalMeshTangentsIndices[i * 2]);
			return;
		}

		// Index represents a vertex from the original mesh
		int32 Index = MeshIndicesMap[OriginalMeshTangentsIndices[i * 2]];

		// Check if there is a triangle mapped to that vertex
		if (!BlendedMapVertexTriangleMap.IsValidIndex(Index))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Index %d for VertexTriangleMap. At AHandsGameMode::SecondMeshTangentComputation()"), Index);
			return;
		}
		int32 Triangle = BlendedMapVertexTriangleMap[Index];
		
		if (i == TestIndex)
		{
			UE_LOG(LogTemp, Warning, TEXT("Index %d correspponds to Triangle %d"), Index, Triangle);
		}

		// Check if there are barycentric coordinates mapped to that vertex
		if (!BarycentricCoordinatesArray.IsValidIndex(Index))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Index %d for BarycentricCoordinatesArray. At AHandsGameMode::SecondMeshTangentComputation()"), Index);
			return;
		}
		FVector BarycentricCoordinates = BarycentricCoordinatesArray[Index];
		
		if (i == TestIndex)
		{
			UE_LOG(LogTemp, Warning, TEXT("Triangle %d Barycentric coordinates are x: %f y: %f z: %f"), Triangle, BarycentricCoordinates.X, BarycentricCoordinates.Y, BarycentricCoordinates.Z);
		}

		// Check if the triangle is found 
		if (!SecondMeshTriangleIndices.IsValidIndex(Triangle))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Index %d for MeshTriangleIndices. At AHandsGameMode::SecondMeshTangentComputation()"), Triangle);
			return;
		}
		// Get the triangle vertices
		FVector TriangleVertices = SecondMeshTriangleIndices[Triangle];
		int32 Vertex1 = TriangleVertices.X;
		int32 Vertex2 = TriangleVertices.Y;
		int32 Vertex3 = TriangleVertices.Z;

		// Get coordinates for the triangle vertices
		if (!VerticesFrom2ndAsset.IsValidIndex(Vertex1) || !NormalsFrom2ndAsset.IsValidIndex(Vertex1))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Index Vertex1 %d for MeshVertices or MeshNormals. At AHandsGameMode::SecondMeshTangentComputation()"), Vertex1);
			return;
		}
		FVector Vertex1Coordinates = VerticesFrom2ndAsset[Vertex1];
		FVector Vertex1Normals = NormalsFrom2ndAsset[Vertex1];
		if (i == TestIndex)
		{
			UE_LOG(LogTemp, Warning, TEXT("Vertex1Normals x: %f y: %f z: %f"), Vertex1Normals.X, Vertex1Normals.Y, Vertex1Normals.Z);
		}

		if (!VerticesFrom2ndAsset.IsValidIndex(Vertex2) || !NormalsFrom2ndAsset.IsValidIndex(Vertex2))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Index Vertex2 %d for MeshVertices or MeshNormals. At AHandsGameMode::SecondMeshTangentComputation()"), Vertex2);
			return;
		}
		FVector Vertex2Coordinates = VerticesFrom2ndAsset[Vertex2];
		FVector Vertex2Normals = NormalsFrom2ndAsset[Vertex2];
		if (i == TestIndex)
		{
			UE_LOG(LogTemp, Warning, TEXT("Vertex2Normals x: %f y: %f z: %f"), Vertex2Normals.X, Vertex2Normals.Y, Vertex2Normals.Z);
		}

		if (!VerticesFrom2ndAsset.IsValidIndex(Vertex3) || !NormalsFrom2ndAsset.IsValidIndex(Vertex3))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Index Vertex3 %d for MeshVertices. At AHandsGameMode::SecondMeshTangentComputation()"), Vertex3);
			return;
		}
		FVector Vertex3Coordinates = VerticesFrom2ndAsset[Vertex3];
		FVector Vertex3Normals = NormalsFrom2ndAsset[Vertex3];
		if (i == TestIndex)
		{
			UE_LOG(LogTemp, Warning, TEXT("Vertex3Normals x: %f y: %f z: %f"), Vertex3Normals.X, Vertex3Normals.Y, Vertex3Normals.Z);
		}

		FVector PointInTriangle = BarycentricCoordinates.X * Vertex1Coordinates + BarycentricCoordinates.Y * Vertex2Coordinates + BarycentricCoordinates.Z * Vertex2Coordinates;
		
		/*if (i == TestIndex)
		{
			UE_LOG(LogTemp, Warning, TEXT("PointinTriangle x: %f y: %f z: %f"), PointInTriangle.X, PointInTriangle.Y, PointInTriangle.Z);
		}*/

		TargetPointArray.Emplace(PointInTriangle);

		// The normal of the blended map point is the average of the triangle vertices normals
		FVector NewNormal = (1.f / 3.f) * (Vertex1Normals + Vertex2Normals + Vertex3Normals);

		if (i == TestIndex)
		{
			UE_LOG(LogTemp, Warning, TEXT("NewNormal x: %f y: %f z: %f"), NewNormal.X, NewNormal.Y, NewNormal.Z);
		}

		TargetNormalsArray.Emplace(NewNormal.GetSafeNormal());

		FPlane PlaneForTangentComputation(FVector(0.f, 0.f, 0.f), NewNormal.GetSafeNormal());

		if (!OriginalMeshTangentsIndices.IsValidIndex((i * 2) + 1))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Index %d for MeshTangentsIndices. At AHandsGameMode::SecondMeshTangentComputation()"), (i * 2) + 1);
			return;
		}
		// Check if said index is valid for the mapped indices of the original mesh
		if (!MeshIndicesMap.IsValidIndex(OriginalMeshTangentsIndices[(i * 2) + 1]))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Index %d for MeshIndicesMap. At AHandsGameMode::SecondMeshTangentComputation()"), OriginalMeshTangentsIndices[(i * 2) + 1]);
			return;
		}

		// Index represents a vertex from the original mesh
		int32 IndexForTangentComputation = MeshIndicesMap[OriginalMeshTangentsIndices[(i * 2) + 1]];

		// Check if there is a triangle mapped to that vertex
		if (!BlendedMapVertexTriangleMap.IsValidIndex(IndexForTangentComputation))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Index %d for VertexTriangleMap. At AHandsGameMode::SecondMeshTangentComputation()"), IndexForTangentComputation);
			return;
		}
		Triangle = BlendedMapVertexTriangleMap[IndexForTangentComputation];

		/*if (i == TestIndex)
		{
			UE_LOG(LogTemp, Warning, TEXT("Index %d correspponds to Triangle %d"), IndexForTangentComputation, Triangle);
		}*/

		// Check if there are barycentric coordinates mapped to that vertex
		if (!BarycentricCoordinatesArray.IsValidIndex(IndexForTangentComputation))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Index %d for BarycentricCoordinatesArray. At AHandsGameMode::SecondMeshTangentComputation()"), IndexForTangentComputation);
			return;
		}
		BarycentricCoordinates = BarycentricCoordinatesArray[IndexForTangentComputation];

		/*if (i == TestIndex)
		{
			UE_LOG(LogTemp, Warning, TEXT("Triangle %d Barycentric coordinates are x: %f y: %f z: %f"), Triangle, BarycentricCoordinates.X, BarycentricCoordinates.Y, BarycentricCoordinates.Z);
		}*/

		// Check if the triangle is found 
		if (!SecondMeshTriangleIndices.IsValidIndex(Triangle))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Index %d for MeshTriangleIndices. At AHandsGameMode::SecondMeshTangentComputation()"), Triangle);
			return;
		}
		// Get the triangle vertices
		TriangleVertices = SecondMeshTriangleIndices[Triangle];
		Vertex1 = TriangleVertices.X;
		Vertex2 = TriangleVertices.Y;
		Vertex3 = TriangleVertices.Z;

		// Get coordinates for the triangle vertices
		if (!VerticesFrom2ndAsset.IsValidIndex(Vertex1))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Index Vertex1 %d for MeshVertices. At AHandsGameMode::SecondMeshTangentComputation()"), Vertex1);
			return;
		}
		Vertex1Coordinates = VerticesFrom2ndAsset[Vertex1];

		if (!VerticesFrom2ndAsset.IsValidIndex(Vertex2))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Index Vertex2 %d for MeshVertices. At AHandsGameMode::SecondMeshTangentComputation()"), Vertex2);
			return;
		}
		Vertex2Coordinates = VerticesFrom2ndAsset[Vertex2];

		if (!VerticesFrom2ndAsset.IsValidIndex(Vertex3))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Index Vertex3 %d for MeshVertices. At AHandsGameMode::SecondMeshTangentComputation()"), Vertex3);
			return;
		}
		Vertex3Coordinates = VerticesFrom2ndAsset[Vertex3];

		FVector PointIn2ndTriangle = BarycentricCoordinates.X * Vertex1Coordinates + BarycentricCoordinates.Y * Vertex2Coordinates + BarycentricCoordinates.Z * Vertex2Coordinates;

		/*if (i == TestIndex)
		{
			UE_LOG(LogTemp, Warning, TEXT("PointinTriangle x: %f y: %f z: %f"), PointIn2ndTriangle.X, PointIn2ndTriangle.Y, PointIn2ndTriangle.Z);
		}*/

		FVector ProposedTangent = (PointIn2ndTriangle - PointInTriangle).GetSafeNormal();
		FVector NewTangent = FVector::PointPlaneProject(ProposedTangent, PlaneForTangentComputation);

		if (i == TestIndex)
		{
		UE_LOG(LogTemp, Warning, TEXT("NewTangent x: %f y: %f z: %f"), NewTangent.X, NewTangent.Y, NewTangent.Z);
		}

		FVector NewBinormal = FVector::CrossProduct(NewNormal, NewTangent);

		if (i == TestIndex)
		{
		UE_LOG(LogTemp, Warning, TEXT("NewBinormal x: %f y: %f z: %f"), NewBinormal.X, NewBinormal.Y, NewBinormal.Z);
		}

		TargetTangentsArray.Emplace(NewTangent.GetSafeNormal());
		TargetBinormalsArray.Emplace(NewBinormal.GetSafeNormal());
	}
	UE_LOG(LogTemp, Warning, TEXT("Size of Points Array: %d. Size of Normals array: %d. Size of Tangents array: %d. Size of Binormals array: %d"), TargetPointArray.Num(), TargetNormalsArray.Num(), TargetTangentsArray.Num(), TargetBinormalsArray.Num());

}