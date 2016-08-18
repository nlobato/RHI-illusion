// Fill out your copyright notice in the Description page of Project Settings.

#include "Hand_animation.h"
#include "Hands_Character.h"
#include "Kismet/GameplayStatics.h"
#include "CalibrationBox.h"
#include "InteractionObject.h"
#include "Public/StaticMeshResources.h"
#include "Public/PackedNormal.h"
#include "Public/GenericPlatform/GenericPlatformMath.h"
#include "HandsGameMode.h"


// Sets default values
AHands_Character::AHands_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set this pawn to be controlled by the lowest-numbered player
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(34.f, 88.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	
	//bAreDPsActive = true;

	// Left hand sensors offset
	LeftHandSensorOffset = FVector(-10.581812, -2.98531, 0.622225);
	LeftIndexFingerSensorOffset = FVector(-1.561717, -1.282756, 0.064839);
	LeftMiddleFingerSensorOffset = FVector(-2.739685, -0.870843, 0.165751);
	LeftRingFingerSensorOffset = FVector(-1.81612, -1.565517, 0.065366);
	LeftPinkyFingerSensorOffset = FVector(-1.296994, -1.144543, 0.005556);
	LeftThumbSensorOffset = FVector(-2.08541, -1.472764, -0.654292);

	LeftMiddleKnuckleSensorOffset = FVector(2.252283, -1.617367, -0.055583);
	
	//LeftHandSensorOffset = FVector(0, -3.5, 0);
	RightHandSensorOffset = FVector(10.581812, 2.98531, -0.622225);

	RightIndexFingerSensorOffset = FVector(1.561717, 1.282756, -0.064839);
	RightMiddleFingerSensorOffset = FVector(2.739685, 0.870843, -0.165751);
	RightRingFingerSensorOffset = FVector(1.81612, 1.565517, -0.065366);
	RightPinkyFingerSensorOffset = FVector(1.296994, 1.144543, -0.005556);
	RightThumbSensorOffset = FVector(2.08541, 1.472764, 0.654292);

	RightMiddleKnuckleSensorOffset = FVector(-2.252283, 1.617367, 0.055583);

	SensorDelayRangeHigh = 0.5;

	SensorDelayRangeLow = 0.f;
	
	MyMesh = GetMesh();
}

// Called when the game starts or when spawned
void AHands_Character::BeginPlay()
{
	Super::BeginPlay();

	bIsSystemCalibrated = false;
	AxisTranslation = FVector(0.f, 0.f, 0.f);
	AlphaValue = 0.f;
	bIsObject1Spawned = false;
	bIsObject2Spawned = false;
	bHasObjectSizeChanged = false;
	bHasObjectMeshChanged = false;
	bAreDPset = false;
	SensorDelayTotalTime = 0;
	bIsDelayCompleted = false;
	bIsDecisionMade = false;
}

// Called every frame
void AHands_Character::Tick( float DeltaTime )
{
	Super::Tick(DeltaTime);

	{
		/*GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("One vertex index: %d"), DenseCorrespondenceIndices[123]));
		GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("Array size: %d"), DenseCorrespondenceIndices.Num()));*/
		// If a virtual object is active, use DP algorithm to calculate the fingers position
		if (SpawnedObject && (bHasObjectSizeChanged || bHasObjectMeshChanged))
		{
			// Empty the array where the descriptor points will be stored
			DPVirtualObject.Empty();

			// Access the vertices from hte object's mesh, stored them in our TArray
			AHands_Character::AccessTriVertices(SpawnedObject->OurVisibleComponent, DPVirtualObject);				
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("DPVirtual Object Num: %d"), DPVirtualObject.Num()));

			AHands_Character::GetMeshCurrentTransform(SpawnedObject->OurVisibleComponent, CurrentMeshLocalToWorldMatrix, CurrentMeshComponentToWorldTransform, CurrentVerticesNum);
			AHands_Character::AssignPointers();
			//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Red, FString::Printf(TEXT("Original Vert Coord x: %f y: %f z: %f"), OriginalMeshVerticesCoordinatesFromUE4Asset[0].X, OriginalMeshVerticesCoordinatesFromUE4Asset[0].Y, OriginalMeshVerticesCoordinatesFromUE4Asset[0].Z));
			//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Red, FString::Printf(TEXT("Original Vert Coord x: %f y: %f z: %f"), (*PointerToCurrentMeshVertices)[0].X, (*PointerToCurrentMeshVertices)[0].Y, (*PointerToCurrentMeshVertices)[0].Z));
			// Draw the position, normal, tangent and binormal of our DPs
			if (bDrawPoints)
			{				
				AHands_Character::DrawDescriptionPoints(DPVirtualObject);
			}
			
			// Time delay for the asynchronous condition is now handled in the AnimBlueprint

			if (bAreDPsActive)
			{
												
				LeftHandWeights.Empty();
				LeftHandTransformation.Empty();
				AHands_Character::WeightsComputation(LeftHandWeights, LeftHandTransformation, LeftHandPosition);
				//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("Weights old method: %f"), LeftHandWeights[795]));
				//TArray<float> TestWeights;
				//AHands_Character::WeightsComputation(LeftHandPosition, LeftHandTransformationArray, TestWeights);
				//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Blue, FString::Printf(TEXT("Weights new method: %f"), TestWeights[795]));
				DPLeftHandPosition = AHands_Character::NewJointPosition(LeftHandWeights, LeftHandTransformation, DPVirtualObject);
				//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("New joint pos  x: %f y: %f z: %f"), DPLeftHandPosition.X, DPLeftHandPosition.Y, DPLeftHandPosition.Z));
				//FVector TestLeftHandPosition = AHands_Character::NewJointPosition(TestWeights, LeftHandTransformationArray);
				//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Blue, FString::Printf(TEXT("New joint pos  x: %f y: %f z: %f"), TestLeftHandPosition.X, TestLeftHandPosition.Y, TestLeftHandPosition.Z));
				
				LeftMiddleKnuckleWeights.Empty();
				LeftMiddleKnuckleTransformation.Empty();
				AHands_Character::WeightsComputation(LeftMiddleKnuckleWeights, LeftMiddleKnuckleTransformation, LeftMiddleKnucklePosition);
				DPLeftMiddleKnucklePosition = AHands_Character::NewJointPosition(LeftMiddleKnuckleWeights, LeftMiddleKnuckleTransformation, DPVirtualObject);

				LeftIndexFingerWeights.Empty();
				LeftIndexFingerTransformation.Empty();
				AHands_Character::WeightsComputation(LeftIndexFingerWeights, LeftIndexFingerTransformation, LeftIndexFingerPosition);
				DPLeftIndexFingerPosition = AHands_Character::NewJointPosition(LeftIndexFingerWeights, LeftIndexFingerTransformation, DPVirtualObject);

				LeftMiddleFingerWeights.Empty();
				LeftMiddleFingerTransformation.Empty();
				AHands_Character::WeightsComputation(LeftMiddleFingerWeights, LeftMiddleFingerTransformation, LeftMiddleFingerPosition);
				DPLeftMiddleFingerPosition = AHands_Character::NewJointPosition(LeftMiddleFingerWeights, LeftMiddleFingerTransformation, DPVirtualObject);

				LeftRingFingerWeights.Empty();
				LeftRingFingerTransformation.Empty();
				AHands_Character::WeightsComputation(LeftRingFingerWeights, LeftRingFingerTransformation, LeftRingFingerPosition);
				DPLeftRingFingerPosition = AHands_Character::NewJointPosition(LeftRingFingerWeights, LeftRingFingerTransformation, DPVirtualObject);

				LeftPinkyFingerWeights.Empty();
				LeftPinkyFingerTransformation.Empty();
				AHands_Character::WeightsComputation(LeftPinkyFingerWeights, LeftPinkyFingerTransformation, LeftPinkyFingerPosition);
				DPLeftPinkyFingerPosition = AHands_Character::NewJointPosition(LeftPinkyFingerWeights, LeftPinkyFingerTransformation, DPVirtualObject);

				LeftThumbWeights.Empty();
				LeftThumbTransformation.Empty();
				AHands_Character::WeightsComputation(LeftThumbWeights, LeftThumbTransformation, LeftThumbPosition);
				DPLeftThumbPosition = AHands_Character::NewJointPosition(LeftThumbWeights, LeftThumbTransformation, DPVirtualObject);

				RightHandWeights.Empty();
				RightHandTransformation.Empty();
				AHands_Character::WeightsComputation(RightHandWeights, RightHandTransformation, RightHandPosition);
				DPRightHandPosition = AHands_Character::NewJointPosition(RightHandWeights, RightHandTransformation, DPVirtualObject);

				RightMiddleKnuckleWeights.Empty();
				RightMiddleKnuckleTransformation.Empty();
				AHands_Character::WeightsComputation(RightMiddleKnuckleWeights, RightMiddleKnuckleTransformation, RightMiddleKnucklePosition);
				DPRightMiddleKnucklePosition = AHands_Character::NewJointPosition(RightMiddleKnuckleWeights, RightMiddleKnuckleTransformation, DPVirtualObject);

				RightIndexFingerWeights.Empty();
				RightIndexFingerTransformation.Empty();
				AHands_Character::WeightsComputation(RightIndexFingerWeights, RightIndexFingerTransformation, RightIndexFingerPosition);
				DPRightIndexFingerPosition = AHands_Character::NewJointPosition(RightIndexFingerWeights, RightIndexFingerTransformation, DPVirtualObject);

				RightMiddleFingerWeights.Empty();
				RightMiddleFingerTransformation.Empty();
				AHands_Character::WeightsComputation(RightMiddleFingerWeights, RightMiddleFingerTransformation, RightMiddleFingerPosition);
				DPRightMiddleFingerPosition = AHands_Character::NewJointPosition(RightMiddleFingerWeights, RightMiddleFingerTransformation, DPVirtualObject);

				RightRingFingerWeights.Empty();
				RightRingFingerTransformation.Empty();
				AHands_Character::WeightsComputation(RightRingFingerWeights, RightRingFingerTransformation, RightRingFingerPosition);
				DPRightRingFingerPosition = AHands_Character::NewJointPosition(RightRingFingerWeights, RightRingFingerTransformation, DPVirtualObject);

				RightPinkyFingerWeights.Empty();
				RightPinkyFingerTransformation.Empty();
				AHands_Character::WeightsComputation(RightPinkyFingerWeights, RightPinkyFingerTransformation, RightPinkyFingerPosition);
				DPRightPinkyFingerPosition = AHands_Character::NewJointPosition(RightPinkyFingerWeights, RightPinkyFingerTransformation, DPVirtualObject);

				RightThumbWeights.Empty();
				RightThumbTransformation.Empty();
				AHands_Character::WeightsComputation(RightThumbWeights, RightThumbTransformation, RightThumbPosition);
				DPRightThumbPosition = AHands_Character::NewJointPosition(RightThumbWeights, RightThumbTransformation, DPVirtualObject);
			}


			// For debug purposes, draw the calculated joint positions
			if (bDrawRightHandPoints)
			{
				DrawDebugSphere(GetWorld(), DPRightHandPosition, 5.0, 50, FColor(0, 255, 0), false, 0.05);
				DrawDebugSphere(GetWorld(), DPRightMiddleKnucklePosition, 2.0, 50, FColor(0, 255, 0), false, 0.05);
				DrawDebugPoint(GetWorld(), DPRightIndexFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
				DrawDebugPoint(GetWorld(), DPRightMiddleFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
				DrawDebugPoint(GetWorld(), DPRightRingFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
				DrawDebugPoint(GetWorld(), DPRightPinkyFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
				DrawDebugPoint(GetWorld(), DPRightThumbPosition, 5.0, FColor(0, 255, 0), false, 0.05);
			}

			if (bDrawLeftHandPoints)
			{
				DrawDebugSphere(GetWorld(), DPLeftHandPosition, 5.0, 50, FColor(0, 255, 0), false, 0.05);
				DrawDebugSphere(GetWorld(), DPLeftMiddleKnucklePosition, 2.0, 50, FColor(0, 255, 0), false, 0.05);
				DrawDebugPoint(GetWorld(), DPLeftIndexFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
				DrawDebugPoint(GetWorld(), DPLeftMiddleFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
				DrawDebugPoint(GetWorld(), DPLeftRingFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
				DrawDebugPoint(GetWorld(), DPLeftPinkyFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
				DrawDebugPoint(GetWorld(), DPLeftThumbPosition, 5.0, FColor(0, 255, 0), false, 0.05);
			}

		}
	}

}

// Called to bind functionality to input
void AHands_Character::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	// Right hand & fingers movement binding
	InputComponent->BindAxis("Right_Hand_X", this, &AHands_Character::RightHandMovement);
	InputComponent->BindAxis("Right_Index_X", this, &AHands_Character::RightIndexFingerMovement);
	InputComponent->BindAxis("Right_Middle_X", this, &AHands_Character::RightMiddleFingerMovement);
	InputComponent->BindAxis("Right_Ring_X", this, &AHands_Character::RightRingFingerMovement);
	InputComponent->BindAxis("Right_Pinky_X", this, &AHands_Character::RightPinkyFingerMovement);
	InputComponent->BindAxis("Right_Thumb_X", this, &AHands_Character::RightThumbMovement);
	
	// Left hand & fingers movement binding
	InputComponent->BindAxis("Left_Hand_X", this, &AHands_Character::LeftHandMovement);
	InputComponent->BindAxis("Left_Index_X", this, &AHands_Character::LeftIndexFingerMovement);
	InputComponent->BindAxis("Left_Middle_X", this, &AHands_Character::LeftMiddleFingerMovement);
	InputComponent->BindAxis("Left_Ring_X", this, &AHands_Character::LeftRingFingerMovement);
	InputComponent->BindAxis("Left_Pinky_X", this, &AHands_Character::LeftPinkyFingerMovement);
	InputComponent->BindAxis("Left_Thumb_X", this, &AHands_Character::LeftThumbMovement);

	// Character movement
	InputComponent->BindAxis("MoveForward", this, &AHands_Character::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AHands_Character::MoveRight);
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	// Object spawning
	InputComponent->BindAction("SpawnObject1", IE_Pressed, this, &AHands_Character::Answer1);
	InputComponent->BindAction("SpawnObject2", IE_Pressed, this, &AHands_Character::Answer2);
	InputComponent->BindAction("SpawnObject3", IE_Pressed, this, &AHands_Character::Answer3);
	InputComponent->BindAction("SpawnObject4", IE_Pressed, this, &AHands_Character::Answer4);

	//Object movement and modification
	InputComponent->BindAxis("ObjectSensor1", this, &AHands_Character::Object1Movement);
	InputComponent->BindAxis("ObjectSensor2", this, &AHands_Character::Object2Movement);
	InputComponent->BindAxis("ObjectSensor3", this, &AHands_Character::Object3Movement);
	InputComponent->BindAxis("ObjectSensor4", this, &AHands_Character::Object4Movement);

	//Object modification
	InputComponent->BindAction("ChangeObjectSize", IE_Pressed, this, &AHands_Character::ModifyObjectSize);
	InputComponent->BindAction("ResetObjectSize", IE_Pressed, this, &AHands_Character::ResetObjectSize);

}

void AHands_Character::CalibrateSystem(FVector AxisTranslationFromGameMode)
{
	AlphaValue = 1.f;
	AxisTranslation = AxisTranslationFromGameMode;
	bIsSystemCalibrated = true;
}

void AHands_Character::SetAlphaValue(float AlphaValueFromGM)
{
	AlphaValue = AlphaValueFromGM;
}

void AHands_Character::ExperimentSetup(bool bIsSync, bool IsDP)
{
	bIsDelayActive = !bIsSync;
	bAreDPsActive = IsDP;
}

void AHands_Character::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AHands_Character::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AHands_Character::RightHandMovement(float ValueX)
{
	FRotator RawOrientation;
	RawOrientation.Roll = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_1RotationRoll");
	RawOrientation.Pitch = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_1RotationPitch");
	RawOrientation.Yaw = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_1RotationYaw");
	RightHandOrientation = RectifyRightHandOrientation(RawOrientation);
		
	FVector RawPosition;
	RawPosition.X = ValueX;
	RawPosition.Y = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_1MotionY");
	RawPosition.Z = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_1MotionZ");
	FVector RectifiedRightHandPosition = RectifyHandPosition(RawPosition);
	RightHandPosition = RectifiedRightHandPosition;
	if (bIsSystemCalibrated)
	{
		//RightHandPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightHandPosition, FVector(0.f,0.f,0.f), RightHandOrientation));
		//DrawDebugPoint(GetWorld(), RightHandPosition, 5.0, FColor(0, 255, 0), false, 0.05);
		RightHandPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightHandPosition, RightHandSensorOffset, RightHandOrientation));
		//DrawDebugPoint(GetWorld(), RightHandPosition, 5.0, FColor(255, 0, 0), false, 0.05);
	}

	RightMiddleKnucklePosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightHandPosition, RightMiddleKnuckleSensorOffset, RightHandOrientation));
}

void AHands_Character::RightIndexFingerMovement(float ValueX)
{
	FRotator RawOrientation;
	RawOrientation.Roll = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_2RotationRoll");
	RawOrientation.Pitch = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_2RotationPitch");
	RawOrientation.Yaw = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_2RotationYaw");
	RightIndexFingerOrientation = RectifyRightHandOrientation(RawOrientation);
	
	FVector RawPosition;
	RawPosition.X = ValueX;
	RawPosition.Y = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_2MotionY");
	RawPosition.Z = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_2MotionZ");
	FVector RectifiedRightIndexFingerPosition = RectifyHandPosition(RawPosition);
	RightIndexFingerPosition = RectifiedRightIndexFingerPosition;	
	if (bIsSystemCalibrated)
	{
		//RightIndexFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightIndexFingerPosition, FVector(0.f,0.f,0.f), RightIndexFingerOrientation));
		//DrawDebugPoint(GetWorld(), RightIndexFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
		RightIndexFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightIndexFingerPosition, RightIndexFingerSensorOffset, RightIndexFingerOrientation));
		//DrawDebugPoint(GetWorld(), RightIndexFingerPosition, 5.0, FColor(255, 0, 0), false, 0.05);
	}	
}

void AHands_Character::RightMiddleFingerMovement(float ValueX)
{
	FRotator RawOrientation;
	RawOrientation.Roll = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_3RotationRoll");
	RawOrientation.Pitch = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_3RotationPitch");
	RawOrientation.Yaw = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_3RotationYaw");
	RightMiddleFingerOrientation = RectifyRightHandOrientation(RawOrientation);
	
	FVector RawPosition;
	RawPosition.X = ValueX;
	RawPosition.Y = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_3MotionY");
	RawPosition.Z = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_3MotionZ");
	FVector RectifiedRightMiddleFingerPosition = RectifyHandPosition(RawPosition);
	RightMiddleFingerPosition = RectifiedRightMiddleFingerPosition;
	if (bIsSystemCalibrated)
	{
		//RightMiddleFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightMiddleFingerPosition, FVector(0.f,0.f,0.f), RightMiddleFingerOrientation));
		//DrawDebugPoint(GetWorld(), RightMiddleFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
		RightMiddleFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightMiddleFingerPosition, RightMiddleFingerSensorOffset, RightMiddleFingerOrientation));
		//DrawDebugPoint(GetWorld(), RightMiddleFingerPosition, 5.0, FColor(255, 0, 0), false, 0.05);
	}	
}

void AHands_Character::RightRingFingerMovement(float ValueX)
{
	FRotator RawOrientation;
	RawOrientation.Roll = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_4RotationRoll");
	RawOrientation.Pitch = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_4RotationPitch");
	RawOrientation.Yaw = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_4RotationYaw");
	RightRingFingerOrientation = RectifyRightHandOrientation(RawOrientation);
	
	FVector RawPosition;
	RawPosition.X = ValueX;
	RawPosition.Y = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_4MotionY");
	RawPosition.Z = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_4MotionZ");
	FVector RectifiedRightRingFingerPosition = RectifyHandPosition(RawPosition);
	RightRingFingerPosition = RectifiedRightRingFingerPosition;
	if (bIsSystemCalibrated)
	{
		//RightRingFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightRingFingerPosition, FVector(0.f,0.f,0.f), RightRingFingerOrientation));
		//DrawDebugPoint(GetWorld(), RightRingFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
		RightRingFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightRingFingerPosition, RightRingFingerSensorOffset, RightRingFingerOrientation));
		//DrawDebugPoint(GetWorld(), RightRingFingerPosition, 5.0, FColor(255, 0, 0), false, 0.05);
	}	
}

void AHands_Character::RightPinkyFingerMovement(float ValueX)
{
	FRotator RawOrientation;
	RawOrientation.Roll = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_5RotationRoll");
	RawOrientation.Pitch = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_5RotationPitch");
	RawOrientation.Yaw = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_5RotationYaw");
	RightPinkyFingerOrientation = RectifyRightHandOrientation(RawOrientation);
	
	FVector RawPosition;
	RawPosition.X = ValueX;
	RawPosition.Y = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_5MotionY");
	RawPosition.Z = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_5MotionZ");
	FVector RectifiedRightPinkyFingerPosition = RectifyHandPosition(RawPosition);
	RightPinkyFingerPosition = RectifiedRightPinkyFingerPosition;
	if (bIsSystemCalibrated)
	{
		//RightPinkyFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightPinkyFingerPosition, FVector(0.f,0.f,0.f), RightPinkyFingerOrientation));
		//DrawDebugPoint(GetWorld(), RightPinkyFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
		RightPinkyFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightPinkyFingerPosition, RightPinkyFingerSensorOffset, RightPinkyFingerOrientation));
		//DrawDebugPoint(GetWorld(), RightPinkyFingerPosition, 5.0, FColor(255, 0, 0), false, 0.05);
	}
}

void AHands_Character::RightThumbMovement(float ValueX)
{
	FRotator RawOrientation;
	RawOrientation.Roll = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_6RotationRoll");
	RawOrientation.Pitch = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_6RotationPitch");
	RawOrientation.Yaw = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_6RotationYaw");
	RightThumbOrientation = RectifyRightHandOrientation(RawOrientation);
	
	FVector RawPosition;
	RawPosition.X = ValueX;
	RawPosition.Y = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_6MotionY");
	RawPosition.Z = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_6MotionZ");
	FVector RectifiedRightThumbPosition = RectifyHandPosition(RawPosition);
	RightThumbPosition = RectifiedRightThumbPosition;
	if (bIsSystemCalibrated)
	{
		//RightThumbPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightThumbPosition, FVector(0.f,0.f,0.f), RightThumbOrientation));
		//DrawDebugPoint(GetWorld(), RightThumbPosition, 5.0, FColor(0, 255, 0), false, 0.05);
		RightThumbPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightThumbPosition, RightThumbSensorOffset, RightThumbOrientation));
		//DrawDebugPoint(GetWorld(), RightThumbPosition, 5.0, FColor(255, 0, 0), false, 0.05);
	}	
}

void AHands_Character::LeftHandMovement(float ValueX)
{
	FRotator RawOrientation;
	RawOrientation.Roll = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_7RotationRoll");
	RawOrientation.Pitch = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_7RotationPitch");
	RawOrientation.Yaw = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_7RotationYaw");
	LeftHandOrientation = RectifyLeftHandOrientation(RawOrientation);
	
	FVector RawPosition;
	RawPosition.X = ValueX;
	RawPosition.Y = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_7MotionY");
	RawPosition.Z = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_7MotionZ");
	//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Red, FString::Printf(TEXT("LeftHand X: %f, Lefthand Y: %f, Lefthand Z: %f"), RawPosition.X, RawPosition.Y, RawPosition.Z));
	FVector RectifiedLeftHandPosition = RectifyHandPosition(RawPosition);
	LeftHandPosition = RectifiedLeftHandPosition;
	if (bIsSystemCalibrated)
	{
		//LeftHandPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftHandPosition, FVector(0.f,0.f,0.f), LeftHandOrientation));
		//DrawDebugPoint(GetWorld(), LeftHandPosition, 5.0, FColor(0, 255, 0), false, 0.05);
		LeftHandPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftHandPosition, LeftHandSensorOffset, LeftHandOrientation));
		//DrawDebugPoint(GetWorld(), LeftHandPosition, 5.0, FColor(255, 0, 0), false, 0.05);
	}

	// Set the knuckle position taking into consideration the sensor offset
	LeftMiddleKnucklePosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftHandPosition, LeftMiddleKnuckleSensorOffset, LeftHandOrientation));
}

void AHands_Character::LeftIndexFingerMovement(float ValueX)
{
	FRotator RawOrientation;
	RawOrientation.Roll = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_8RotationRoll");
	RawOrientation.Pitch = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_8RotationPitch");
	RawOrientation.Yaw = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_8RotationYaw");
	LeftIndexFingerOrientation = RectifyLeftHandOrientation(RawOrientation);
	
	FVector RawPosition;
	RawPosition.X = ValueX;
	RawPosition.Y = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_8MotionY");
	RawPosition.Z = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_8MotionZ");
	FVector RectifiedLeftIndexFingerPosition = RectifyHandPosition(RawPosition);
	LeftIndexFingerPosition = RectifiedLeftIndexFingerPosition;
	if (bIsSystemCalibrated)
	{
		//LeftIndexFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftIndexFingerPosition, FVector(0.f,0.f,0.f), LeftIndexFingerOrientation));
		//DrawDebugPoint(GetWorld(), LeftIndexFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
		LeftIndexFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftIndexFingerPosition, LeftIndexFingerSensorOffset, LeftIndexFingerOrientation));
		//DrawDebugPoint(GetWorld(), LeftIndexFingerPosition, 5.0, FColor(255, 0, 0), false, 0.05);
	}	
}

void AHands_Character::LeftMiddleFingerMovement(float ValueX)
{
	FRotator RawOrientation;
	RawOrientation.Roll = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_9RotationRoll");
	RawOrientation.Pitch = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_9RotationPitch");
	RawOrientation.Yaw = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_9RotationYaw");
	LeftMiddleFingerOrientation = RectifyLeftHandOrientation(RawOrientation);

	FVector RawPosition;
	RawPosition.X = ValueX;
	RawPosition.Y = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_9MotionY");
	RawPosition.Z = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_9MotionZ");
	FVector RectifiedLeftMiddleFingerPosition = RectifyHandPosition(RawPosition);
	LeftMiddleFingerPosition = RectifiedLeftMiddleFingerPosition;
	if (bIsSystemCalibrated)
	{
		//LeftMiddleFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftMiddleFingerPosition, FVector(0.f,0.f,0.f), LeftMiddleFingerOrientation));
		//DrawDebugPoint(GetWorld(), LeftMiddleFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
		LeftMiddleFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftMiddleFingerPosition, LeftMiddleFingerSensorOffset, LeftMiddleFingerOrientation));
		//DrawDebugPoint(GetWorld(), LeftMiddleFingerPosition, 5.0, FColor(255, 0, 0), false, 0.05);
	}
}

void AHands_Character::LeftRingFingerMovement(float ValueX)
{
	FRotator RawOrientation;
	RawOrientation.Roll = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_10RotationRoll");
	RawOrientation.Pitch = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_10RotationPitch");
	RawOrientation.Yaw = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_10RotationYaw");
	LeftRingFingerOrientation = RectifyLeftHandOrientation(RawOrientation);
	
	FVector RawPosition;
	RawPosition.X = ValueX;
	RawPosition.Y = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_10MotionY");
	RawPosition.Z = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_10MotionZ");
	FVector RectifiedLeftRingFingerPosition = RectifyHandPosition(RawPosition);
	LeftRingFingerPosition = RectifiedLeftRingFingerPosition;
	if (bIsSystemCalibrated)
	{
		//LeftRingFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftRingFingerPosition, FVector(0.f,0.f,0.f), LeftRingFingerOrientation));
		//DrawDebugPoint(GetWorld(), LeftRingFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
		LeftRingFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftRingFingerPosition, LeftRingFingerSensorOffset, LeftRingFingerOrientation));
		//DrawDebugPoint(GetWorld(), LeftRingFingerPosition, 5.0, FColor(255, 0, 0), false, 0.05);
	}
}

void AHands_Character::LeftPinkyFingerMovement(float ValueX)
{
	FRotator RawOrientation;
	RawOrientation.Roll = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_11RotationRoll");
	RawOrientation.Pitch = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_11RotationPitch");
	RawOrientation.Yaw = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_11RotationYaw");
	LeftPinkyFingerOrientation = RectifyLeftHandOrientation(RawOrientation);
	
	FVector RawPosition;
	RawPosition.X = ValueX;
	RawPosition.Y = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_11MotionY");
	RawPosition.Z = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_11MotionZ");
	FVector RectifiedLeftPinkyFingerPosition = RectifyHandPosition(RawPosition);
	LeftPinkyFingerPosition = RectifiedLeftPinkyFingerPosition;
	if (bIsSystemCalibrated)
	{
		//LeftPinkyFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftPinkyFingerPosition, FVector(0.f,0.f,0.f), LeftPinkyFingerOrientation));
		//DrawDebugPoint(GetWorld(), LeftPinkyFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
		LeftPinkyFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftPinkyFingerPosition, LeftPinkyFingerSensorOffset, LeftPinkyFingerOrientation));
		//DrawDebugPoint(GetWorld(), LeftPinkyFingerPosition, 5.0, FColor(255, 0, 0), false, 0.05);
	}
}

void AHands_Character::LeftThumbMovement(float ValueX)
{
	FRotator RawOrientation;
	RawOrientation.Roll = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_12RotationRoll");
	RawOrientation.Pitch = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_12RotationPitch");
	RawOrientation.Yaw = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_12RotationYaw");
	LeftThumbOrientation = RectifyLeftHandOrientation(RawOrientation);

	FVector RawPosition;
	RawPosition.X = ValueX;
	RawPosition.Y = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_12MotionY");
	RawPosition.Z = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_12MotionZ");
	FVector RectifiedLeftThumbPosition = RectifyHandPosition(RawPosition);
	LeftThumbPosition = RectifiedLeftThumbPosition;
	if (bIsSystemCalibrated)
	{
		//LeftThumbPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftThumbPosition, FVector(0.f,0.f,0.f), LeftThumbOrientation));
		//DrawDebugPoint(GetWorld(), LeftThumbPosition, 5.0, FColor(0, 255, 0), false, 0.05);
		LeftThumbPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftThumbPosition, LeftThumbSensorOffset, LeftThumbOrientation));
		//DrawDebugPoint(GetWorld(), LeftThumbPosition, 5.0, FColor(255, 0, 0), false, 0.05);
	}
}

// Modify original left hand orientation as to correctly follow the sensor input
FRotator AHands_Character::RectifyRightHandOrientation(FRotator RawOrientation)
{	
	// Rotate 180 degrees on Y axis as to have X axis facing in the opposite direction
	FQuat FirstChange = FQuat(RawOrientation) * FQuat(FVector(0.0f, 1.0f, 0.0f), FMath::DegreesToRadians(180));
	// Rotate 90 degrees on the X axis as to match the sensor's pitch and yaw with that of the hand
	FQuat SecondChange = FirstChange * FQuat(FVector(1.0f, 0.0f, 0.0f), FMath::DegreesToRadians(90));
	// Rotate the hand 90 degrees to the right as to have it face the correct direction
	FQuat ThirdChange = FQuat(FVector(0.0f, 0.0f, 1.0f), FMath::DegreesToRadians(90)) * SecondChange;
	// set hand correct orientation
	FRotator CorrectedOrientation = ThirdChange.Rotator();
	return CorrectedOrientation;
}

// Modify original left hand orientation as to correctly follow the sensor input
FRotator AHands_Character::RectifyLeftHandOrientation(FRotator RawOrientation)
{	
	// Rotate 90 degrees around X as sensor's pitch was hand's yaw, and sensor's yaw was hand's pitch
	FQuat FirstChange = FQuat(RawOrientation) * FQuat(FVector(1.0f, 0.0f, 0.0f), FMath::DegreesToRadians(90));
	// Rotate 90 degrees around Z as hand had an offset of 90 degrees to the left of the character
	FQuat SecondChange = FQuat(FVector(0.0f, 0.0f, 1.0f), FMath::DegreesToRadians(90)) * FirstChange;
	// Set hand correct rotation
	FRotator CorrectedOrientation = SecondChange.Rotator();
	return CorrectedOrientation;
}

FVector AHands_Character::RectifyHandPosition(FVector RawPosition, FVector SensorOffset, FRotator SensorOrientation)
{
	// Rotate sensor offset
	FVector RotatedSensorOffset = SensorOrientation.RotateVector(SensorOffset);
	// Rotate 90 degress around Z to match the skeletal mesh axis to those of the sensor
	FVector FirstChange = RawPosition.RotateAngleAxis(90.f, FVector(0.f, 0.f, 1.f));
	// Transform to share the same origin as the character
	FVector SecondChange = FirstChange - AxisTranslation;
	// Apply the sensor offset
	FVector CorrectedPosition = SecondChange + RotatedSensorOffset;
	// Return correct position
	return CorrectedPosition;
}

FVector AHands_Character::RectifyHandPosition(FVector RawPosition)
{
	// Rotate 90 degress around Z to match the skeletal mesh axis to those of the sensor
	FVector FirstChange = RawPosition.RotateAngleAxis(90.f, FVector(0.f, 0.f, 1.f));
	// Transform to share the same origin as the character
	FVector CorrectedPosition = FirstChange - AxisTranslation;
	// Return correct position
	return CorrectedPosition;
}

FVector AHands_Character::ApplySensorOffset(FVector Position, FVector SensorOffset, FRotator SensorOrientation)
{
	// Rotate sensor offset
	FVector RotatedSensorOffset = SensorOrientation.RotateVector(SensorOffset);
	// Apply the sensor offset
	FVector CorrectedPosition = Position + RotatedSensorOffset;
	// Return correct position
	return CorrectedPosition;
}

// Spawn the object we will interact with
void AHands_Character::SpawnObject1()
{
	if (SpawnedObject)
	{
		SpawnedObject->Destroy();
		bIsObject2Spawned = false;
		bIsObject3Spawned = false;
		bIsObject4Spawned = false;
	}
	
	// Check for a valid world
	UWorld* const World = GetWorld();
	if (World)
	{
		// Set the spawn parameters
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = Instigator;

		// spawn the pickup
		SpawnedObject = World->SpawnActor<AInteractionObject>(ObjectToSpawn1, FVector(500.f,500.f,50.f), FRotator(0.f,0.f,0.f), SpawnParams);
		if (SpawnedObject)
		{
			bIsObject1Spawned = true;
			OriginalMesh = SpawnedObject->OurVisibleComponent->StaticMesh;
		}
	}
}

void AHands_Character::SpawnObject2()
{
	if (SpawnedObject)
	{
		SpawnedObject->Destroy();
		bIsObject1Spawned = false;
		bIsObject3Spawned = false;
		bIsObject4Spawned = false;
	}
	
	// Check for a valid world
	UWorld* const World = GetWorld();
	if (World)
	{
		// Set the spawn parameters
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = Instigator;

		// spawn the pickup
		SpawnedObject = World->SpawnActor<AInteractionObject>(ObjectToSpawn2, FVector(500.f, 500.f, 50.f), FRotator(0.f, 0.f, 0.f), SpawnParams);
		if (SpawnedObject)
		{
			bIsObject2Spawned = true;
			OriginalMesh = SpawnedObject->OurVisibleComponent->StaticMesh;
		}
	}
}

void AHands_Character::SpawnObject3()
{
	if (SpawnedObject)
	{
		SpawnedObject->Destroy();
		bIsObject1Spawned = false;
		bIsObject2Spawned = false;
		bIsObject4Spawned = false;
	}

	// Check for a valid world
	UWorld* const World = GetWorld();
	if (World)
	{
		// Set the spawn parameters
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = Instigator;

		// spawn the pickup
		SpawnedObject = World->SpawnActor<AInteractionObject>(ObjectToSpawn3, FVector(500.f, 500.f, 50.f), FRotator(0.f, 0.f, 0.f), SpawnParams);
		if (SpawnedObject)
		{
			bIsObject3Spawned = true;
			OriginalMesh = SpawnedObject->OurVisibleComponent->StaticMesh;
		}
	}
}

void AHands_Character::SpawnObject4()
{
	if (SpawnedObject)
	{
		SpawnedObject->Destroy();
		bIsObject1Spawned = false;
		bIsObject2Spawned = false;
		bIsObject3Spawned = false;
	}
	
		// Check for a valid world
		UWorld* const World = GetWorld();
		if (World)
		{
			// Set the spawn parameters
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;

			// spawn the pickup
			SpawnedObject = World->SpawnActor<AInteractionObject>(ObjectToSpawn4, FVector(500.f, 500.f, 50.f), FRotator(0.f, 0.f, 0.f), SpawnParams);
			if (SpawnedObject)
			{
				bIsObject4Spawned = true;
				OriginalMesh = SpawnedObject->OurVisibleComponent->StaticMesh;
			}
		}
}

// set the P & O of our interaction object
void AHands_Character::Object1Movement(float ValueX)
{
	if (bIsObject1Spawned)
	{
		// Access the skeletal mesh
		
		// get the mesh position in world coordinates
		FVector RootLocation = MyMesh->GetSocketLocation(TEXT("Root"));
		// get the direction the mesh is facing
		FRotator RootDirection = MyMesh->GetForwardVector().GetSafeNormal().Rotation();
		
		// Get the position of the object's sensor
		FVector RawPosition;
		RawPosition.X = ValueX;
		RawPosition.Y = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_13MotionY");
		RawPosition.Z = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_13MotionZ");
		
		// Apply our calibration settings and rotate them -90 degrees to match the x,y axis
		FVector FirstChange = RawPosition - AxisTranslation.RotateAngleAxis(-90.f, FVector(0.f, 0.f, 1.f));
		// Rotate the sensor position to match the direction faced by the skeletal mesh
		FVector SecondChange = FirstChange.RotateAngleAxis(RootDirection.Yaw + 90, FVector(0.f, 0.f, 1.f));
		// Translate our sensor position to match the world position of the skeletal mesh
		FVector ObjectPosition = SecondChange + RootLocation;

		// Get the rotation of the object's sensor
		FRotator RawOrientation;
		RawOrientation.Roll = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_13RotationRoll");
		RawOrientation.Pitch = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_13RotationPitch");
		RawOrientation.Yaw = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_13RotationYaw");

		//UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAxisKeyValue

		// Set the position and orientation for the object
		SpawnedObject->SetActorLocationAndRotation(ObjectPosition, RawOrientation, true, false);
	}
}

void AHands_Character::Object2Movement(float ValueX)
{
	if (bIsObject2Spawned)
	{
		// Access the skeletal mesh

		// get the mesh position in world coordinates
		FVector RootLocation = MyMesh->GetSocketLocation(TEXT("Root"));
		// get the direction the mesh is facing
		FRotator RootDirection = MyMesh->GetForwardVector().GetSafeNormal().Rotation();

		// Get the position of the object's sensor
		FVector RawPosition;
		RawPosition.X = ValueX;
		RawPosition.Y = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_14MotionY");
		RawPosition.Z = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_14MotionZ");

		// Apply our calibration settings and rotate them -90 degrees to match the x,y axis
		FVector FirstChange = RawPosition - AxisTranslation.RotateAngleAxis(-90.f, FVector(0.f, 0.f, 1.f));
		// Rotate the sensor position to match the direction faced by the skeletal mesh
		FVector SecondChange = FirstChange.RotateAngleAxis(RootDirection.Yaw + 90, FVector(0.f, 0.f, 1.f));
		// Translate our sensor position to match the world position of the skeletal mesh
		FVector ObjectPosition = SecondChange + RootLocation;

		// Get the rotation of the object's sensor
		FRotator RawOrientation;
		RawOrientation.Roll = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_14RotationRoll");
		RawOrientation.Pitch = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_14RotationPitch");
		RawOrientation.Yaw = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_14RotationYaw");

		// Set the position and orientation for the object
		SpawnedObject->SetActorLocationAndRotation(ObjectPosition, RawOrientation, false, false);
	}
}

void AHands_Character::Object3Movement(float ValueX)
{
	if (bIsObject3Spawned)
	{
		// Access the skeletal mesh

		// get the mesh position in world coordinates
		FVector RootLocation = MyMesh->GetSocketLocation(TEXT("Root"));
		// get the direction the mesh is facing
		FRotator RootDirection = MyMesh->GetForwardVector().GetSafeNormal().Rotation();

		// Get the position of the object's sensor
		FVector RawPosition;
		RawPosition.X = ValueX;
		RawPosition.Y = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_15MotionY");
		RawPosition.Z = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_15MotionZ");

		// Apply our calibration settings and rotate them -90 degrees to match the x,y axis
		FVector FirstChange = RawPosition - AxisTranslation.RotateAngleAxis(-90.f, FVector(0.f, 0.f, 1.f));
		// Rotate the sensor position to match the direction faced by the skeletal mesh
		FVector SecondChange = FirstChange.RotateAngleAxis(RootDirection.Yaw + 90, FVector(0.f, 0.f, 1.f));
		// Translate our sensor position to match the world position of the skeletal mesh
		FVector ObjectPosition = SecondChange + RootLocation;

		// Get the rotation of the object's sensor
		FRotator RawOrientation;
		RawOrientation.Roll = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_15RotationRoll");
		RawOrientation.Pitch = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_15RotationPitch");
		RawOrientation.Yaw = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_15RotationYaw");

		// Set the position and orientation for the object
		SpawnedObject->SetActorLocationAndRotation(ObjectPosition, RawOrientation, false, false);
	}
}

void AHands_Character::Object4Movement(float ValueX)
{
	if (bIsObject4Spawned)
	{
		// Access the skeletal mesh

		// get the mesh position in world coordinates
		FVector RootLocation = MyMesh->GetSocketLocation(TEXT("Root"));
		// get the direction the mesh is facing
		FRotator RootDirection = MyMesh->GetForwardVector().GetSafeNormal().Rotation();

		// Get the position of the object's sensor
		FVector RawPosition;
		RawPosition.X = ValueX;
		RawPosition.Y = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_16MotionY");
		RawPosition.Z = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_16MotionZ");

		// Apply our calibration settings and rotate them -90 degrees to match the x,y axis
		FVector FirstChange = RawPosition - AxisTranslation.RotateAngleAxis(-90.f, FVector(0.f, 0.f, 1.f));
		// Rotate the sensor position to match the direction faced by the skeletal mesh
		FVector SecondChange = FirstChange.RotateAngleAxis(RootDirection.Yaw + 90, FVector(0.f, 0.f, 1.f));
		// Translate our sensor position to match the world position of the skeletal mesh
		FVector ObjectPosition = SecondChange + RootLocation;

		// Get the rotation of the object's sensor
		FRotator RawOrientation;
		RawOrientation.Roll = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_16RotationRoll");
		RawOrientation.Pitch = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_16RotationPitch");
		RawOrientation.Yaw = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputAnalogKeyState("Sensor_16RotationYaw");

		// Set the position and orientation for the object
		SpawnedObject->SetActorLocationAndRotation(ObjectPosition, RawOrientation, false, false);
	}
}

void AHands_Character::ModifyObjectSize()
{
	if (SpawnedObject)
	{
		FVector ObjectScale = SpawnedObject->OurVisibleComponent->RelativeScale3D;
		//float NewZScale = ObjectScale.Z * 1.05f;
		//SpawnedObject->OurVisibleComponent->SetRelativeScale3D(FVector(ObjectScale.X * 1.05f, ObjectScale.Y * 1.05f, ObjectScale.Z * 1.05f));
		SpawnedObject->OurVisibleComponent->SetRelativeScale3D(FVector(ObjectScale.X, ObjectScale.Y, ObjectScale.Z * 1.05f));
		bHasObjectSizeChanged = true;
	}
}

void AHands_Character::ResetObjectSize()
{
	if (SpawnedObject)
	{
		SpawnedObject->OurVisibleComponent->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
		bHasObjectSizeChanged = false;
	}
}

void AHands_Character::AssignPointers()
{
	switch (CurrentMeshIdentificator)
	{
	case 1:
		PointerToCurrentMeshVertices = &OriginalMeshVerticesCoordinatesFromUE4Asset;
		PointerToCurrentMeshNormals = &OriginalMeshVerticesNormalsFromUE4Asset;
		PointerToCurrentMeshTangents = &OriginalMeshVerticesTangentsFromUE4Asset;
		PointerToCurrentMeshBinormals = &OriginalMeshVerticesBinormalsFromUE4Asset;
		break;
	case 2:
		PointerToCurrentMeshVertices = &SecondMeshVerticesCoordinatesFromUE4Asset;
		PointerToCurrentMeshNormals = &SecondMeshVerticesNormalsFromUE4Asset;
		PointerToCurrentMeshTangents = &SecondMeshVerticesTangentsFromUE4Asset;
		PointerToCurrentMeshBinormals = &SecondMeshVerticesBinormalsFromUE4Asset;
		break;
	default:
		PointerToCurrentMeshVertices = &OriginalMeshVerticesCoordinatesFromUE4Asset;
		PointerToCurrentMeshNormals = &OriginalMeshVerticesNormalsFromUE4Asset;
		PointerToCurrentMeshTangents = &OriginalMeshVerticesTangentsFromUE4Asset;
		PointerToCurrentMeshBinormals = &OriginalMeshVerticesBinormalsFromUE4Asset;
	}
}

void AHands_Character::GetMeshCurrentTransform(const UStaticMeshComponent* InStaticMesh, FMatrix& CurrentMatrix, FTransform& CurrentTransform, int32& VerticesNum)
{
	UStaticMesh* StaticMesh = InStaticMesh->StaticMesh;
	if (StaticMesh == nullptr || StaticMesh->RenderData == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Error while accesing 'StaticMesh' on GetMeshCurrentTransform()"));
		return;
	}
	FStaticMeshLODResources& LODModel = StaticMesh->RenderData->LODResources[0];
	FPositionVertexBuffer& PositionVertexBuffer = LODModel.PositionVertexBuffer;
	if (PositionVertexBuffer.GetNumVertices() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("NumVertices == 0 on GetMeshCurrentTransform()"));
		return;
	}
	FIndexArrayView Indices = LODModel.IndexBuffer.GetArrayView();
	VerticesNum = Indices.Num();
	CurrentMatrix = InStaticMesh->ComponentToWorld.ToMatrixWithScale().InverseFast().GetTransposed();
	CurrentTransform = InStaticMesh->ComponentToWorld;
}

// Randomly set the descriptor points
void AHands_Character::InitDescriptionPoints(uint32 NumDP, uint32 NumVertices)
{
	// Run the for loop for as many descriptor points we need
	for (uint32 i = 0; i < NumDP; i++)
		{

			// Generate random number
			uint32 RandomIndex = FMath::RandRange(0, NumVertices - 1);
			// Is that random rumber repeated?
			uint32 val = VertexIndices[RandomIndex];
			bool bIsIndexRepeated = DPIndices.Contains(val);
			// while the ramdom number is repeated, keep generating new random numbers
			while (bIsIndexRepeated)
			{
				RandomIndex = FMath::RandRange(0, NumVertices - 1);
				val = VertexIndices[RandomIndex];
				bIsIndexRepeated = DPIndices.Contains(val);
			}
			// store random number on our indices array
			DPIndices.Emplace(val);
		}
	return;
}

// Access the mesh information and get the position, normal, tangent & binormal of the vertices
void AHands_Character::AccessTriVertices(const UStaticMeshComponent* InStaticMesh, TArray<FVector>& DPInfo)
{
	UStaticMesh* StaticMesh = InStaticMesh->StaticMesh;
	if (StaticMesh == nullptr || StaticMesh->RenderData == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Error while accesing the static mesh")));
		return;
	}
	FStaticMeshLODResources& LODModel = StaticMesh->RenderData->LODResources[0];
	FPositionVertexBuffer& PositionVertexBuffer = LODModel.PositionVertexBuffer;
	if (PositionVertexBuffer.GetNumVertices() == 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Error with the number of vertices")));
		return;
	}
	FIndexArrayView Indices = LODModel.IndexBuffer.GetArrayView();
	FMatrix LocalToWorldInverseTranspose = InStaticMesh->ComponentToWorld.ToMatrixWithScale().InverseFast().GetTransposed();
	uint32 NumIndices = Indices.Num();
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("%i"), NumIndices));
	//if (!bAreDPset)
	//{
	//	TArray<FVector> VertexPositions;
	//	VertexPositions.Empty();
	//	VertexIndices.Empty();
	//	
	//	for (uint32 Index = 0; Index < NumIndices; Index++)
	//	{
	//		FVector WorldVert0 = InStaticMesh->ComponentToWorld.TransformPosition(PositionVertexBuffer.VertexPosition(Indices[Index]));
	//		bool bIsPointAlreadyUsed = VertexPositions.Contains(WorldVert0);
	//		if (!bIsPointAlreadyUsed)
	//		{
	//			VertexPositions.Emplace(WorldVert0);
	//			VertexIndices.Emplace(Index);

	//		}
	//	}
	//	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("%i"), VertexIndices.Num()));
	//	if (NumDescriptorPoints <= uint32(VertexIndices.Num()))
	//	{
	//		DPIndices.Empty();
	//		AHands_Character::InitDescriptionPoints(NumDescriptorPoints, VertexIndices.Num());
	//		pDPIndices = &DPIndices;
	//		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("if")));
	//	}
	//	else
	//	{
	//		pDPIndices = &VertexIndices;
	//		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("else")));
	//	}
	//	bAreDPset = true;
	//	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%i"), NumIndices));
	//}
	//TArray<FVector>& DPInfo = *DescriptorPointsArray;
	//TArray<uint32>& RpDPIndices = *pDPIndices;
	//uint32 limit = RpDPIndices.Num();
	uint32 limit = NumIndices;
	for (uint32 Index = 0; Index < limit; Index++)
	{
		//uint32 I = RpDPIndices[Index];
		
		/*FVector WorldVert0 = InStaticMesh->ComponentToWorld.TransformPosition(PositionVertexBuffer.VertexPosition(Indices[I]));
		DescriptorPointInfo.Emplace(WorldVert0);
		const FVector& Normal = LocalToWorldInverseTranspose.TransformVector(LODModel.VertexBuffer.VertexTangentZ(Indices[I])).GetSafeNormal();
		DescriptorPointInfo.Emplace(Normal);
		const FVector& Tangent = LocalToWorldInverseTranspose.TransformVector(LODModel.VertexBuffer.VertexTangentX(Indices[I])).GetSafeNormal();
		DescriptorPointInfo.Emplace(Tangent);
		const FVector& Binormal = LocalToWorldInverseTranspose.TransformVector(LODModel.VertexBuffer.VertexTangentY(Indices[I])).GetSafeNormal();
		DescriptorPointInfo.Emplace(Binormal);*/
		FVector WorldVert0 = InStaticMesh->ComponentToWorld.TransformPosition(PositionVertexBuffer.VertexPosition(Indices[Index]));
		DPInfo.Emplace(WorldVert0);
		const FVector& Normal = LocalToWorldInverseTranspose.TransformVector(LODModel.VertexBuffer.VertexTangentZ(Indices[Index])).GetSafeNormal();
		DPInfo.Emplace(Normal);
		const FVector& Tangent = LocalToWorldInverseTranspose.TransformVector(LODModel.VertexBuffer.VertexTangentX(Indices[Index])).GetSafeNormal();
		DPInfo.Emplace(Tangent);
		const FVector& Binormal = LocalToWorldInverseTranspose.TransformVector(LODModel.VertexBuffer.VertexTangentY(Indices[Index])).GetSafeNormal();
		DPInfo.Emplace(Binormal);
	}
	//UE_LOG(LogTemp, Warning, TEXT("Array final size: %d"), DPInfo.Num()/4);
	return;
}

// Draw the normal, tangent & binormal of our descriptor points
void AHands_Character::DrawDescriptionPoints(TArray<FVector>& DPInfo)
{
	//TArray<FVector>& DPInfo = *DescriptorPointsArray;
	uint32 limit = pDPIndices->Num();
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("draw debug points %d"), limit));
	for (uint32 i = 0; i < limit; i++)
	{
		//Position = DescriptorPointInfo[i * 4]
		//Normal = DescriptorPointInfo [(i * 4) + 1]
		//Tangent = DescriptorPointInfo [(i * 4) + 2]
		//Binormal = DescriptorPointInfo[(i * 4) + 3]
		DrawDebugPoint(GetWorld(), DPInfo[i * 4], 2.0, FColor(255, 0, 255), false, 0.03);
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("%f, %f, %f"), DPInfo[i * 4].X, DPInfo[i * 4].Y, DPInfo[i * 4].Z));
		DrawDebugLine(GetWorld(), DPInfo[i * 4], DPInfo[i * 4] + DPInfo[(i * 4) + 1] * 3.f, FColor(255, 0, 0), false, -1, 0, .5f);
		DrawDebugLine(GetWorld(), DPInfo[i * 4], DPInfo[i * 4] + DPInfo[(i * 4) + 2] * 3.f, FColor(0, 255, 0), false, -1, 0, .5f);
		DrawDebugLine(GetWorld(), DPInfo[i * 4], DPInfo[i * 4] + DPInfo[(i * 4) + 3] * 3.f, FColor(0, 0, 255), false, -1, 0, .5f);
	}

}

// Calculate the weights for the descriptor points
void AHands_Character::WeightsComputation(TArray<float>& w_biprime, TArray<float>& transformation, FVector p_j)
{	
	//TArray<float>& transformation = *TransformationArray;
	//TArray<float>& w_biprime = *FingerWeightsArray;
	TArray<float> distance;
	TArray<float> w_prime;
	if (bHasObjectSizeChanged)
	{
		FVector ObjectScale = SpawnedObject->OurVisibleComponent->RelativeScale3D;
		SpawnedObject->OurVisibleComponent->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));		
		DPOriginalObject.Empty();
		AHands_Character::AccessTriVertices(SpawnedObject->OurVisibleComponent, DPOriginalObject);
		//AHands_Character::DrawDescriptionPoints(DPOriginalObject);
		DescriptorPointsPointer = &DPOriginalObject;
		SpawnedObject->OurVisibleComponent->SetRelativeScale3D(ObjectScale);
	}
	else if (bHasObjectMeshChanged)
	{
		FTransform ObjectTransform = SpawnedObject->GetTransform();
		DescriptorPointsPointer = &DPVirtualObject;
	}
	else
	{
		DescriptorPointsPointer = &DPVirtualObject;
	}
	
	TArray<FVector>& DPInfo = *DescriptorPointsPointer;
	TArray<FVector>& DPInfo2 = DPVirtualObject;
	//uint32 limit = pDPIndices->Num();
	uint32 limit = DPInfo.Num()/4;
	//UE_LOG(LogTemp, Warning, TEXT("Limit %d"), limit);
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("weights computation %d"), limit));
	for (uint32 i = 0; i < limit; i++)
	{
		//Position = DescriptorPointInfo[i * 4]
		//Normal = DescriptorPointInfo [(i * 4) + 1]
		//Tangent = DescriptorPointInfo [(i * 4) + 2]
		//Binormal = DescriptorPointInfo[(i * 4) + 3]
		if (DPInfo.IsValidIndex(i * 4))
		{
			distance.Emplace(FVector::Dist(p_j, DPInfo[i * 4]));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Index i * 4: %d of DPInfo is invalid at AHands_Character::WeightsComputation"), limit);
		}
		//if (false)
		if (FVector::DotProduct(DPInfo[(i * 4) + 1], p_j - DPInfo[i * 4]) < 0)
		{
			w_prime.Emplace(0.f);
		}
		else
		{			
			float w_prime_val = (FVector::DotProduct(DPInfo[(i * 4) + 1], p_j - DPInfo[i * 4])) / distance[i];
			w_prime.Emplace(w_prime_val);
		}
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("weights computation %f"), w_prime_val));
		transformation.Emplace(FVector::DotProduct(p_j - DPInfo2[i * 4], DPInfo[(i * 4) + 1]));
		transformation.Emplace(FVector::DotProduct(p_j - DPInfo2[i * 4], DPInfo[(i * 4) + 2]));
		transformation.Emplace(FVector::DotProduct(p_j - DPInfo2[i * 4], DPInfo[(i * 4) + 3]));		
	}
	//FVector puntito(transformation[60], transformation[61], transformation[62]);
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Transformation x: %f y: %f z: %f"), puntito.X, puntito.Y, puntito.Z));
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Distance %f"), distance[20]));
	//DrawDebugLine(GetWorld(), DPInfo[20], DPInfo[20] + (puntito), FColor(0, 0, 255), false, -1, 0, .5f);
	float r_j_1 = FMath::Min<float>(distance);
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("r_j_1 %f"), r_j_1));
	float r_j_2 = r_j_1 + (0.25 * 200);
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("weights computation %f"), r_j_2));
	sum_w_biprime = 0;
	float valor = 0;
	for (uint32 i = 0; i < limit; i++)
	{
		
		if (r_j_2 <= distance[i])
		{
			valor = 0;
		}
		else if (r_j_1 < distance[i] && r_j_2 > distance[i])
		{
			valor = 1 - (distance[i] - r_j_1) / (r_j_2 - r_j_1);
		}
		else if (distance[i] <= r_j_1)
		{
			valor = 1;
		}
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("weights computation %f"), val));
		float w_biprime_val = w_prime[i] * valor;
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("w_biprime_val: %f"), w_biprime_val));
		sum_w_biprime += w_biprime_val;
		w_biprime.Emplace(w_biprime_val);
	}
	return;
}

void AHands_Character::WeightsComputation(FVector p_j, TArray<FVector>& TransformationComponents, TArray<float>& w_biprime)
{
	int32 limit;
	if (bHasObjectSizeChanged)
	{
		FVector ObjectScale = SpawnedObject->OurVisibleComponent->RelativeScale3D;
		SpawnedObject->OurVisibleComponent->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
		
		GetMeshCurrentTransform(SpawnedObject->OurVisibleComponent, OriginalMeshLocalToWorldMatrix, OriginalMeshComponentToWorldTransform, OriginalVerticesNum);
		SpawnedObject->OurVisibleComponent->SetRelativeScale3D(ObjectScale);

		// The object is just changing sizes, so the vertices num doesn't change
		limit = OriginalVerticesNum;
	} 
	else if (bHasObjectMeshChanged)
	{
		UStaticMesh* CurrentMesh = SpawnedObject->OurVisibleComponent->StaticMesh;
		SpawnedObject->OurVisibleComponent->SetStaticMesh(OriginalMesh);

		GetMeshCurrentTransform(SpawnedObject->OurVisibleComponent, OriginalMeshLocalToWorldMatrix, OriginalMeshComponentToWorldTransform, OriginalVerticesNum);

		SpawnedObject->OurVisibleComponent->SetStaticMesh(CurrentMesh);

		limit = OriginalVerticesNum;
	}
	
	w_biprime.Empty();
	w_biprime.Reserve(limit);
	TransformationComponents.Empty();
	TransformationComponents.Reserve(limit);
	TArray<float> Distance;
	Distance.Reserve(limit);
	TArray<float> w_prime;
	w_prime.Reserve(limit);
	TArray<FVector>& Vertices = OriginalMeshVerticesCoordinatesFromUE4Asset;
	TArray<FVector>& Normals = OriginalMeshVerticesNormalsFromUE4Asset;
	TArray<FVector>& Tangents = OriginalMeshVerticesTangentsFromUE4Asset;
	TArray<FVector>& Binormals = OriginalMeshVerticesBinormalsFromUE4Asset;
	//UE_LOG(LogTemp, Warning, TEXT("Limit %d"), limit);
	//Engine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("Num Vertices %d"), Vertices.Num()));
	for (int32 i = 0; i < limit; i++)
	{
		FVector TransformedVertices;
		FVector TransformedNormals;
		FVector TransformedTangents;
		FVector TransformedBinormals;
		float Alpha;
		float Beta;
		float Gamma;

		if (Vertices.IsValidIndex(i))
		{
			TransformedVertices = OriginalMeshComponentToWorldTransform.TransformPosition(Vertices[i]);
			Distance.Emplace(FVector::Dist(p_j, TransformedVertices));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid index while calculating index %d on 'TransformedVertices' on WeightComputation()"), i);
			return;
		}
		
		if (Normals.IsValidIndex(i))
		{
			TransformedNormals = OriginalMeshLocalToWorldMatrix.TransformVector(Normals[i]).GetSafeNormal();
			float w_prime_val = (FVector::DotProduct(TransformedNormals, p_j - TransformedVertices)) / Distance[i];
			w_prime.Emplace(w_prime_val);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid index while calculating 'TransformedNormals' on WeightComputation()"));
			return;
		}
		
		Alpha = FVector::DotProduct(p_j - TransformedVertices, TransformedNormals);
		
		if (Tangents.IsValidIndex(i))
		{
			TransformedTangents = OriginalMeshLocalToWorldMatrix.TransformVector(Tangents[i]).GetSafeNormal();
			Beta = FVector::DotProduct(p_j - TransformedVertices, TransformedTangents);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid index while calculating 'TransformedTangents' on WeightComputation()"));
			return;
		}

		if (Binormals.IsValidIndex(i))
		{
			TransformedBinormals = OriginalMeshLocalToWorldMatrix.TransformVector(Binormals[i]).GetSafeNormal();
			Gamma = FVector::DotProduct(p_j - TransformedVertices, TransformedBinormals);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid index while calculating 'TransformedBinormals' on WeightComputation()"));
			return;
		}
		TransformationComponents.Emplace(FVector(Alpha, Beta, Gamma));
	}

	float r_j_1 = FMath::Min<float>(Distance);
	float r_j_2 = r_j_1 + (0.25 * 200);
	sum_w_biprime = 0;
	float valor = 0;
	for (int32 i = 0; i < limit; i++)
	{

		if (r_j_2 <= Distance[i])
		{
			valor = 0;
		}
		else if (r_j_1 < Distance[i] && r_j_2 > Distance[i])
		{
			valor = 1 - (Distance[i] - r_j_1) / (r_j_2 - r_j_1);
		}
		else if (Distance[i] <= r_j_1)
		{
			valor = 1;
		}
		float w_biprime_val = w_prime[i] * valor;
		sum_w_biprime += w_biprime_val;
		w_biprime.Emplace(w_biprime_val);
	}

}

FVector AHands_Character::NewJointPosition(TArray<float>& w_biprime, TArray<float>& transformation, TArray<FVector>& DPInfo)
{
	//TArray<float>& w_biprime = *FingerWeightsArray;
	//TArray<float>& transformation = *TransformationArray;
	//TArray<FVector>& DPInfo = *DescriptorPointsArray;
	FVector vector(0.f, 0.f, 0.f);
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("New joint position %d"), limit));
	float sum_wbiprime = 0;
	uint32 limit = DPInfo.Num()/4;
	for (uint32 i = 0; i < limit; i++)
	{
		sum_wbiprime += w_biprime[i];
	}
	
	for (uint32 i = 0; i < limit; i++)
	{
		vector += (w_biprime[i] / sum_wbiprime) * (DPInfo[i * 4] + (transformation[(i * 3)] * (DPInfo[(i * 4) + 1])) +
			(transformation[(i * 3) + 1] * (DPInfo[(i * 4) + 2])) + (transformation[(i * 3) + 2] * (DPInfo[(i * 4) + 3])));
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("Normalized weight: %f"), (w_biprime[i] / sum_wbiprime)));
	}
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("Normalized weight: %f"), (w_biprime[i] / sum_wbiprime)));
	return vector;
}

FVector AHands_Character::NewJointPosition(TArray<float>& w_biprime, TArray<FVector>& TransformationComponents)
{
	FVector NewJointPosition(0.f, 0.f, 0.f);
	
	float sum_wbiprime = 0;
	int32 limit = CurrentVerticesNum;
	for (int32 i = 0; i < limit; i++)
	{
		if (w_biprime.IsValidIndex(i))
		{
			sum_wbiprime += w_biprime[i];
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid index %d while calculating 'sum_biprime' on AHands_Character::NewJointPosition()"), i);
			return FVector(0.f, 0.f, 0.f);
		}
	}

	TArray<FVector>& Vertices = *PointerToCurrentMeshVertices;
	TArray<FVector>& Normals = *PointerToCurrentMeshNormals;
	TArray<FVector>& Tangents = *PointerToCurrentMeshTangents;
	TArray<FVector>& Binormals = *PointerToCurrentMeshBinormals;
	
	for (int32 i = 0; i < limit; i++)
	{
		float& Alpha = TransformationComponents[i].X;
		float& Beta = TransformationComponents[i].Y;
		float& Gamma = TransformationComponents[i].Z;
		FVector TransformedVertices = CurrentMeshComponentToWorldTransform.TransformPosition(Vertices[i]);
		FVector TransformedNormals = CurrentMeshLocalToWorldMatrix.TransformVector(Normals[i]).GetSafeNormal();
		FVector TransformedTangents = CurrentMeshLocalToWorldMatrix.TransformVector(Tangents[i]).GetSafeNormal();
		FVector TransformedBinormals = CurrentMeshLocalToWorldMatrix.TransformVector(Binormals[i]).GetSafeNormal();

		NewJointPosition += (w_biprime[i] / sum_wbiprime) * (TransformedVertices + (Alpha * TransformedNormals) + (Beta * TransformedTangents) + (Gamma * TransformedBinormals));
	}
	
	return NewJointPosition;
}

void AHands_Character::Answer1()
{
	// if experiment has ended
	if (bIsExperimentFinished)
	{
		ObjectChosen = 0;
		bIsDecisionMade = true;
	}
}

void AHands_Character::Answer2()
{
	// if experiment has ended
	if (bIsExperimentFinished)
	{
		ObjectChosen = 1;
		bIsDecisionMade = true;
	}
}

void AHands_Character::Answer3()
{
	// if experiment has ended
	if (bIsExperimentFinished)
	{
		ObjectChosen = 2;
		bIsDecisionMade = true;
	}
}

void AHands_Character::Answer4()
{
	// if experiment has ended
	if (bIsExperimentFinished)
	{
		ObjectChosen = 3;
		bIsDecisionMade = true;
	}
}

bool AHands_Character::GetDelayState()
{
	return bIsDelayActive;
}

float AHands_Character::GetAlphaValue()
{
	return AlphaValue;
}

// Functions to access the left hand and finger P & O
FVector AHands_Character::GetLeftHandPosition()
{
	//if (false)
	if (SpawnedObject && bAreDPsActive)
	{
		return DPLeftHandPosition;
	}
	else
	{
		return LeftHandPosition;
	}
}

FVector AHands_Character::GetLeftIndexFingerPosition()
{
	if (SpawnedObject && bAreDPsActive)
	{
		return DPLeftIndexFingerPosition;
	}
	else
	{
		return LeftIndexFingerPosition;
	}
}

FVector AHands_Character::GetLeftMiddleFingerPosition()
{
	if (SpawnedObject && bAreDPsActive)
	{
		return DPLeftMiddleFingerPosition;
	}
	else
	{
		return LeftMiddleFingerPosition;
	}
}

FVector AHands_Character::GetLeftRingFingerPosition()
{
	if (SpawnedObject && bAreDPsActive)
	{
		return DPLeftRingFingerPosition;
	}
	else
	{
		return LeftRingFingerPosition;
	}
}

FVector AHands_Character::GetLeftPinkyFingerPosition()
{
	if (SpawnedObject && bAreDPsActive)
	{
		return DPLeftPinkyFingerPosition;
	}
	else
	{
		return LeftPinkyFingerPosition;
	}
}

FVector AHands_Character::GetLeftThumbPosition()
{
	if (SpawnedObject && bAreDPsActive)
	{
		return DPLeftThumbPosition;
	}
	else
	{
		return LeftThumbPosition;
	}
}

FRotator AHands_Character::GetLeftHandOrientation()
{
	return LeftHandOrientation;
}

FRotator AHands_Character::GetLeftIndexFingerOrientation()
{
	return LeftIndexFingerOrientation;
}

FRotator AHands_Character::GetLeftMiddleFingerOrientation()
{
	return LeftMiddleFingerOrientation;
}

FRotator AHands_Character::GetLeftRingFingerOrientation()
{
	return LeftRingFingerOrientation;
}

FRotator AHands_Character::GetLeftPinkyFingerOrientation()
{
	return LeftPinkyFingerOrientation;
}

FRotator AHands_Character::GetLeftThumbOrientation()
{
	return LeftThumbOrientation;
}

// Functions to access the left knuckles P & O

FVector AHands_Character::GetLeftMiddleKnucklePosition()
{
	//if (false)
	if (SpawnedObject && bAreDPsActive)
	{
		return DPLeftMiddleKnucklePosition;
	}
	else
	{
		return LeftMiddleKnucklePosition;
	}
}

// Functions to access the right hand and fingers P & O
FVector AHands_Character::GetRightHandPosition()
{
	if (SpawnedObject && bAreDPsActive)
	{
		return DPRightHandPosition;
	}
	else
	{
		return RightHandPosition;
	}
}

FVector AHands_Character::GetRightIndexFingerPosition()
{
	if (SpawnedObject && bAreDPsActive)
	{
		return DPRightIndexFingerPosition;
	}
	else
	{
		return RightIndexFingerPosition;
	}
}

FVector AHands_Character::GetRightMiddleFingerPosition()
{
	if (SpawnedObject && bAreDPsActive)
	{
		return DPRightMiddleFingerPosition;
	}
	else
	{
		return RightMiddleFingerPosition;
	}
}

FVector AHands_Character::GetRightRingFingerPosition()
{
	if (SpawnedObject && bAreDPsActive)
	{
		return DPRightRingFingerPosition;
	}
	else
	{
		return RightRingFingerPosition;
	}
}

FVector AHands_Character::GetRightPinkyFingerPosition()
{
	if (SpawnedObject && bAreDPsActive)
	{
		return DPRightPinkyFingerPosition;
	}
	else
	{
		return RightPinkyFingerPosition;
	}
}

FVector AHands_Character::GetRightThumbPosition()
{
	if (SpawnedObject && bAreDPsActive)
	{
		return DPRightThumbPosition;
	}
	else
	{
		return RightThumbPosition;
	}
}

FRotator AHands_Character::GetRightHandOrientation()
{
	return RightHandOrientation;
}

FRotator AHands_Character::GetRightIndexFingerOrientation()
{
	return RightIndexFingerOrientation;
}

FRotator AHands_Character::GetRightMiddleFingerOrientation()
{
	return RightMiddleFingerOrientation;
}

FRotator AHands_Character::GetRightRingFingerOrientation()
{
	return RightRingFingerOrientation;
}

FRotator AHands_Character::GetRightPinkyFingerOrientation()
{
	return RightPinkyFingerOrientation;
}

FRotator AHands_Character::GetRightThumbOrientation()
{
	return RightThumbOrientation;
}

// Functions to access the Right knuckles P & O

FVector AHands_Character::GetRightMiddleKnucklePosition()
{
	//if (false)
	if (SpawnedObject && bAreDPsActive)
	{
		return DPRightMiddleKnucklePosition;
	}
	else
	{
		return RightMiddleKnucklePosition;
	}
}