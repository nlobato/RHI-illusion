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
	LeftMiddleFingerSensorOffset = FVector(-2.739685, -0.870843, -0.165751);
	LeftRingFingerSensorOffset = FVector(-1.81612, -1.565517, 0.065366);
	LeftPinkyFingerSensorOffset = FVector(-1.296994, -1.144543, 0.005556);
	LeftThumbSensorOffset = FVector(-2.08541, -1.472764, -0.654292);

	LeftIndexKnuckleSensorOffset = FVector(2.520503, -1.646863, -2.551013);
	LeftMiddleKnuckleSensorOffset = FVector(2.252283, -1.617367, -0.055583);
	LeftRingKnuckleSensorOffset = FVector(2.199424, -0.868081, 2.279425);
	LeftPinkyKnuckleSensorOffset = FVector(1.821449, -0.867593, 4.397741);

	
	//LeftHandSensorOffset = FVector(0, -3.5, 0);
	RightHandSensorOffset = FVector(10.581812, 2.98531, -0.622225);

	RightIndexFingerSensorOffset = FVector(1.561717, 1.282756, -0.064839);
	RightMiddleFingerSensorOffset = FVector(2.739685, 0.870843, -0.165751);
	RightRingFingerSensorOffset = FVector(1.81612, 1.565517, -0.065366);
	RightPinkyFingerSensorOffset = FVector(1.296994, 1.144543, -0.005556);
	RightThumbSensorOffset = FVector(2.08541, 1.472764, 0.654292);

	RightIndexKnuckleSensorOffset = FVector(-2.520503, 1.646863, 2.551013);
	RightMiddleKnuckleSensorOffset = FVector(-2.252283, 1.617367, 0.055583);
	RightRingKnuckleSensorOffset = FVector(-2.199424, 0.868081, -2.279425);
	RightPinkyKnuckleSensorOffset = FVector(-1.821449, 0.867593, -4.397741);

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
			if (bDrawCurrentMeshPoints)
			{				
				AHands_Character::DrawDescriptionPoints(DPVirtualObject);
			}
			
			// Time delay for the asynchronous condition is now handled in the AnimBlueprint

			if (bAreDPsActive)
			{
				// Left Hand

				//UE_LOG(LogTemp, Warning, TEXT("We entered the if statement for AreDPsActive"));
				//LeftHandWeights.Empty();
				//LeftHandTransformation.Empty();
				//AHands_Character::WeightsComputation(LeftHandWeights, LeftHandTransformation, LeftHandPosition);
				//DPLeftHandPosition = AHands_Character::NewJointPosition(LeftHandWeights, LeftHandTransformation, DPVirtualObject);
				WeightsComputation(LeftHandPosition, LeftHandTransformationArray, LeftHandWeights, bDrawDebugWeightsLeftHand);
				DPLeftHandPosition = NewJointPosition(LeftHandWeights, LeftHandTransformationArray, bDrawDebugLeftHandPosition);
				
				/*LeftMiddleKnuckleWeights.Empty();
				LeftMiddleKnuckleTransformation.Empty();
				AHands_Character::WeightsComputation(LeftMiddleKnuckleWeights, LeftMiddleKnuckleTransformation, LeftMiddleKnucklePosition);
				DPLeftMiddleKnucklePosition = AHands_Character::NewJointPosition(LeftMiddleKnuckleWeights, LeftMiddleKnuckleTransformation, DPVirtualObject);*/
				WeightsComputation(LeftMiddleKnucklePosition, LeftMiddleKnuckleTransformationArray, LeftMiddleKnuckleWeights, bDrawDebugWeightsLeftKnuckle);
				DPLeftMiddleKnucklePosition = NewJointPosition(LeftMiddleKnuckleWeights, LeftMiddleKnuckleTransformationArray, bDrawDebugLeftKnucklePosition);

				WeightsComputation(LeftIndexKnucklePosition, LeftIndexKnuckleTransformationArray, LeftIndexKnuckleWeights, false);
				DPLeftIndexKnucklePosition = NewJointPosition(LeftIndexKnuckleWeights, LeftIndexKnuckleTransformationArray, bDrawDebugWeightsLeftKnuckle);

				/*LeftIndexFingerWeights.Empty();
				LeftIndexFingerTransformation.Empty();
				AHands_Character::WeightsComputation(LeftIndexFingerWeights, LeftIndexFingerTransformation, LeftIndexFingerPosition);
				DPLeftIndexFingerPosition = AHands_Character::NewJointPosition(LeftIndexFingerWeights, LeftIndexFingerTransformation, DPVirtualObject);*/
				GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("Left Index Orientation %s"), *LeftIndexFingerOrientation.ToString()));
				WeightsComputation(LeftIndexFingerPosition, LeftIndexFingerTransformationArray, LeftIndexFingerWeights, LeftIndexFingerOrientation, LeftIndexFingerRelativeOrientationArray, bDrawDebugWeightsLeftIndex);
				DPLeftIndexFingerPosition = NewJointPosition(LeftIndexFingerWeights, LeftIndexFingerTransformationArray, bDrawDebugLeftIndexPosition);
				DPLeftIndexFingerOrientation = NewJointOrientation(LeftIndexFingerWeights, LeftIndexFingerRelativeOrientationArray);
				GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Blue, FString::Printf(TEXT("Left Index DP Orientation %s"), *DPLeftIndexFingerOrientation.ToString()));

				/*LeftMiddleFingerWeights.Empty();
				LeftMiddleFingerTransformation.Empty();
				AHands_Character::WeightsComputation(LeftMiddleFingerWeights, LeftMiddleFingerTransformation, LeftMiddleFingerPosition);
				DPLeftMiddleFingerPosition = AHands_Character::NewJointPosition(LeftMiddleFingerWeights, LeftMiddleFingerTransformation, DPVirtualObject);*/
				WeightsComputation(LeftMiddleFingerPosition, LeftMiddleFingerTransformationArray, LeftMiddleFingerWeights, bDrawDebugWeightsLeftMiddle);
				DPLeftMiddleFingerPosition = NewJointPosition(LeftMiddleFingerWeights, LeftMiddleFingerTransformationArray, bDrawDebugLeftMiddlePosition);

				WeightsComputation(LeftRingKnucklePosition, LeftRingKnuckleTransformationArray, LeftRingKnuckleWeights, bDrawDebugWeightsLeftKnuckle);
				DPLeftRingKnucklePosition = NewJointPosition(LeftRingKnuckleWeights, LeftRingKnuckleTransformationArray, bDrawDebugLeftKnucklePosition);

				/*LeftRingFingerWeights.Empty();
				LeftRingFingerTransformation.Empty();
				AHands_Character::WeightsComputation(LeftRingFingerWeights, LeftRingFingerTransformation, LeftRingFingerPosition);
				DPLeftRingFingerPosition = AHands_Character::NewJointPosition(LeftRingFingerWeights, LeftRingFingerTransformation, DPVirtualObject);*/
				WeightsComputation(LeftRingFingerPosition, LeftRingFingerTransformationArray, LeftRingFingerWeights, bDrawDebugWeightsLeftRing);
				DPLeftRingFingerPosition = NewJointPosition(LeftRingFingerWeights, LeftRingFingerTransformationArray, bDrawDebugLeftRingPosition);

				WeightsComputation(LeftPinkyKnucklePosition, LeftPinkyKnuckleTransformationArray, LeftPinkyKnuckleWeights, bDrawDebugWeightsLeftKnuckle);
				DPLeftPinkyKnucklePosition = NewJointPosition(LeftPinkyKnuckleWeights, LeftPinkyKnuckleTransformationArray, bDrawDebugLeftKnucklePosition);

				/*LeftPinkyFingerWeights.Empty();
				LeftPinkyFingerTransformation.Empty();
				AHands_Character::WeightsComputation(LeftPinkyFingerWeights, LeftPinkyFingerTransformation, LeftPinkyFingerPosition);
				DPLeftPinkyFingerPosition = AHands_Character::NewJointPosition(LeftPinkyFingerWeights, LeftPinkyFingerTransformation, DPVirtualObject);*/
				WeightsComputation(LeftPinkyFingerPosition, LeftPinkyFingerTransformationArray, LeftPinkyFingerWeights, bDrawDebugWeightsLeftPinky);
				DPLeftPinkyFingerPosition = NewJointPosition(LeftPinkyFingerWeights, LeftPinkyFingerTransformationArray, bDrawDebugLeftPinkyPosition);

				/*LeftThumbWeights.Empty();
				LeftThumbTransformation.Empty();
				AHands_Character::WeightsComputation(LeftThumbWeights, LeftThumbTransformation, LeftThumbPosition);
				DPLeftThumbPosition = AHands_Character::NewJointPosition(LeftThumbWeights, LeftThumbTransformation, DPVirtualObject);*/
				WeightsComputation(LeftThumbPosition, LeftThumbTransformationArray, LeftThumbWeights, bDrawDebugWeightsLeftThumb);
				DPLeftThumbPosition = NewJointPosition(LeftThumbWeights, LeftThumbTransformationArray, bDrawDebugLeftThumbPosition);

				// Right hand

				/*RightHandWeights.Empty();
				RightHandTransformation.Empty();
				AHands_Character::WeightsComputation(RightHandWeights, RightHandTransformation, RightHandPosition);
				DPRightHandPosition = AHands_Character::NewJointPosition(RightHandWeights, RightHandTransformation, DPVirtualObject);*/
				WeightsComputation(RightHandPosition, RightHandTransformationArray, RightHandWeights, bDrawDebugWeightsRightHand);
				DPRightHandPosition = NewJointPosition(RightHandWeights, RightHandTransformationArray, bDrawDebugRightHandPosition);

				/*RightMiddleKnuckleWeights.Empty();
				RightMiddleKnuckleTransformation.Empty();
				AHands_Character::WeightsComputation(RightMiddleKnuckleWeights, RightMiddleKnuckleTransformation, RightMiddleKnucklePosition);
				DPRightMiddleKnucklePosition = AHands_Character::NewJointPosition(RightMiddleKnuckleWeights, RightMiddleKnuckleTransformation, DPVirtualObject);*/
				WeightsComputation(RightMiddleKnucklePosition, RightMiddleKnuckleTransformationArray, RightMiddleKnuckleWeights, bDrawDebugWeightsRightKnuckle);
				DPRightMiddleKnucklePosition = NewJointPosition(RightMiddleKnuckleWeights, RightMiddleKnuckleTransformationArray, bDrawDebugRightKnucklePosition);

				/*RightIndexFingerWeights.Empty();
				RightIndexFingerTransformation.Empty();
				AHands_Character::WeightsComputation(RightIndexFingerWeights, RightIndexFingerTransformation, RightIndexFingerPosition);
				DPRightIndexFingerPosition = AHands_Character::NewJointPosition(RightIndexFingerWeights, RightIndexFingerTransformation, DPVirtualObject);*/
				WeightsComputation(RightIndexFingerPosition, RightIndexFingerTransformationArray, RightIndexFingerWeights, bDrawDebugWeightsRightIndex);
				DPRightIndexFingerPosition = NewJointPosition(RightIndexFingerWeights, RightIndexFingerTransformationArray, bDrawDebugRightIndexPosition);
				WeightsComputation(RightIndexKnucklePosition, RightIndexKnuckleTransformationArray, RightIndexKnuckleWeights, bDrawDebugWeightsRightKnuckle);
				DPRightIndexKnucklePosition = NewJointPosition(RightIndexKnuckleWeights, RightIndexKnuckleTransformationArray, bDrawDebugRightKnucklePosition);

				/*RightMiddleFingerWeights.Empty();
				RightMiddleFingerTransformation.Empty();
				AHands_Character::WeightsComputation(RightMiddleFingerWeights, RightMiddleFingerTransformation, RightMiddleFingerPosition);
				DPRightMiddleFingerPosition = AHands_Character::NewJointPosition(RightMiddleFingerWeights, RightMiddleFingerTransformation, DPVirtualObject);*/
				WeightsComputation(RightMiddleFingerPosition, RightMiddleFingerTransformationArray, RightMiddleFingerWeights, bDrawDebugWeightsRightMiddle);
				DPRightMiddleFingerPosition = NewJointPosition(RightMiddleFingerWeights, RightMiddleFingerTransformationArray, bDrawDebugRightMiddlePosition);

				/*RightRingFingerWeights.Empty();
				RightRingFingerTransformation.Empty();
				AHands_Character::WeightsComputation(RightRingFingerWeights, RightRingFingerTransformation, RightRingFingerPosition);
				DPRightRingFingerPosition = AHands_Character::NewJointPosition(RightRingFingerWeights, RightRingFingerTransformation, DPVirtualObject);*/
				WeightsComputation(RightRingFingerPosition, RightRingFingerTransformationArray, RightRingFingerWeights, bDrawDebugWeightsRightRing);
				DPRightRingFingerPosition = NewJointPosition(RightRingFingerWeights, RightRingFingerTransformationArray, bDrawDebugRightRingPosition);
				WeightsComputation(RightRingKnucklePosition, RightRingKnuckleTransformationArray, RightRingKnuckleWeights, bDrawDebugWeightsRightKnuckle);
				DPRightRingKnucklePosition = NewJointPosition(RightRingKnuckleWeights, RightRingKnuckleTransformationArray, bDrawDebugRightKnucklePosition);


				/*RightPinkyFingerWeights.Empty();
				RightPinkyFingerTransformation.Empty();
				AHands_Character::WeightsComputation(RightPinkyFingerWeights, RightPinkyFingerTransformation, RightPinkyFingerPosition);
				DPRightPinkyFingerPosition = AHands_Character::NewJointPosition(RightPinkyFingerWeights, RightPinkyFingerTransformation, DPVirtualObject);*/
				WeightsComputation(RightPinkyFingerPosition, RightPinkyFingerTransformationArray, RightPinkyFingerWeights, bDrawDebugWeightsRightPinky);
				DPRightPinkyFingerPosition = NewJointPosition(RightPinkyFingerWeights, RightPinkyFingerTransformationArray, bDrawDebugRightPinkyPosition);
				WeightsComputation(RightPinkyKnucklePosition, RightPinkyKnuckleTransformationArray, RightPinkyKnuckleWeights, bDrawDebugWeightsRightKnuckle);
				DPRightPinkyKnucklePosition = NewJointPosition(RightPinkyKnuckleWeights, RightPinkyKnuckleTransformationArray, bDrawDebugRightKnucklePosition);

				/*RightThumbWeights.Empty();
				RightThumbTransformation.Empty();
				AHands_Character::WeightsComputation(RightThumbWeights, RightThumbTransformation, RightThumbPosition);
				DPRightThumbPosition = AHands_Character::NewJointPosition(RightThumbWeights, RightThumbTransformation, DPVirtualObject);*/
				WeightsComputation(RightThumbPosition, RightThumbTransformationArray, RightThumbWeights, bDrawDebugWeightsRightThumb);
				DPRightThumbPosition = NewJointPosition(RightThumbWeights, RightThumbTransformationArray, bDrawDebugRightThumbPosition);
			}


			// For debug purposes, draw the calculated joint positions
			if (bDrawRightHandPoints)
			{
				DrawDebugPoint(GetWorld(), DPRightHandPosition, 5.0, FColor(255, 255, 0), false, 0.1);
				DrawDebugPoint(GetWorld(), DPRightMiddleKnucklePosition, 2.0, FColor(255, 255, 0), false, 0.1);
				DrawDebugPoint(GetWorld(), DPRightIndexFingerPosition, 5.0, FColor(255, 255, 0), false, 0.1);
				DrawDebugPoint(GetWorld(), DPRightMiddleFingerPosition, 5.0, FColor(255, 255, 0), false, 0.1);
				DrawDebugPoint(GetWorld(), DPRightRingFingerPosition, 5.0, FColor(255, 255, 0), false, 0.1);
				DrawDebugPoint(GetWorld(), DPRightPinkyFingerPosition, 5.0, FColor(255, 255, 0), false, 0.1);
				DrawDebugPoint(GetWorld(), DPRightThumbPosition, 5.0, FColor(255, 255, 0), false, 0.1);
			}

			if (bDrawLeftHandPoints)
			{
				//UE_LOG(LogTemp, Warning, TEXT("We entered the if statement for DrawLeftHandPoints"));
				DrawDebugPoint(GetWorld(), DPLeftHandPosition, 5.0, FColor(0, 255, 255), false, 0.1);
				DrawDebugPoint(GetWorld(), MyMesh->GetSocketLocation(TEXT("Hand_L")), 5.0, FColor(255, 255, 0), false, 0.1);
				DrawDebugPoint(GetWorld(), DPLeftMiddleKnucklePosition, 5.0, FColor(0, 255, 255), false, 0.1);
				DrawDebugPoint(GetWorld(), MyMesh->GetSocketLocation(TEXT("middle_01_l")), 5.0, FColor(255, 255, 0), false, 0.1);
				DrawDebugPoint(GetWorld(), DPLeftIndexFingerPosition, 5.0, FColor(0, 255, 255), false, 0.1);
				DrawDebugPoint(GetWorld(), MyMesh->GetSocketLocation(TEXT("index_03_l")), 5.0, FColor(255, 255, 0), false, 0.1);
				DrawDebugPoint(GetWorld(), DPLeftMiddleFingerPosition, 5.0, FColor(0, 255, 255), false, 0.1);
				DrawDebugPoint(GetWorld(), MyMesh->GetSocketLocation(TEXT("middle_03_l")), 5.0, FColor(255, 255, 0), false, 0.1);
				DrawDebugPoint(GetWorld(), DPLeftRingFingerPosition, 5.0, FColor(0, 255, 255), false, 0.1);
				DrawDebugPoint(GetWorld(), MyMesh->GetSocketLocation(TEXT("ring_03_l")), 5.0, FColor(255, 255, 0), false, 0.1);
				DrawDebugPoint(GetWorld(), DPLeftPinkyFingerPosition, 5.0, FColor(0, 255, 255), false, 0.1);
				DrawDebugPoint(GetWorld(), MyMesh->GetSocketLocation(TEXT("pinky_03_l")), 5.0, FColor(255, 255, 0), false, 0.1);
				DrawDebugPoint(GetWorld(), DPLeftThumbPosition, 5.0, FColor(0, 255, 255), false, 0.1);
				DrawDebugPoint(GetWorld(), MyMesh->GetSocketLocation(TEXT("thumb_03_l")), 5.0, FColor(255, 255, 0), false, 0.1);
			}

		}
		/*if (bDrawLeftHandPoints)
		{
			DrawDebugSphere(GetWorld(), LeftHandPosition, 3.0, 50, FColor(0, 255, 0), false, 0.05);
			DrawDebugSphere(GetWorld(), LeftMiddleKnucklePosition, 1.f, 50, FColor(0, 255, 0), false, 0.05);
			DrawDebugPoint(GetWorld(), LeftIndexFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
			DrawDebugPoint(GetWorld(), LeftMiddleFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
			DrawDebugPoint(GetWorld(), LeftRingFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
			DrawDebugPoint(GetWorld(), LeftPinkyFingerPosition, 5.0, FColor(0, 255, 0), false, 0.05);
			DrawDebugPoint(GetWorld(), LeftThumbPosition, 5.0, FColor(0, 255, 0), false, 0.05);
		}*/
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
	InputComponent->BindAction("DecisionObject1", IE_Pressed, this, &AHands_Character::Answer1);
	InputComponent->BindAction("DecisionObject2", IE_Pressed, this, &AHands_Character::Answer2);
	InputComponent->BindAction("DecisionObject3", IE_Pressed, this, &AHands_Character::Answer3);
	InputComponent->BindAction("DecisionObject4", IE_Pressed, this, &AHands_Character::Answer4);

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
		FVector RightHandSensorPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightHandPosition, FVector(0.f,0.f,0.f), RightHandOrientation));
		RightHandPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightHandPosition, RightHandSensorOffset, RightHandOrientation));
		if (bDrawRightHandPoints)
		{
			DrawDebugPoint(GetWorld(), RightHandSensorPosition, 5.0, FColor(0, 255, 0), false, 0.1);
			DrawDebugPoint(GetWorld(), RightHandPosition, 5.0, FColor(255, 0, 0), false, 0.1);
		}
	}

	// Set the knuckle position taking into consideration the sensor offset
	FVector RightIndexKnuckleWithOffset = ApplySensorOffset(RectifiedRightHandPosition, RightIndexKnuckleSensorOffset, RightHandOrientation);
	RightIndexKnucklePosition = MyMesh->ComponentToWorld.TransformPosition(RightIndexKnuckleWithOffset);
	RightMiddleKnucklePosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightHandPosition, RightMiddleKnuckleSensorOffset, RightHandOrientation));
	FVector RightRingKnuckleWithOffset = ApplySensorOffset(RectifiedRightHandPosition, RightRingKnuckleSensorOffset, RightHandOrientation);
	RightRingKnucklePosition = MyMesh->ComponentToWorld.TransformPosition(RightRingKnuckleWithOffset);
	FVector RightPinkyKnuckleWithOffset = ApplySensorOffset(RectifiedRightHandPosition, RightPinkyKnuckleSensorOffset, RightHandOrientation);
	RightPinkyKnucklePosition = MyMesh->ComponentToWorld.TransformPosition(RightPinkyKnuckleWithOffset);
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
		FVector RightIndexFingerSensorPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightIndexFingerPosition, FVector(0.f,0.f,0.f), RightIndexFingerOrientation));
		RightIndexFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightIndexFingerPosition, RightIndexFingerSensorOffset, RightIndexFingerOrientation));
		if (bDrawRightHandPoints)
		{
			DrawDebugPoint(GetWorld(), RightIndexFingerSensorPosition, 5.0, FColor(0, 255, 0), false, 0.1);
			DrawDebugPoint(GetWorld(), RightIndexFingerPosition, 5.0, FColor(255, 0, 0), false, 0.1);
		}
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
		FVector RightMiddleFingerSensorPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightMiddleFingerPosition, FVector(0.f,0.f,0.f), RightMiddleFingerOrientation));
		RightMiddleFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightMiddleFingerPosition, RightMiddleFingerSensorOffset, RightMiddleFingerOrientation));
		if (bDrawRightHandPoints)
		{
			DrawDebugPoint(GetWorld(), RightMiddleFingerSensorPosition, 5.0, FColor(0, 255, 0), false, 0.1);
			DrawDebugPoint(GetWorld(), RightMiddleFingerPosition, 5.0, FColor(255, 0, 0), false, 0.1);
		}
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
		FVector RightRingFingerSensorPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightRingFingerPosition, FVector(0.f,0.f,0.f), RightRingFingerOrientation));
		RightRingFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightRingFingerPosition, RightRingFingerSensorOffset, RightRingFingerOrientation));
		if (bDrawRightHandPoints)
		{
			DrawDebugPoint(GetWorld(), RightRingFingerSensorPosition, 5.0, FColor(0, 255, 0), false, 0.1);
			DrawDebugPoint(GetWorld(), RightRingFingerPosition, 5.0, FColor(255, 0, 0), false, 0.1);
		}
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
		FVector RightPinkyFingerSensorPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightPinkyFingerPosition, FVector(0.f,0.f,0.f), RightPinkyFingerOrientation));
		RightPinkyFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightPinkyFingerPosition, RightPinkyFingerSensorOffset, RightPinkyFingerOrientation));
		if (bDrawRightHandPoints)
		{
			DrawDebugPoint(GetWorld(), RightPinkyFingerSensorPosition, 5.0, FColor(0, 255, 0), false, 0.1);
			DrawDebugPoint(GetWorld(), RightPinkyFingerPosition, 5.0, FColor(255, 0, 0), false, 0.1);
		}
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
		FVector RightThumbSensorPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightThumbPosition, FVector(0.f,0.f,0.f), RightThumbOrientation));
		RightThumbPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedRightThumbPosition, RightThumbSensorOffset, RightThumbOrientation));
		if (bDrawRightHandPoints)
		{
			DrawDebugPoint(GetWorld(), RightThumbSensorPosition, 5.0, FColor(0, 255, 0), false, 0.1);
			DrawDebugPoint(GetWorld(), RightThumbPosition, 5.0, FColor(255, 0, 0), false, 0.1);
		}
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
	FVector RectifiedLeftHandPosition = RectifyHandPosition(RawPosition);
	LeftHandPosition = RectifiedLeftHandPosition;
	if (bIsSystemCalibrated)
	{
		FVector LeftHandSensorPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftHandPosition, FVector(0.f,0.f,0.f), LeftHandOrientation));
		LeftHandPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftHandPosition, LeftHandSensorOffset, LeftHandOrientation));
		if (bDrawLeftHandPoints)
		{
			DrawDebugPoint(GetWorld(), LeftHandSensorPosition, 5.0, FColor::Green, false, 0.1);
			DrawDebugPoint(GetWorld(), LeftHandPosition, 5.0, FColor::Red, false, 0.1);			
		}
	}

	// Set the knuckle position taking into consideration the sensor offset
	FVector LeftIndexKnuckleWithOffset = ApplySensorOffset(RectifiedLeftHandPosition, LeftIndexKnuckleSensorOffset, LeftHandOrientation);
	LeftIndexKnucklePosition = MyMesh->ComponentToWorld.TransformPosition(LeftIndexKnuckleWithOffset);
	LeftMiddleKnucklePosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftHandPosition, LeftMiddleKnuckleSensorOffset, LeftHandOrientation));
	FVector LeftRingKnuckleWithOffset = ApplySensorOffset(RectifiedLeftHandPosition, LeftRingKnuckleSensorOffset, LeftHandOrientation);
	LeftRingKnucklePosition = MyMesh->ComponentToWorld.TransformPosition(LeftRingKnuckleWithOffset);
	FVector LeftPinkyKnuckleWithOffset = ApplySensorOffset(RectifiedLeftHandPosition, LeftPinkyKnuckleSensorOffset, LeftHandOrientation);
	LeftPinkyKnucklePosition = MyMesh->ComponentToWorld.TransformPosition(LeftPinkyKnuckleWithOffset);
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
		FVector LeftIndexFingerSensorPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftIndexFingerPosition, FVector(0.f,0.f,0.f), LeftIndexFingerOrientation));
		LeftIndexFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftIndexFingerPosition, LeftIndexFingerSensorOffset, LeftIndexFingerOrientation));
		if (bDrawLeftHandPoints)
		{
			DrawDebugPoint(GetWorld(), LeftIndexFingerSensorPosition, 5.0, FColor(0, 255, 0), false, 0.1);
			DrawDebugPoint(GetWorld(), LeftIndexFingerPosition, 5.0, FColor(255, 0, 0), false, 0.1);
		}
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
		FVector LeftMiddleFingerSensorPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftMiddleFingerPosition, FVector(0.f,0.f,0.f), LeftMiddleFingerOrientation));
		LeftMiddleFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftMiddleFingerPosition, LeftMiddleFingerSensorOffset, LeftMiddleFingerOrientation));
		if (bDrawLeftHandPoints)
		{
			DrawDebugPoint(GetWorld(), LeftMiddleFingerSensorPosition, 5.0, FColor(0, 255, 0), false, 0.1);
			DrawDebugPoint(GetWorld(), LeftMiddleFingerPosition, 5.0, FColor(255, 0, 0), false, 0.1);
		}
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
		FVector LeftRingFingerSensorPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftRingFingerPosition, FVector(0.f,0.f,0.f), LeftRingFingerOrientation));
		LeftRingFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftRingFingerPosition, LeftRingFingerSensorOffset, LeftRingFingerOrientation));
		if (bDrawLeftHandPoints)
		{
			DrawDebugPoint(GetWorld(), LeftRingFingerSensorPosition, 5.0, FColor(0, 255, 0), false, 0.1);
			DrawDebugPoint(GetWorld(), LeftRingFingerPosition, 5.0, FColor(255, 0, 0), false, 0.1);
		}
		
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
		FVector LeftPinkyFingerSensorPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftPinkyFingerPosition, FVector(0.f,0.f,0.f), LeftPinkyFingerOrientation));
		LeftPinkyFingerPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftPinkyFingerPosition, LeftPinkyFingerSensorOffset, LeftPinkyFingerOrientation));
		if (bDrawLeftHandPoints)
		{
			DrawDebugPoint(GetWorld(), LeftPinkyFingerSensorPosition, 5.0, FColor(0, 255, 0), false, 0.1);
			DrawDebugPoint(GetWorld(), LeftPinkyFingerPosition, 5.0, FColor(255, 0, 0), false, 0.1);
		}
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
		FVector LeftThumbSensorPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftThumbPosition, FVector(0.f,0.f,0.f), LeftThumbOrientation));
		LeftThumbPosition = MyMesh->ComponentToWorld.TransformPosition(ApplySensorOffset(RectifiedLeftThumbPosition, LeftThumbSensorOffset, LeftThumbOrientation));
		if (bDrawLeftHandPoints)
		{
			DrawDebugPoint(GetWorld(), LeftThumbSensorPosition, 5.0, FColor(0, 255, 0), false, 0.1);
			DrawDebugPoint(GetWorld(), LeftThumbPosition, 5.0, FColor(255, 0, 0), false, 0.1);
		}
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
			OriginalMesh = SpawnedObject->OurVisibleComponent->GetStaticMesh();
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
			OriginalMesh = SpawnedObject->OurVisibleComponent->GetStaticMesh();
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
			OriginalMesh = SpawnedObject->OurVisibleComponent->GetStaticMesh();
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
				OriginalMesh = SpawnedObject->OurVisibleComponent->GetStaticMesh();
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
		PointerToCurrentMeshVertices = &OriginalMeshVertices;
		PointerToCurrentMeshNormals = &OriginalMeshNormals;
		PointerToCurrentMeshTangents = &OriginalMeshTangents;
		PointerToCurrentMeshBinormals = &OriginalMeshBinormals;
		break;
	case 2:
		PointerToCurrentMeshVertices = &SecondMeshVertices;
		PointerToCurrentMeshNormals = &SecondMeshNormals;
		PointerToCurrentMeshTangents = &SecondMeshTangents;
		PointerToCurrentMeshBinormals = &SecondMeshBinormals;
		break;
	default:
		PointerToCurrentMeshVertices = &OriginalMeshVertices;
		PointerToCurrentMeshNormals = &OriginalMeshNormals;
		PointerToCurrentMeshTangents = &OriginalMeshTangents;
		PointerToCurrentMeshBinormals = &OriginalMeshBinormals;
	}
}

void AHands_Character::GetMeshCurrentTransform(const UStaticMeshComponent* InStaticMesh, FMatrix& CurrentMatrix, FTransform& CurrentTransform, int32& VerticesNum)
{
	UStaticMesh* StaticMesh = InStaticMesh->GetStaticMesh();
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
	UStaticMesh* StaticMesh = InStaticMesh->GetStaticMesh();
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
	/*uint32 limit = pDPIndices->Num();
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
	}*/
	//UE_LOG(LogTemp, Warning, TEXT("Accessed function to draw points"));
	TArray<FVector>& Vertices = *PointerToCurrentMeshVertices;
	TArray<FVector>& Normals = *PointerToCurrentMeshNormals;
	TArray<FVector>& Tangents = *PointerToCurrentMeshTangents;
	TArray<FVector>& Binormals = *PointerToCurrentMeshBinormals;

	TArray<int32>&ICPIndex = DenseCorrespondenceIndices;

	int32 Test_index = (2284/5) * 0;
	int32 upper_limit = Test_index + 1;

	int32 SamplingRate = 5;

	int32 limit;
	if (bHasObjectMeshChanged)
	{
		limit = ICPIndex.Num();
	}
	else
	{
		limit = Vertices.Num();
	}
	for (int32 i = 0; i < limit; i++)
	{
		//int32 i = 500;
		int32 Module = CalculateModule(bSamplePoints, limit , i);
		
		if (Module == 0)
		//if (i >= Test_index && i < upper_limit)
		{

			FVector TransformedVertices;
			FVector TransformedNormals;
			FVector TransformedTangents;
			FVector TransformedBinormals;

			if (bHasObjectMeshChanged)
			{
				int32 NewIndex = ICPIndex[i];

				TransformedVertices = CurrentMeshComponentToWorldTransform.TransformPosition(Vertices[NewIndex]);
				TransformedNormals = CurrentMeshLocalToWorldMatrix.TransformVector(Normals[NewIndex]).GetSafeNormal();
				TransformedTangents = CurrentMeshLocalToWorldMatrix.TransformVector(Tangents[NewIndex]).GetSafeNormal();
				TransformedBinormals = CurrentMeshLocalToWorldMatrix.TransformVector(Binormals[NewIndex]).GetSafeNormal();

				DrawDebugString(GetWorld(), TransformedVertices, FString::FromInt(NewIndex), NULL, FColor::Red, .1f, false);
			}
			else
			{
				TransformedVertices = CurrentMeshComponentToWorldTransform.TransformPosition(Vertices[i]);
				TransformedNormals = CurrentMeshLocalToWorldMatrix.TransformVector(Normals[i]).GetSafeNormal();
				TransformedTangents = CurrentMeshLocalToWorldMatrix.TransformVector(Tangents[i]).GetSafeNormal();
				TransformedBinormals = CurrentMeshLocalToWorldMatrix.TransformVector(Binormals[i]).GetSafeNormal();

				DrawDebugString(GetWorld(), TransformedVertices, FString::FromInt(i), NULL, FColor::Red, .1f, false);
			}	

			DrawDebugLine(GetWorld(), TransformedVertices, TransformedVertices + TransformedNormals * 1.f, FColor(0, 255, 0), false, -1, 0, .1f);
			DrawDebugLine(GetWorld(), TransformedVertices, TransformedVertices + TransformedTangents * 1.f, FColor(255, 0, 0), false, -1, 0, .1f);
			DrawDebugLine(GetWorld(), TransformedVertices, TransformedVertices + TransformedBinormals * 1.f, FColor(0, 0, 255), false, -1, 0, .1f);
		}

		/*if (i == 284)
		{
			FVector TransformedTestingVertices = CurrentMeshComponentToWorldTransform.TransformPosition(ArrayForTestingVertices[i] * 5.f);
			FVector TransformedTestingVertices2 = CurrentMeshComponentToWorldTransform.TransformPosition(ArrayForTestingVertices[557] * 5.f);
			FVector TransformedTestingVertices3 = CurrentMeshComponentToWorldTransform.TransformPosition(ArrayForTestingVertices[719] * 5.f);
			//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("ArrayForTestingVertices[%d] x: %f y: %f z: %f"), i, ArrayForTestingVertices[i].X, ArrayForTestingVertices[i].Y, ArrayForTestingVertices[i].Z));
			//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("TransformedTestingVertices x: %f y: %f z: %f"), TransformedTestingVertices.X, TransformedTestingVertices.Y, TransformedTestingVertices.Z));
			//DrawDebugSphere(GetWorld(), TransformedTestingVertices, 0.2f, 10, FColor(255, 0, 255), false, -1);
			//DrawDebugPoint(GetWorld(), TransformedTestingVertices, 5.0, FColor(0, 255, 255), false, 0.05);

			DrawDebugLine(GetWorld(), TransformedTestingVertices, TransformedTestingVertices2, FColor(0, 255, 0), false, -1, 0, .1f);
			DrawDebugLine(GetWorld(), TransformedTestingVertices2, TransformedTestingVertices3, FColor(0, 255, 0), false, -1, 0, .1f);
			DrawDebugLine(GetWorld(), TransformedTestingVertices, TransformedTestingVertices3, FColor(0, 255, 0), false, -1, 0, .1f);

			FVector TransformedVertices = CurrentMeshComponentToWorldTransform.TransformPosition(Vertices[1289] * 5.f);
			DrawDebugPoint(GetWorld(), TransformedVertices, 5.0, FColor(0, 0, 255), false, -1);

			//DrawDebugLine(GetWorld(), TransformedTestingVertices * 2, TransformedTestingVertices2 * 2, FColor(0, 255, 0), false, -1, 0, 1.1f);
			//DrawDebugLine(GetWorld(), TransformedTestingVertices2 * 2, TransformedTestingVertices3 * 2, FColor(0, 255, 0), false, -1, 0, 1.1f);
			//DrawDebugLine(GetWorld(), TransformedTestingVertices * 2, TransformedTestingVertices3 * 2, FColor(0, 255, 0), false, -1, 0, 1.1f);
		}*/
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
		transformation.Emplace(FVector::DotProduct(p_j - DPInfo[i * 4], DPInfo[(i * 4) + 1]));
		transformation.Emplace(FVector::DotProduct(p_j - DPInfo[i * 4], DPInfo[(i * 4) + 2]));
		transformation.Emplace(FVector::DotProduct(p_j - DPInfo[i * 4], DPInfo[(i * 4) + 3]));		
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

void AHands_Character::WeightsComputation(FVector p_j, TArray<FVector>& TransformationComponents, TArray<float>& w_biprime, bool bDrawDebugPoints)
{
	int32 limit;
	if (bHasObjectSizeChanged)
	{
		UStaticMesh* OriginalMesh = SpawnedObject->OurVisibleComponent->GetStaticMesh();
		FVector ObjectScale = SpawnedObject->OurVisibleComponent->RelativeScale3D;
		SpawnedObject->OurVisibleComponent->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
		//SpawnedObject->OurVisibleComponent->SetMaterial();
		//SpawnedObject->OurVisibleComponent->SetStaticMesh(SecondMesh);
		GetMeshCurrentTransform(SpawnedObject->OurVisibleComponent, OriginalMeshLocalToWorldMatrix, OriginalMeshComponentToWorldTransform, OriginalVerticesNum);
		SpawnedObject->OurVisibleComponent->SetRelativeScale3D(ObjectScale);
		//SpawnedObject->OurVisibleComponent->SetStaticMesh(OriginalMesh);
		// The object is just changing sizes, so the vertices num doesn't change
		limit = OriginalMeshVertices.Num();
	} 
	else if (bHasObjectMeshChanged)
	{
		UStaticMesh* CurrentMesh = SpawnedObject->OurVisibleComponent->GetStaticMesh();
		SpawnedObject->OurVisibleComponent->SetStaticMesh(OriginalMesh);

		//SpawnedObject->OurVisibleComponent->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
		GetMeshCurrentTransform(SpawnedObject->OurVisibleComponent, OriginalMeshLocalToWorldMatrix, OriginalMeshComponentToWorldTransform, OriginalVerticesNum);

		SpawnedObject->OurVisibleComponent->SetStaticMesh(CurrentMesh);
		//SpawnedObject->OurVisibleComponent->SetRelativeScale3D(FVector(1.5f, 1.5f, 1.5f));

		limit = OriginalMeshVertices.Num();
	}
	else
	{
		limit = CurrentVerticesNum;
	}
	
	//// The whole previous block can be allocated inside Tick() instead of having it executed on each function call

	w_biprime.Empty();
	w_biprime.Reserve(limit);
	TransformationComponents.Empty();
	TransformationComponents.Reserve(limit);
	TArray<float> Distance;
	Distance.Reserve(limit);
	TArray<float> w_prime;
	w_prime.Reserve(limit);
	TArray<FVector>& Vertices = OriginalMeshVertices;
	TArray<FVector>& Normals = OriginalMeshNormals;
	TArray<FVector>& Tangents = OriginalMeshTangents;
	TArray<FVector>& Binormals = OriginalMeshBinormals;
	TArray<FVector> ArrayTransformedVertices;

	int32 Test_index = (2284 / 5) * 0;
	int32 upper_limit = Test_index + 1;
	int32 SamplingRate = 5;
	//UE_LOG(LogTemp, Warning, TEXT("OriginalMeshVertices.Num() %d"), limit);
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("OriginalMeshVertices.Num() %d"), limit));
	for (int32 i = 0; i < Vertices.Num(); i++)
	{
		FVector TransformedVertices;
		FVector TransformedNormals;
		FVector TransformedTangents;
		FVector TransformedBinormals;
		float Alpha;
		float Beta;
		float Gamma;
		int32 Module = CalculateModule(bSamplePoints, Vertices.Num(), i);

		if (Module == 0)
		//if (i >= Test_index && i < upper_limit)
		{

			if (!Vertices.IsValidIndex(i))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index while calculating index %d on 'TransformedVertices' on WeightComputation()"), i);
				return;
			}
			TransformedVertices = OriginalMeshComponentToWorldTransform.TransformPosition(Vertices[i]);
			ArrayTransformedVertices.Emplace(TransformedVertices);
			//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("TransformedVertices x: %f y: %f z: %f"), TransformedVertices.X, TransformedVertices.Y, TransformedVertices.Z));

			Distance.Emplace(FVector::Dist(p_j, TransformedVertices));
			if (!Normals.IsValidIndex(i))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index while calculating 'TransformedNormals' on WeightComputation()"));
				return;
			}
			TransformedNormals = OriginalMeshLocalToWorldMatrix.TransformVector(Normals[i]).GetSafeNormal();
			//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("TransformedVertices x: %f y: %f z: %f"), TransformedNormals.X, TransformedNormals.Y, TransformedNormals.Z));

			if (FVector::DotProduct(TransformedNormals, p_j - TransformedVertices) < 0)
			{
				w_prime.Emplace(0);
			}
			else
			{
				float w_prime_val = (FVector::DotProduct(TransformedNormals, p_j - TransformedVertices)) / Distance.Last();
				w_prime.Emplace(w_prime_val);
			}

			Alpha = FVector::DotProduct(p_j - TransformedVertices, TransformedNormals);

			if (!Tangents.IsValidIndex(i))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index while calculating 'TransformedTangents' on WeightComputation()"));
				return;
			}
			TransformedTangents = OriginalMeshLocalToWorldMatrix.TransformVector(Tangents[i]).GetSafeNormal();
			Beta = FVector::DotProduct(p_j - TransformedVertices, TransformedTangents);

			if (!Binormals.IsValidIndex(i))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index while calculating 'TransformedBinormals' on WeightComputation()"));
				return;
			}
			TransformedBinormals = OriginalMeshLocalToWorldMatrix.TransformVector(Binormals[i]).GetSafeNormal();
			Gamma = FVector::DotProduct(p_j - TransformedVertices, TransformedBinormals);

			TransformationComponents.Emplace(FVector(Alpha, Beta, Gamma));

			if (bDrawOriginalMeshPoints)
			{
				//DrawDebugSphere(GetWorld(), TransformedVertices, 0.2f, 10, FColor(255, 255, 0), false, -1);
				DrawDebugString(GetWorld(), TransformedVertices, FString::FromInt(i), NULL, FColor::Blue, 0.1f, false);
				//DrawDebugPoint(GetWorld(), TransformedVertices, 5.f, FColor::Green, false, 0.1f);
				DrawDebugLine(GetWorld(), TransformedVertices, TransformedVertices + TransformedNormals * 1.f, FColor(0, 255, 0), false, -1, 0, .1f);
				DrawDebugLine(GetWorld(), TransformedVertices, TransformedVertices + TransformedTangents * 1.f, FColor(255, 0, 0), false, -1, 0, .1f);
				DrawDebugLine(GetWorld(), TransformedVertices, TransformedVertices + TransformedBinormals * 1.f, FColor(0, 0, 255), false, -1, 0, .1f);
			}
			//FVector TransformedVertices2 = CurrentMeshComponentToWorldTransform.TransformPosition((*PointerToCurrentMeshVertices)[i]);

			//DrawDebugLine(GetWorld(), TransformedVertices2, TransformedVertices, FColor::Cyan, false, -1, 0, .1f);

			//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("UntransformedTangent x: %f y: %f z: %f"),Tangents[i].X, Tangents[i].Y, Tangents[i].Z));
			//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("TransformedTangent x: %f y: %f z: %f"), TransformedTangents.X, TransformedTangents.Y, TransformedTangents.Z));

		}
	}

	float r_j_1 = FMath::Min<float>(Distance);
	float r_j_2 = r_j_1 + (0.25 * 200);
	sum_w_biprime = 0;
	float valor = 0;
	int32 j = 0;
	for (int32 i = 0; i < limit; i++)
	{
		//j = i;
		int32 Module = CalculateModule(bSamplePoints, limit, i, j);
		if (Module == 0)
		//if (i >= Test_index && i < upper_limit)
		{	
			
			if (!Distance.IsValidIndex(j))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index %d for Distance. on AHands_Character::WeightsComputation()"), j);
				return;
			}
			if (r_j_2 <= Distance[j])
			{
				valor = 0;
			}
			else if (r_j_1 < Distance[j] && r_j_2 > Distance[j])
			{
				valor = 1 - (Distance[j] - r_j_1) / (r_j_2 - r_j_1);
			}
			else if (Distance[j] <= r_j_1)
			{
				valor = 1;
			}

			if (!w_prime.IsValidIndex(j))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index %d for w_prime. on AHands_Character::WeightsComputation()"), j);
				return;
			}
			float w_biprime_val = w_prime[j] * valor;
			sum_w_biprime += w_biprime_val;
			w_biprime.Emplace(w_biprime_val);

			j++;
		}
	}
	if (bDrawDebugPoints)
	{

		j = 0;
		float sum_wbiprime = 0;
		//UE_LOG(LogTemp, Warning, TEXT("Limit %d"), limit);
		for (int32 i = 0; i < limit; i++)
		{
			//j = i;
			int32 Module = CalculateModule(bSamplePoints, limit, i, j);
			if (Module == 0)
				//if (i >= Test_index && i < upper_limit)
			{

				if (!w_biprime.IsValidIndex(j))
				{
					UE_LOG(LogTemp, Warning, TEXT("Invalid index %d while calculating 'sum_biprime' on AHands_Character::WeightsComputation()"), j);
					return;
				}
				sum_wbiprime += w_biprime[j];
				j++;
			}
		}

		j = 0;
		for (int32 i = 0; i < limit; i++)
		{
			//j = i;
			int32 Module = CalculateModule(bSamplePoints, limit, i, j);
			if (Module == 0)
				//if (i >= Test_index && i < upper_limit)
			{
				if (!ArrayTransformedVertices.IsValidIndex(j))
				{
					UE_LOG(LogTemp, Warning, TEXT("Invalid index %d for ArrayTransformedVertices on AHands_Character::WeightsComputation()"), j);
					return;
				}
				if (w_biprime[j] != 0)
				{
					FColor colorgradient = FColor::MakeRedToGreenColorFromScalar(w_biprime[j] / sum_wbiprime);
					FVector TransformedVertices = ArrayTransformedVertices[j];
					DrawDebugLine(GetWorld(), TransformedVertices, p_j, colorgradient, false, -1, 0, .1f);
					//UE_LOG(LogTemp, Warning, TEXT("index %d at ArrayTransformedVertices"), j);
				}
				j++;
			}
		}
	}

}

void AHands_Character::WeightsComputation(FVector p_j, TArray<FVector>& TransformationComponents, TArray<float>& w_biprime, FRotator JointOrientation, TArray<FQuat>& RelativeOrientation, bool bDrawDebugPoints)
{
	int32 limit;
	if (bHasObjectSizeChanged)
	{
		UStaticMesh* OriginalMesh = SpawnedObject->OurVisibleComponent->GetStaticMesh();
		FVector ObjectScale = SpawnedObject->OurVisibleComponent->RelativeScale3D;
		SpawnedObject->OurVisibleComponent->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
		//SpawnedObject->OurVisibleComponent->SetMaterial();
		//SpawnedObject->OurVisibleComponent->SetStaticMesh(SecondMesh);
		GetMeshCurrentTransform(SpawnedObject->OurVisibleComponent, OriginalMeshLocalToWorldMatrix, OriginalMeshComponentToWorldTransform, OriginalVerticesNum);
		SpawnedObject->OurVisibleComponent->SetRelativeScale3D(ObjectScale);
		//SpawnedObject->OurVisibleComponent->SetStaticMesh(OriginalMesh);
		// The object is just changing sizes, so the vertices num doesn't change
		limit = OriginalMeshVertices.Num();
	}
	else if (bHasObjectMeshChanged)
	{
		UStaticMesh* CurrentMesh = SpawnedObject->OurVisibleComponent->GetStaticMesh();
		SpawnedObject->OurVisibleComponent->SetStaticMesh(OriginalMesh);

		//SpawnedObject->OurVisibleComponent->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
		GetMeshCurrentTransform(SpawnedObject->OurVisibleComponent, OriginalMeshLocalToWorldMatrix, OriginalMeshComponentToWorldTransform, OriginalVerticesNum);

		SpawnedObject->OurVisibleComponent->SetStaticMesh(CurrentMesh);
		//SpawnedObject->OurVisibleComponent->SetRelativeScale3D(FVector(1.5f, 1.5f, 1.5f));

		limit = OriginalMeshVertices.Num();
	}
	else
	{
		limit = CurrentVerticesNum;
	}

	//// The whole previous block can be allocated inside Tick() instead of having it executed on each function call

	w_biprime.Empty();
	w_biprime.Reserve(limit);
	TransformationComponents.Empty();
	TransformationComponents.Reserve(limit);
	TArray<float> Distance;
	Distance.Reserve(limit);
	TArray<float> w_prime;
	w_prime.Reserve(limit);
	TArray<FVector>& Vertices = OriginalMeshVertices;
	TArray<FVector>& Normals = OriginalMeshNormals;
	TArray<FVector>& Tangents = OriginalMeshTangents;
	TArray<FVector>& Binormals = OriginalMeshBinormals;
	TArray<FVector> ArrayTransformedVertices;

	int32 Test_index = (2284 / 5) * 0;
	int32 upper_limit = Test_index + 1;
	int32 SamplingRate = 5;
	//UE_LOG(LogTemp, Warning, TEXT("OriginalMeshVertices.Num() %d"), limit);
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("OriginalMeshVertices.Num() %d"), limit));
	for (int32 i = 0; i < Vertices.Num(); i++)
	{
		FVector TransformedVertices;
		FVector TransformedNormals;
		FVector TransformedTangents;
		FVector TransformedBinormals;
		float Alpha;
		float Beta;
		float Gamma;
		int32 Module = CalculateModule(bSamplePoints, Vertices.Num(), i);

		if (Module == 0)
			//if (i >= Test_index && i < upper_limit)
		{

			if (!Vertices.IsValidIndex(i))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index while calculating index %d on 'TransformedVertices' on WeightComputation()"), i);
				return;
			}
			TransformedVertices = OriginalMeshComponentToWorldTransform.TransformPosition(Vertices[i]);
			ArrayTransformedVertices.Emplace(TransformedVertices);
			//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("TransformedVertices x: %f y: %f z: %f"), TransformedVertices.X, TransformedVertices.Y, TransformedVertices.Z));

			Distance.Emplace(FVector::Dist(p_j, TransformedVertices));
			if (!Normals.IsValidIndex(i))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index while calculating 'TransformedNormals' on WeightComputation()"));
				return;
			}
			TransformedNormals = OriginalMeshLocalToWorldMatrix.TransformVector(Normals[i]).GetSafeNormal();
			//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("TransformedVertices x: %f y: %f z: %f"), TransformedNormals.X, TransformedNormals.Y, TransformedNormals.Z));

			if (FVector::DotProduct(TransformedNormals, p_j - TransformedVertices) < 0)
			{
				w_prime.Emplace(0);
			}
			else
			{
				float w_prime_val = (FVector::DotProduct(TransformedNormals, p_j - TransformedVertices)) / Distance.Last();
				w_prime.Emplace(w_prime_val);
			}

			Alpha = FVector::DotProduct(p_j - TransformedVertices, TransformedNormals);

			if (!Tangents.IsValidIndex(i))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index while calculating 'TransformedTangents' on WeightComputation()"));
				return;
			}
			TransformedTangents = OriginalMeshLocalToWorldMatrix.TransformVector(Tangents[i]).GetSafeNormal();
			Beta = FVector::DotProduct(p_j - TransformedVertices, TransformedTangents);

			if (!Binormals.IsValidIndex(i))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index while calculating 'TransformedBinormals' on WeightComputation()"));
				return;
			}
			TransformedBinormals = OriginalMeshLocalToWorldMatrix.TransformVector(Binormals[i]).GetSafeNormal();
			Gamma = FVector::DotProduct(p_j - TransformedVertices, TransformedBinormals);

			TransformationComponents.Emplace(FVector(Alpha, Beta, Gamma));

			FQuat VertexOrientationQuat = FMatrix(TransformedTangents, TransformedBinormals, TransformedNormals, FVector::ZeroVector).ToQuat();

			FQuat JointOrientationQuat = JointOrientation.Quaternion();

			RelativeOrientation.Emplace(VertexOrientationQuat.Inverse() * JointOrientationQuat);
			
			//FMatrix JointOrientationMatrix = FMatrix(JointOrientationQuat.GetAxisX(), JointOrientationQuat.GetAxisY(), JointOrientationQuat.GetAxisZ(), FVector::ZeroVector);
			
			//FMatrix VertexOrientationMatrix = FMatrix(TransformedTangents, TransformedBinormals, TransformedNormals, FVector::ZeroVector);

			//RelativeOrientation.Emplace(VertexOrientationMatrix.GetTransposed() * JointOrientationMatrix);

			if (bDrawOriginalMeshPoints)
			{
				//DrawDebugSphere(GetWorld(), TransformedVertices, 0.2f, 10, FColor(255, 255, 0), false, -1);
				DrawDebugString(GetWorld(), TransformedVertices, FString::FromInt(i), NULL, FColor::Blue, 0.1f, false);
				//DrawDebugPoint(GetWorld(), TransformedVertices, 5.f, FColor::Green, false, 0.1f);
				DrawDebugLine(GetWorld(), TransformedVertices, TransformedVertices + TransformedNormals * 1.f, FColor(0, 255, 0), false, -1, 0, .1f);
				DrawDebugLine(GetWorld(), TransformedVertices, TransformedVertices + TransformedTangents * 1.f, FColor(255, 0, 0), false, -1, 0, .1f);
				DrawDebugLine(GetWorld(), TransformedVertices, TransformedVertices + TransformedBinormals * 1.f, FColor(0, 0, 255), false, -1, 0, .1f);
			}
			//FVector TransformedVertices2 = CurrentMeshComponentToWorldTransform.TransformPosition((*PointerToCurrentMeshVertices)[i]);

			//DrawDebugLine(GetWorld(), TransformedVertices2, TransformedVertices, FColor::Cyan, false, -1, 0, .1f);

			//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("UntransformedTangent x: %f y: %f z: %f"),Tangents[i].X, Tangents[i].Y, Tangents[i].Z));
			//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("TransformedTangent x: %f y: %f z: %f"), TransformedTangents.X, TransformedTangents.Y, TransformedTangents.Z));

		}
	}

	float r_j_1 = FMath::Min<float>(Distance);
	float r_j_2 = r_j_1 + (0.25 * 200);
	sum_w_biprime = 0;
	float valor = 0;
	int32 j = 0;
	for (int32 i = 0; i < limit; i++)
	{
		//j = i;
		int32 Module = CalculateModule(bSamplePoints, limit, i, j);
		if (Module == 0)
			//if (i >= Test_index && i < upper_limit)
		{

			if (!Distance.IsValidIndex(j))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index %d for Distance. on AHands_Character::WeightsComputation()"), j);
				return;
			}
			if (r_j_2 <= Distance[j])
			{
				valor = 0;
			}
			else if (r_j_1 < Distance[j] && r_j_2 > Distance[j])
			{
				valor = 1 - (Distance[j] - r_j_1) / (r_j_2 - r_j_1);
			}
			else if (Distance[j] <= r_j_1)
			{
				valor = 1;
			}

			if (!w_prime.IsValidIndex(j))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index %d for w_prime. on AHands_Character::WeightsComputation()"), j);
				return;
			}
			float w_biprime_val = w_prime[j] * valor;
			sum_w_biprime += w_biprime_val;
			w_biprime.Emplace(w_biprime_val);

			j++;
		}
	}
	if (bDrawDebugPoints)
	{

		j = 0;
		float sum_wbiprime = 0;
		//UE_LOG(LogTemp, Warning, TEXT("Limit %d"), limit);
		for (int32 i = 0; i < limit; i++)
		{
			//j = i;
			int32 Module = CalculateModule(bSamplePoints, limit, i, j);
			if (Module == 0)
				//if (i >= Test_index && i < upper_limit)
			{

				if (!w_biprime.IsValidIndex(j))
				{
					UE_LOG(LogTemp, Warning, TEXT("Invalid index %d while calculating 'sum_biprime' on AHands_Character::WeightsComputation()"), j);
					return;
				}
				sum_wbiprime += w_biprime[j];
				j++;
			}
		}

		j = 0;
		for (int32 i = 0; i < limit; i++)
		{
			//j = i;
			int32 Module = CalculateModule(bSamplePoints, limit, i, j);
			if (Module == 0)
				//if (i >= Test_index && i < upper_limit)
			{
				if (!ArrayTransformedVertices.IsValidIndex(j))
				{
					UE_LOG(LogTemp, Warning, TEXT("Invalid index %d for ArrayTransformedVertices on AHands_Character::WeightsComputation()"), j);
					return;
				}
				if (w_biprime[j] != 0)
				{
					FColor colorgradient = FColor::MakeRedToGreenColorFromScalar(w_biprime[j] / sum_wbiprime);
					FVector TransformedVertices = ArrayTransformedVertices[j];
					DrawDebugLine(GetWorld(), TransformedVertices, p_j, colorgradient, false, -1, 0, .1f);
					//UE_LOG(LogTemp, Warning, TEXT("index %d at ArrayTransformedVertices"), j);
				}
				j++;
			}
		}
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

FVector AHands_Character::NewJointPosition(TArray<float>& w_biprime, TArray<FVector>& TransformationComponents, bool bDrawDebugLines)
{
	FVector NewJointPosition(0.f, 0.f, 0.f);
	TArray<FVector>& Vertices = *PointerToCurrentMeshVertices;
	TArray<FVector>& Normals = *PointerToCurrentMeshNormals;
	TArray<FVector>& Tangents = *PointerToCurrentMeshTangents;
	TArray<FVector>& Binormals = *PointerToCurrentMeshBinormals;

	TArray<int32>&ICPIndices = DenseCorrespondenceIndices;

	float sum_wbiprime = 0;
	
	int32 j = 0;

	int32 limit;
	//if (bHasObjectMeshChanged) limit = ICPIndices.Num();
	if (false) limit = ICPIndices.Num();
	else limit = Vertices.Num();
	for (int32 i = 0; i < limit; i++)
	{
		//j = i;
		int32 Module = CalculateModule(bSamplePoints, limit, i, j);
		if (Module == 0)
		//if (i >= Test_index && i < upper_limit)
		{
			
			if (!w_biprime.IsValidIndex(j))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index %d while calculating 'sum_biprime' on AHands_Character::NewJointPosition()"), j);
				return FVector(0.f, 0.f, 0.f);
			}
			sum_wbiprime += w_biprime[j];
			j++;
		}
	}
		j = 0;
	for (int32 i = 0; i < limit; i++)
	{
		//j = i;
		int32 Module = CalculateModule(bSamplePoints, limit, i, j);
		if (Module == 0)
		//if (i >= Test_index && i < upper_limit)
		{
			if (!TransformationComponents.IsValidIndex(j))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index %d for TransformationComponent 'sum_biprime' on AHands_Character::NewJointPosition()"), j);
				return FVector(0.f, 0.f, 0.f);
			}
			float& Alpha = TransformationComponents[j].X;
			float& Beta = TransformationComponents[j].Y;
			float& Gamma = TransformationComponents[j].Z;			
			FVector TransformedVertices;
			FVector TransformedNormals;
			FVector TransformedTangents;
			FVector TransformedBinormals;
			
			if (false)
			//if (bHasObjectMeshChanged)
			{
				int32 IndexToUse = ICPIndices[i];
				
				if (!Vertices.IsValidIndex(IndexToUse))
				{
					UE_LOG(LogTemp, Warning, TEXT("Invalid index %d for Vertices, iteration %d. on AHands_Character::NewJointPosition()"), IndexToUse, i);
					return FVector(0.f, 0.f, 0.f);
				}
				
				TransformedVertices = CurrentMeshComponentToWorldTransform.TransformPosition(Vertices[IndexToUse]);
				TransformedNormals = CurrentMeshLocalToWorldMatrix.TransformVector(Normals[IndexToUse]).GetSafeNormal();
				TransformedTangents = CurrentMeshLocalToWorldMatrix.TransformVector(Tangents[IndexToUse]).GetSafeNormal();
				TransformedBinormals = CurrentMeshLocalToWorldMatrix.TransformVector(Binormals[IndexToUse]).GetSafeNormal();
			}
			else
			{
				TransformedVertices = CurrentMeshComponentToWorldTransform.TransformPosition(Vertices[i]);
				TransformedNormals = CurrentMeshLocalToWorldMatrix.TransformVector(Normals[i]).GetSafeNormal();
				TransformedTangents = CurrentMeshLocalToWorldMatrix.TransformVector(Tangents[i]).GetSafeNormal();
				TransformedBinormals = CurrentMeshLocalToWorldMatrix.TransformVector(Binormals[i]).GetSafeNormal();
			}
				NewJointPosition += (w_biprime[j] / sum_wbiprime) * (TransformedVertices + (Alpha * TransformedNormals) + (Beta * TransformedTangents) + (Gamma * TransformedBinormals));
				//GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("Current Mesh TransformedNormals x: %f y: %f z: %f"), TransformedNormals.X, TransformedNormals.Y, TransformedNormals.Z));
			if (bDrawDebugLines)
			{
				if (w_biprime[j] != 0)
				{
					FVector CurrentVector = (TransformedVertices + (Alpha * TransformedNormals) + (Beta * TransformedTangents) + (Gamma * TransformedBinormals));
					FColor colorgradient = FColor::MakeRedToGreenColorFromScalar(w_biprime[j] / sum_wbiprime);
					DrawDebugLine(GetWorld(), TransformedVertices, CurrentVector, colorgradient, false, -1, 0, .1f);
				}
			}
			j++;
		}

	}
	return NewJointPosition;

}

FRotator AHands_Character::NewJointOrientation(TArray<float>& WeightsArray, TArray<FQuat>&RelativeOrientation)
{
	TArray<FVector>& Vertices = *PointerToCurrentMeshVertices;
	TArray<FVector>& Normals = *PointerToCurrentMeshNormals;
	TArray<FVector>& Tangents = *PointerToCurrentMeshTangents;
	TArray<FVector>& Binormals = *PointerToCurrentMeshBinormals;

	int32 limit;

	TArray<int32>&ICPIndices = DenseCorrespondenceIndices;

	if (bHasObjectMeshChanged) limit = ICPIndices.Num();
	else limit = Vertices.Num();
	
	float sum_wbiprime = 0;
	int32 j = 0;
	for (int32 i = 0; i < limit; i++)
	{
		//j = i;
		int32 Module = CalculateModule(bSamplePoints, limit, i, j);
		if (Module == 0)
			//if (i >= Test_index && i < upper_limit)
		{

			if (!WeightsArray.IsValidIndex(j))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index %d while calculating 'sum_biprime' on AHands_Character::NewJointOrientation()"), j);
				return FRotator(0.f, 0.f, 0.f);
			}
			sum_wbiprime += WeightsArray[j];
			j++;
		}
	}

	FVector SumAxis(0.f);
	float SumAngle = 0;
	j = 0;
	for (int32 i = 0; i < limit; i++)
	{
		int32 Module = CalculateModule(bSamplePoints, limit, i, j);
		if (Module == 0)
			//if (i >= Test_index && i < upper_limit)
		{
			if (!RelativeOrientation.IsValidIndex(j))
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index %d for RelativeOrientation on AHands_Character::NewJointOrientation()"), j);
				return FRotator(0.f, 0.f, 0.f);
			}
			
			FVector TransformedVertices;
			FVector TransformedNormals;
			FVector TransformedTangents;
			FVector TransformedBinormals;
			if(false)
			//if (bHasObjectMeshChanged)
			{
				int32 IndexToUse = ICPIndices[i];

				if (!Vertices.IsValidIndex(IndexToUse))
				{
					UE_LOG(LogTemp, Warning, TEXT("Invalid index %d for Vertices, iteration %d. on AHands_Character::NewJointOrientation()"), IndexToUse, i);
					return FRotator(0.f, 0.f, 0.f);
				}

				TransformedVertices = CurrentMeshComponentToWorldTransform.TransformPosition(Vertices[IndexToUse]);
				TransformedNormals = CurrentMeshLocalToWorldMatrix.TransformVector(Normals[IndexToUse]).GetSafeNormal();
				TransformedTangents = CurrentMeshLocalToWorldMatrix.TransformVector(Tangents[IndexToUse]).GetSafeNormal();
				TransformedBinormals = CurrentMeshLocalToWorldMatrix.TransformVector(Binormals[IndexToUse]).GetSafeNormal();
			}
			else
			{
				TransformedVertices = CurrentMeshComponentToWorldTransform.TransformPosition(Vertices[i]);
				TransformedNormals = CurrentMeshLocalToWorldMatrix.TransformVector(Normals[i]).GetSafeNormal();
				TransformedTangents = CurrentMeshLocalToWorldMatrix.TransformVector(Tangents[i]).GetSafeNormal();
				TransformedBinormals = CurrentMeshLocalToWorldMatrix.TransformVector(Binormals[i]).GetSafeNormal();
			}

			FQuat VertexOrientation = FMatrix(TransformedTangents, TransformedBinormals, TransformedNormals, FVector::ZeroVector).ToQuat();

			FQuat NewOrientation = VertexOrientation *RelativeOrientation[j];

			FVector Axis;
			float Angle;
			NewOrientation.ToAxisAndAngle(Axis, Angle);

			SumAxis += (WeightsArray[j] / sum_wbiprime) * Axis;
			SumAngle += (WeightsArray[j] / sum_wbiprime) * Angle;
		}
		j++;
	}
	return FQuat(SumAxis, FMath::DegreesToRadians(SumAngle)).Rotator();
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
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
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
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
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
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
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
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
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
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
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
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
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
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
	{
		return DPLeftIndexFingerOrientation;
	}
	else
	{
		return LeftIndexFingerOrientation;
	}
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

FVector AHands_Character::GetLeftIndexKnucklePosition()
{
	//if (false)
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
	{
		return DPLeftIndexKnucklePosition;
	}
	else
	{
		return LeftIndexKnucklePosition;
	}
}

FVector AHands_Character::GetLeftMiddleKnucklePosition()
{
	//if (false)
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
	{
		return DPLeftMiddleKnucklePosition;
	}
	else
	{
		return LeftMiddleKnucklePosition;
	}
}

FVector AHands_Character::GetLeftRingKnucklePosition()
{
	//if (false)
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
	{
		return DPLeftRingKnucklePosition;
	}
	else
	{
		return LeftRingKnucklePosition;
	}
}

FVector AHands_Character::GetLeftPinkyKnucklePosition()
{
	//if (false)
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
	{
		return DPLeftPinkyKnucklePosition;
	}
	else
	{
		return LeftPinkyKnucklePosition;
	}
}

// Functions to access the right hand and fingers P & O
FVector AHands_Character::GetRightHandPosition()
{
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
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
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
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
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
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
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
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
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
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
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
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

FVector AHands_Character::GetRightIndexKnucklePosition()
{
	//if (false)
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
	{
		return DPRightIndexKnucklePosition;
	}
	else
	{
		return RightIndexKnucklePosition;
	}
}

FVector AHands_Character::GetRightMiddleKnucklePosition()
{
	//if (false)
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
	{
		return DPRightMiddleKnucklePosition;
	}
	else
	{
		return RightMiddleKnucklePosition;
	}
}

FVector AHands_Character::GetRightRingKnucklePosition()
{
	//if (false)
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
	{
		return DPRightRingKnucklePosition;
	}
	else
	{
		return RightRingKnucklePosition;
	}
}

FVector AHands_Character::GetRightPinkyKnucklePosition()
{
	//if (false)
	if (SpawnedObject && bAreDPsActive && (bHasObjectSizeChanged || bHasObjectMeshChanged))
	{
		return DPRightPinkyKnucklePosition;
	}
	else
	{
		return RightPinkyKnucklePosition;
	}
}

int32 AHands_Character::CalculateModule(bool bSamplePoints, int32 VerticesNum, int32 CurrentIndex)
{
	if (bSamplePoints)
	{
		return CurrentIndex % (VerticesNum / NumberSamplingPoints);
	}
	else
	{
		return 0;
	}
}

int32 AHands_Character::CalculateModule(bool bSamplePoints, int32 VerticesNum, int32 CurrentIndex, int32& SampledIndex)
{
	if (bSamplePoints)
	{
		return CurrentIndex % (VerticesNum / NumberSamplingPoints);
	}
	else
	{
		SampledIndex = CurrentIndex;
		return 0;
	}
}