// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "Hands_Character.generated.h"

USTRUCT()
struct FArrayForStoringIndices
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<int32> IndicesArray;

	/*void AddNewArray()
	{
		IndicesArray.Add(NULL);
	}*/

	FArrayForStoringIndices()
	{
	}

};

UCLASS()
class HAND_ANIMATION_API AHands_Character : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
public:
	// Sets default values for this character's properties
	AHands_Character();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;
	
protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** Right hand & fingers movement functions */
	void RightHandMovement(float Value);
	void RightIndexFingerMovement(float Value);
	void RightMiddleFingerMovement(float Value);
	void RightRingFingerMovement(float Value);
	void RightPinkyFingerMovement(float Value);
	void RightThumbMovement(float Value);

	/** Left hand & fingers movement functions */
	void LeftHandMovement(float Value);
	void LeftIndexFingerMovement(float Value);
	void LeftMiddleFingerMovement(float Value);
	void LeftRingFingerMovement(float Value);
	void LeftPinkyFingerMovement(float Value);
	void LeftThumbMovement(float Value);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

public:

	/* What object to spawn */
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<class AInteractionObject> ObjectToSpawn1;

	/* What object to spawn */
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<class AInteractionObject> ObjectToSpawn2;

	/* What object to spawn */
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<class AInteractionObject> ObjectToSpawn3;

	/* What object to spawn */
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<class AInteractionObject> ObjectToSpawn4;

protected:
	
	/** Has object 1 been spawned? */
	bool bIsObject1Spawned;

	/** Has object 2 been spawned? */
	bool bIsObject2Spawned;

	/** Has object 3 been spawned? */
	bool bIsObject3Spawned;

	/** Has object 4 been spawned? */
	bool bIsObject4Spawned;
			
	/** Object P & O*/
	void Object1Movement(float Value);

	void Object2Movement(float Value);

	void Object3Movement(float Value);

	void Object4Movement(float Value);

	void ModifyObjectSize();

	void ResetObjectSize();

	UStaticMesh* OriginalMesh;

public:

	TArray<FVector> OriginalMeshVertices;
	TArray<FVector> OriginalMeshNormals;
	TArray<FVector> OriginalMeshTangents;
	TArray<FVector> OriginalMeshBinormals;
	TArray<int32> OriginalMeshIndices;

	TArray<FVector> SecondMeshVertices;
	TArray<FVector> SecondMeshNormals;
	TArray<FVector> SecondMeshTangents;
	TArray<FVector> SecondMeshBinormals;
	TArray<int32> SecondMeshIndices;

	//TArray<FVector> ArrayForTestingVertices;

	TArray<int32> DenseCorrespondenceIndices;
	
	TArray<FArrayForStoringIndices> Mesh2Mesh1Correspondences;

protected:

	//USkeletalMeshComponent* MyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector MeshScale;
	
	/** Array storing the position, normal, tangent and binormal of each descriptor point */
	TArray<FVector> DescriptorPointInfo;
	TArray<FVector> DPOriginalObject;
	TArray<FVector> DPVirtualObject;
	TArray<uint32> VertexIndices;
	
	TArray<FVector>* PointerToOriginalMeshVertices;
	TArray<FVector>* PointerToOriginalMeshNormals;
	TArray<FVector>* PointerToOriginalMeshTangents;
	TArray<FVector>* PointerToOriginalMeshBinormals;

	TArray<FVector>* PointerToCurrentMeshVertices;
	TArray<FVector>* PointerToCurrentMeshNormals;
	TArray<FVector>* PointerToCurrentMeshTangents;
	TArray<FVector>* PointerToCurrentMeshBinormals;

	TArray<float>* WeightsPointer;
	TArray<float>* TransformationPointer;
	TArray<FVector>* DescriptorPointsPointer;

	TArray<float> LeftHandWeights;
	TArray<float> LeftIndexFingerWeights;
	TArray<float> LeftMiddleFingerWeights;
	TArray<float> LeftRingFingerWeights;
	TArray<float> LeftPinkyFingerWeights;
	TArray<float> LeftThumbWeights;

	TArray<float> LeftIndexKnuckleWeights;
	TArray<float> LeftMiddleKnuckleWeights;
	TArray<float> LeftRingKnuckleWeights;
	TArray<float> LeftPinkyKnuckleWeights;

	TArray<float> RightHandWeights;
	TArray<float> RightIndexFingerWeights;
	TArray<float> RightMiddleFingerWeights;
	TArray<float> RightRingFingerWeights;
	TArray<float> RightPinkyFingerWeights;
	TArray<float> RightThumbWeights;

	TArray<float> RightIndexKnuckleWeights;
	TArray<float> RightMiddleKnuckleWeights;
	TArray<float> RightRingKnuckleWeights;
	TArray<float> RightPinkyKnuckleWeights;

	TArray<float> LeftHandTransformation;
	TArray<float> LeftIndexFingerTransformation;
	TArray<float> LeftMiddleFingerTransformation;
	TArray<float> LeftRingFingerTransformation;
	TArray<float> LeftPinkyFingerTransformation;
	TArray<float> LeftThumbTransformation;

	TArray<float> LeftMiddleKnuckleTransformation;

	TArray<float> RightHandTransformation;
	TArray<float> RightIndexFingerTransformation;
	TArray<float> RightMiddleFingerTransformation;
	TArray<float> RightRingFingerTransformation;
	TArray<float> RightPinkyFingerTransformation;
	TArray<float> RightThumbTransformation;

	TArray<float> RightMiddleKnuckleTransformation;

	TArray<FVector> LeftHandTransformationArray;
	TArray<FVector> LeftIndexFingerTransformationArray;
	TArray<FVector> LeftMiddleFingerTransformationArray;
	TArray<FVector> LeftRingFingerTransformationArray;
	TArray<FVector> LeftPinkyFingerTransformationArray;
	TArray<FVector> LeftThumbTransformationArray;

	TArray<FMatrix> LeftHandRelativeOrientationArray;
	TArray<FMatrix> LeftIndexFingerRelativeOrientationArray;
	TArray<FMatrix> LeftMiddleFingerRelativeOrientationArray;
	TArray<FMatrix> LeftRingFingerRelativeOrientationArray;
	TArray<FMatrix> LeftPinkyFingerRelativeOrientationArray;
	TArray<FMatrix> LeftThumbRelativeOrientationArray;

	TArray<FVector> LeftIndexKnuckleTransformationArray;
	TArray<FVector> LeftMiddleKnuckleTransformationArray;
	TArray<FVector> LeftRingKnuckleTransformationArray;
	TArray<FVector> LeftPinkyKnuckleTransformationArray;

	TArray<FVector> RightHandTransformationArray;
	TArray<FVector> RightIndexFingerTransformationArray;
	TArray<FVector> RightMiddleFingerTransformationArray;
	TArray<FVector> RightRingFingerTransformationArray;
	TArray<FVector> RightPinkyFingerTransformationArray;
	TArray<FVector> RightThumbTransformationArray;

	TArray<FVector> RightIndexKnuckleTransformationArray;
	TArray<FVector> RightMiddleKnuckleTransformationArray;
	TArray<FVector> RightRingKnuckleTransformationArray;
	TArray<FVector> RightPinkyKnuckleTransformationArray;
	
	/** Number of descriptor points */
	UPROPERTY(EditAnywhere, Category = "Vertices")
	uint32 NumDescriptorPoints;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawCurrentMeshPoints;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawOriginalMeshPoints;

	// Green points represent sensor location, red points represent their offset position
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawRightHandPoints;

	// Green points represent sensor location, red points represent their offset position
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawLeftHandPoints;

	// Left hand & fingers weights
	UPROPERTY(EditAnywhere, Category = "Debug|Left Hand")
	bool bDrawDebugWeightsLeftHand;
	UPROPERTY(EditAnywhere, Category = "Debug|Left Hand")
	bool bDrawDebugWeightsLeftKnuckle;
	UPROPERTY(EditAnywhere, Category = "Debug|Left Hand")
	bool bDrawDebugWeightsLeftIndex;
	UPROPERTY(EditAnywhere, Category = "Debug|Left Hand")
	bool bDrawDebugWeightsLeftMiddle;
	UPROPERTY(EditAnywhere, Category = "Debug|Left Hand")
	bool bDrawDebugWeightsLeftRing;
	UPROPERTY(EditAnywhere, Category = "Debug|Left Hand")
	bool bDrawDebugWeightsLeftPinky;
	UPROPERTY(EditAnywhere, Category = "Debug|Left Hand")
	bool bDrawDebugWeightsLeftThumb;
	
	// Right hand & fingers weights
	UPROPERTY(EditAnywhere, Category = "Debug|Right Hand")
	bool bDrawDebugWeightsRightHand;
	UPROPERTY(EditAnywhere, Category = "Debug|Right Hand")
	bool bDrawDebugWeightsRightKnuckle;
	UPROPERTY(EditAnywhere, Category = "Debug|Right Hand")
	bool bDrawDebugWeightsRightIndex;
	UPROPERTY(EditAnywhere, Category = "Debug|Right Hand")
	bool bDrawDebugWeightsRightMiddle;
	UPROPERTY(EditAnywhere, Category = "Debug|Right Hand")
	bool bDrawDebugWeightsRightRing;
	UPROPERTY(EditAnywhere, Category = "Debug|Right Hand")
	bool bDrawDebugWeightsRightPinky;
	UPROPERTY(EditAnywhere, Category = "Debug|Right Hand")
	bool bDrawDebugWeightsRightThumb;

	// Left hand & fingers position vectors
	UPROPERTY(EditAnywhere, Category = "Debug|Left Hand")
	bool bDrawDebugLeftHandPosition;
	UPROPERTY(EditAnywhere, Category = "Debug|Left Hand")
	bool bDrawDebugLeftKnucklePosition;
	UPROPERTY(EditAnywhere, Category = "Debug|Left Hand")
	bool bDrawDebugLeftIndexPosition;
	UPROPERTY(EditAnywhere, Category = "Debug|Left Hand")
	bool bDrawDebugLeftMiddlePosition;
	UPROPERTY(EditAnywhere, Category = "Debug|Left Hand")
	bool bDrawDebugLeftRingPosition;
	UPROPERTY(EditAnywhere, Category = "Debug|Left Hand")
	bool bDrawDebugLeftPinkyPosition;
	UPROPERTY(EditAnywhere, Category = "Debug|Left Hand")
	bool bDrawDebugLeftThumbPosition;

	// Right hand & fingers position vectors
	UPROPERTY(EditAnywhere, Category = "Debug|Right Hand")
	bool bDrawDebugRightHandPosition;
	UPROPERTY(EditAnywhere, Category = "Debug|Right Hand")
	bool bDrawDebugRightKnucklePosition;
	UPROPERTY(EditAnywhere, Category = "Debug|Right Hand")
	bool bDrawDebugRightIndexPosition;
	UPROPERTY(EditAnywhere, Category = "Debug|Right Hand")
	bool bDrawDebugRightMiddlePosition;
	UPROPERTY(EditAnywhere, Category = "Debug|Right Hand")
	bool bDrawDebugRightRingPosition;
	UPROPERTY(EditAnywhere, Category = "Debug|Right Hand")
	bool bDrawDebugRightPinkyPosition;
	UPROPERTY(EditAnywhere, Category = "Debug|Right Hand")
	bool bDrawDebugRightThumbPosition;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bSamplePoints;

	UPROPERTY(EditAnywhere, Category = "Debug")
	int32 NumberSamplingPoints;

	UPROPERTY(EditAnywhere, Category = "Debug")
	UStaticMesh* SecondMesh;

	//bool bAreDPset;
	TArray<uint32>* pDPIndices;

	/** Vertices indices for the descriptor points */
	TArray<uint32> DPIndices;

	float sum_w_biprime;

protected:
	
	int32 Index1;

	void GetMeshCurrentTransform(const UStaticMeshComponent* InStaticMeshComponent, FMatrix& TransformationMatrix, FTransform& CurrentTranform, int32& VerticesNum);
	
	//Function for accesing the triangles information
	void AccessTriVertices(const UStaticMeshComponent* InStaticMeshComponent, TArray<FVector>& DescriptorPointsArray);

	//Random initial descriptor points
	void InitDescriptionPoints(uint32 NumDP, uint32 NumVertices);

	//Draw the descriptor points
	void DrawDescriptionPoints(TArray<FVector>& DescriptorPointsArray);

	//Weights calculation
	void WeightsComputation(TArray<float>& WeightsArray, TArray<float>& TransformationArray, FVector p_j);

	void WeightsComputation(FVector p_j, TArray<FVector>& TransformationComponents, TArray<float>& WeightsArray, bool bDrawDebugPoints);

	void WeightsComputation(FVector p_j, TArray<FVector>& TransformationComponents, TArray<float>& WeightsArray, FRotator JointOrientation, TArray<FMatrix>& OrientationArrays, bool bDrawDebugPoints);

	void AssignPointers();

	/** Calculate a new joint position with the descriptor points weights */
	FVector NewJointPosition(TArray<float>& WeightsArray, TArray<float>& TransformationArray, TArray<FVector>& DescriptorPointsArray);

	FVector NewJointPosition(TArray<float>& WeightsArray, TArray<FVector>& TransformationArray, bool bDrawDebugLines);

	FRotator NewJointOrientation(TArray<float>& WeightsArray, TArray<FMatrix>& RelativeOrientation);

	FTransform NewJointTransform(TArray<float>& WeightsArray, TArray<FVector>& TransformationArray, TArray<FMatrix>& RelativeOrientation);

protected:

	FMatrix CurrentMeshLocalToWorldMatrix;
	FTransform CurrentMeshComponentToWorldTransform;

	FMatrix OriginalMeshLocalToWorldMatrix;
	FTransform OriginalMeshComponentToWorldTransform;

	int32 CurrentVerticesNum;
	int32 OriginalVerticesNum;

	UPROPERTY(EditAnywhere, Category = "Hand", meta = (BlueprintProtected = "true"))
	bool bAreDPsActive;

	/*UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugPoints;*/

	// Right hand & fingers
	/** Right hand P & O */
	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector RightHandPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPRightHandPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator RightHandOrientation;

	/** Right Index finger P & O */
	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector RightIndexFingerPosition;
	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPRightIndexFingerPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator RightIndexFingerOrientation;

	/** Right Middle finger P & O */
	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector RightMiddleFingerPosition;
	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPRightMiddleFingerPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator RightMiddleFingerOrientation;

	/** Right ring finger P & O */
	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector RightRingFingerPosition;
	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPRightRingFingerPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator RightRingFingerOrientation;

	/** Right pinky P & O */
	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector RightPinkyFingerPosition;
	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPRightPinkyFingerPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator RightPinkyFingerOrientation;

	/** Right thumb P & O */
	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector RightThumbPosition;
	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPRightThumbPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator RightThumbOrientation;
	//

	//Left hand & fingers
	/** Left hand 1 P & O */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector LeftHandPosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPLeftHandPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator LeftHandOrientation;
	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator DPLeftHandOrientation;
		
	/** Left Index finger P & O */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector LeftIndexFingerPosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPLeftIndexFingerPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator LeftIndexFingerOrientation;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator DPLeftIndexFingerOrientation;

	/** Left Middle finger P & O */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector LeftMiddleFingerPosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPLeftMiddleFingerPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator LeftMiddleFingerOrientation;
	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator DPLeftMiddleFingerOrientation;

	/** Left ring finger P & O */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector LeftRingFingerPosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPLeftRingFingerPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator LeftRingFingerOrientation;
	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator DPLeftRingFingerOrientation;

	/** Left pinky P & O */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector LeftPinkyFingerPosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPLeftPinkyFingerPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator LeftPinkyFingerOrientation;
	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator DPLeftPinkyFingerOrientation;

	/** Left thumb P & O */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector LeftThumbPosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPLeftThumbPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator LeftThumbOrientation;
	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator DPLeftThumbOrientation;
	//
	
	// Left hand knuckles
	
	/* Middle finger knuckle P & O*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector LeftIndexKnucklePosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPLeftIndexKnucklePosition;


	/* Middle finger knuckle P & O*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector LeftMiddleKnucklePosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPLeftMiddleKnucklePosition;

	/* Ring finger knuckle P & O*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector LeftRingKnucklePosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPLeftRingKnucklePosition;

	/* Pinky finger knuckle P & O*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector LeftPinkyKnucklePosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPLeftPinkyKnucklePosition;

	// Right hand knuckles

	/* Middle finger knuckle P & O*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector RightIndexKnucklePosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPRightIndexKnucklePosition;

	/* Middle Index finger knuckle P & O*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector RightMiddleKnucklePosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPRightMiddleKnucklePosition;

	/* Ring finger knuckle P & O*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector RightRingKnucklePosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPRightRingKnucklePosition;

	/* Pinky finger knuckle P & O*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector RightPinkyKnucklePosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPRightPinkyKnucklePosition;

protected:

	/** Transformation to account for our location from the magnetic source */
	UPROPERTY(BlueprintReadWrite, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector AxisTranslation;

	/** Is the system calibrated? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	bool bIsSystemCalibrated;

	/** Aplha value to allow bone transformation, hence activating the hand and fingers motion */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	float AlphaValue;

	/** Position of the left hand sensor measured from the skeletal mesh hand bone position */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector LeftHandSensorOffset;	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector LeftIndexFingerSensorOffset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector LeftMiddleFingerSensorOffset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector LeftRingFingerSensorOffset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector LeftPinkyFingerSensorOffset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector LeftThumbSensorOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector LeftMiddleKnuckleSensorOffset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector LeftIndexKnuckleSensorOffset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector LeftRingKnuckleSensorOffset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector LeftPinkyKnuckleSensorOffset;
	
	/** Position of the left hand sensor measured from the skeletal mesh hand bone position */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector RightHandSensorOffset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector RightIndexFingerSensorOffset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector RightMiddleFingerSensorOffset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector RightRingFingerSensorOffset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector RightPinkyFingerSensorOffset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector RightThumbSensorOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector RightIndexKnuckleSensorOffset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector RightMiddleKnuckleSensorOffset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector RightRingKnuckleSensorOffset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector RightPinkyKnuckleSensorOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = (BlueprintProtected = "true"))
	float SensorDelayRangeLow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = (BlueprintProtected = "true"))
	float SensorDelayRangeHigh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = (BlueprintProtected = "true"))
	bool bIsDelayActive;

	void Answer1();

	void Answer2();
	
	void Answer3();

	void Answer4();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	USkeletalMeshComponent* MyMesh;

	/* The spawned object */
	AInteractionObject* SpawnedObject;

	// Calibration completed
	void CalibrateSystem(FVector AxisTranslation);

	void SetAlphaValue(float AlphaValue);

	void ExperimentSetup(bool bIsSynchronous, bool bIsExperimentForDP);

	UFUNCTION(BlueprintPure, Category = "Experiment")
	bool GetDelayState();

	/** Object spawning */
	void SpawnObject1();

	/** Object spawning */
	void SpawnObject2();
	
	/** Object spawning */
	void SpawnObject3();
	
	/** Object spawning */
	void SpawnObject4();

	void CheckAnswer();

	bool bAreDPset;

	bool bHasObjectSizeChanged;

	bool bHasObjectMeshChanged;

	bool bIsExperimentFinished;

	bool bIsDecisionMade;

	int32 ObjectChosen;

	int32 CurrentMeshIdentificator;
	
	// 
	UFUNCTION(BlueprintPure, Category = "Hand")
	float GetAlphaValue();

	/** Functions to obtain left hand and fingersP & O */
	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetLeftHandPosition();
	UFUNCTION(BlueprintPure, Category = "Hand")
	FRotator GetLeftHandOrientation();

	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetLeftIndexFingerPosition();
	UFUNCTION(BlueprintPure, Category = "Hand")
	FRotator GetLeftIndexFingerOrientation();

	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetLeftMiddleFingerPosition();
	UFUNCTION(BlueprintPure, Category = "Hand")
	FRotator GetLeftMiddleFingerOrientation();

	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetLeftRingFingerPosition();
	UFUNCTION(BlueprintPure, Category = "Hand")
	FRotator GetLeftRingFingerOrientation();

	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetLeftPinkyFingerPosition();
	UFUNCTION(BlueprintPure, Category = "Hand")
	FRotator GetLeftPinkyFingerOrientation();
	
	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetLeftThumbPosition();
	UFUNCTION(BlueprintPure, Category = "Hand")
	FRotator GetLeftThumbOrientation();

	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetLeftIndexKnucklePosition();

	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetLeftMiddleKnucklePosition();

	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetLeftRingKnucklePosition();

	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetLeftPinkyKnucklePosition();

	//

	/** Functions to obtain right hand and fingers P & O */
	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetRightHandPosition();
	UFUNCTION(BlueprintPure, Category = "Hand")
	FRotator GetRightHandOrientation();

	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetRightIndexFingerPosition();
	UFUNCTION(BlueprintPure, Category = "Hand")
	FRotator GetRightIndexFingerOrientation();

	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetRightMiddleFingerPosition();
	UFUNCTION(BlueprintPure, Category = "Hand")
	FRotator GetRightMiddleFingerOrientation();

	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetRightRingFingerPosition();
	UFUNCTION(BlueprintPure, Category = "Hand")
	FRotator GetRightRingFingerOrientation();

	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetRightPinkyFingerPosition();
	UFUNCTION(BlueprintPure, Category = "Hand")
	FRotator GetRightPinkyFingerOrientation();

	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetRightThumbPosition();
	UFUNCTION(BlueprintPure, Category = "Hand")
	FRotator GetRightThumbOrientation();	
	
	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetRightIndexKnucklePosition();

	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetRightMiddleKnucklePosition();	
	
	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetRightRingKnucklePosition();

	UFUNCTION(BlueprintPure, Category = "Hand")
	FVector GetRightPinkyKnucklePosition();

	//

private:
	// Function to correct the right hand orientation
	FRotator RectifyRightHandOrientation(FRotator RawRotation);

	// Function to correct the left hand orientation
	FRotator RectifyLeftHandOrientation(FRotator RawRotation);

	/** Function to correct the hand position */
	FVector RectifyHandPosition(FVector RawPosition, FVector SensorOffset, FRotator SensorOrientation);

	FVector RectifyHandPosition(FVector RawPosition);

	FVector ApplySensorOffset(FVector CurrentPosition, FVector SensorOffset, FRotator SensorOrientation);

	float SensorDelayTotalTime;

	float SensorDelayElapsedTime;

	bool bIsDelayCompleted;

	int32 CalculateModule(bool bSample, int32 VerticesNum, int32 CurrentIndex);

	int32 CalculateModule(bool bSample, int32 VerticesNum, int32 CurrentIndex, int32& SampledIndex);
	
};
