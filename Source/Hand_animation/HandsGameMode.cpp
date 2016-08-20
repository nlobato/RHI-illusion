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
				MyCharacter->CurrentMeshIdentificator = 1;
				MyCharacter->bHasObjectMeshChanged = true;
				bIsOriginalMesh = false;
			}
			else
			{
				MyCharacter->SpawnedObject->OurVisibleComponent->SetStaticMesh(OriginalMesh);
				MyCharacter->bAreDPset = false;
				MyCharacter->CurrentMeshIdentificator = 2;
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
			UE_LOG(LogTemp, Warning, TEXT("Could not find file at AHandsGameMode::ReadTextFile(TargetCoordinatessArray)"));
		}		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find directory at AHandsGameMode::ReadTextFile(TargetCoordinatessArray)"));
	}

	TargetCoordinatesArray.Empty();
	TargetCoordinatesArray.Reserve(ArrayForTextFile.Num());
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
				/*switch (WhitespaceCounter)
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
				}*/
			}
		}
		ComponentZ = FCString::Atof(*StringVector);
		TargetCoordinatesArray.Emplace(FVector(ComponentX, ComponentY, ComponentZ));
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
	UE_LOG(LogTemp, Warning, TEXT("We are in"));
	AHands_Character* MyCharacter = Cast<AHands_Character>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter)
	{
		//PtrDenseCorrespondenceIndices = &MyCharacter->DenseCorrespondenceIndices;
		//PtrDenseCorrespondenceCoordinates = &MyCharacter->DenseCorrespondenceCoordinates;
		//PtrOriginalMeshVerticesCoordinatesFromObjFile = &MyCharacter->OriginalMeshVerticesCoordinatesFromObjFile;
		//PtrSecondMeshVerticesCoordinatesFromObjFile = &MyCharacter->SecondMeshVerticesCoordinatesFromObjFile;

		//PtrMapped1stMeshCorrespondences = &MyCharacter->Mapped1stMeshCorrespondences;
		//PtrMapped2ndMeshCorrespondences = &MyCharacter->Mapped2ndMeshCorrespondences;
		PtrMesh2Mesh1Correspondences = &MyCharacter->Mesh2Mesh1Correspondences;

		PtrOriginalMeshVerticesCoordinatesFromUE4Asset = &MyCharacter->OriginalMeshVerticesCoordinatesFromUE4Asset;
		PtrOriginalMeshVerticesNormalsFromUE4Asset = &MyCharacter->OriginalMeshVerticesNormalsFromUE4Asset;
		PtrOriginalMeshVerticesTangentsFromUE4Asset = &MyCharacter->OriginalMeshVerticesTangentsFromUE4Asset;
		PtrOriginalMeshVerticesBinormalsFromUE4Asset = &MyCharacter->OriginalMeshVerticesBinormalsFromUE4Asset;

		PtrSecondMeshVerticesCoordinatesFromUE4Asset = &MyCharacter->SecondMeshVerticesCoordinatesFromUE4Asset;
		PtrSecondMeshVerticesNormalsFromUE4Asset = &MyCharacter->SecondMeshVerticesNormalsFromUE4Asset;
		PtrSecondMeshVerticesTangentsFromUE4Asset = &MyCharacter->SecondMeshVerticesTangentsFromUE4Asset;
		PtrSecondMeshVerticesBinormalsFromUE4Asset = &MyCharacter->SecondMeshVerticesBinormalsFromUE4Asset;

		UE_LOG(LogTemp, Warning, TEXT("Pointers created succesfully"));

		// Read the text file with the dense correspondence indices
		FString DenseCorrespondaceIndicesFilePath = LoadDirectory + "/" + DenseCorrespondanceIndicesFileName;
		ReadTextFile(DenseCorrespondaceIndicesFilePath, DenseCorrespondenceIndices);

		UE_LOG(LogTemp, Warning, TEXT("First Dense Correspondence file read succesfully"));

		//Read the text file with the dense correspondence coordinates
		FString VerticesCorrespondaceFilePath = LoadDirectory + "/" + DenseCorrespondanceVerticesFileName;
		ReadTextFile(VerticesCorrespondaceFilePath, DenseCorrespondenceCoordinates);

		UE_LOG(LogTemp, Warning, TEXT("Second Dense Correspondence file read succesfully"));
		
		// Read the text file with the OBJ coordinates for the first mesh
		FString OriginalMeshFilePath = LoadDirectory + "/" + OriginalMeshVerticesCoordinatesFromObjFileName;
		ReadTextFile(OriginalMeshFilePath, OriginalMeshVerticesCoordinatesFromObjFile);

		UE_LOG(LogTemp, Warning, TEXT("1st obj text file read succesfully"));

		// Read the text file with the OBJ coordinates for the first mesh
		FString SecondMeshFilePath = LoadDirectory + "/" + SecondMeshVerticesCoordinatesFromObjFileName;
		ReadTextFile(SecondMeshFilePath, SecondMeshVerticesCoordinatesFromObjFile);

		UE_LOG(LogTemp, Warning, TEXT("2nd obj text file read succesfully"));

		AInteractionObject* InteractionObjectForMeshChange = MyCharacter->ObjectToSpawn4.GetDefaultObject();
		UStaticMesh* OneMesh = MyCharacter->ObjectToSpawn4->GetDefaultObject<AInteractionObject>()->OurVisibleComponent->StaticMesh;
		//PointerToObjectSpawnedByCharacter = &MyCharacter->ObjectToSpawn4;
		//if (PointerToObjectSpawnedByCharacter)
		if (InteractionObjectForMeshChange)
		{
			AccessMeshVertices(OneMesh, *PtrOriginalMeshVerticesCoordinatesFromUE4Asset, *PtrOriginalMeshVerticesNormalsFromUE4Asset, *PtrOriginalMeshVerticesTangentsFromUE4Asset, *PtrOriginalMeshVerticesBinormalsFromUE4Asset);
			
			UE_LOG(LogTemp, Warning, TEXT("Succesfully accesed the vertices of Original mesh"));			
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("InteractionObjectForMeshChange is invalid. Located at AHandsGameMode::InitializeArrays()"));
		}

		if (SecondMesh != nullptr || SecondMesh->RenderData != nullptr)
		{
			AccessMeshVertices(SecondMesh, *PtrSecondMeshVerticesCoordinatesFromUE4Asset, *PtrSecondMeshVerticesNormalsFromUE4Asset, *PtrSecondMeshVerticesTangentsFromUE4Asset, *PtrSecondMeshVerticesBinormalsFromUE4Asset);
			UE_LOG(LogTemp, Warning, TEXT("Succesfully accesed the vertices of second mesh"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SecondMesh or SecondMesh->RenderData are invalid. Located at AHandsGameMode::InitializeArrays()"));
		}

		Map2ndMeshCorrespondences(*PtrSecondMeshVerticesCoordinatesFromUE4Asset, SecondMeshVerticesCoordinatesFromObjFile, Mapped2ndMeshCorrespondences);
		
		UE_LOG(LogTemp, Warning, TEXT("Succesfully mapped the correspondances of second mesh"));

		MappingBetweenMeshes(*PtrOriginalMeshVerticesCoordinatesFromUE4Asset, OriginalMeshVerticesCoordinatesFromObjFile, *PtrMesh2Mesh1Correspondences);
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
	uint32 NumIndices = Indices.Num();
	TargetArray.Reserve(NumIndices);
	NormalsArray.Reserve(NumIndices);
	TangentsArray.Reserve(NumIndices);
	BinormalsArray.Reserve(NumIndices);
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
		int32 CorrespondenceIndex = DenseCorrespondenceIndices[index_one];
		MappingAssetToObj.Emplace(CorrespondenceIndex);
		/*if (CorrespondenceIndex == 948)
		{
			UE_LOG(LogTemp, Warning, TEXT("Coordinates on Asset[%d] were found on Obj[%d], dense correspondence index is %d"), i, index_one, CorrespondenceIndex);
		}*/
	}
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

