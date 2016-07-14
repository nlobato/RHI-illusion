// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "Hands_Character.generated.h"

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

protected:
	
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

	/* The spawned object */
	//AInteractionObject* SpawnedObject;

	/** Has the object been spawned? */
	bool bIsObject1Spawned;

	/** Has the object been spawned? */
	bool bIsObject2Spawned;

	/** Has the object been spawned? */
	bool bIsObject3Spawned;

	/** Has the object been spawned? */
	bool bIsObject4Spawned;


	//bool bHasObjectSizeChanged;
		
	/** Object P & O*/
	void Object1Movement(float Value);

	void Object2Movement(float Value);

	void Object3Movement(float Value);

	void Object4Movement(float Value);

	void ModifyObjectSize();

	void ResetObjectSize();

protected:

	USkeletalMeshComponent* MyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Calibration", meta = (BlueprintProtected = "true"))
	FVector MeshScale;
	
	/** Array storing the position, normal, tangent and binormal of each descriptor point */
	TArray<FVector> DescriptorPointInfo;
	TArray<FVector> DPOriginalObject;
	TArray<FVector> DPVirtualObject;
	TArray<uint32> VertexIndices;
		
	TArray<float>* WeightsPointer;
	TArray<float>* TransformationPointer;
	TArray<FVector>* DescriptorPointsPointer;

	TArray<float> LeftHandWeights;
	TArray<float> LeftIndexFingerWeights;
	TArray<float> LeftMiddleFingerWeights;
	TArray<float> LeftRingFingerWeights;
	TArray<float> LeftPinkyFingerWeights;
	TArray<float> LeftThumbWeights;

	TArray<float> LeftMiddleKnuckleWeights;

	TArray<float> RightHandWeights;
	TArray<float> RightIndexFingerWeights;
	TArray<float> RightMiddleFingerWeights;
	TArray<float> RightRingFingerWeights;
	TArray<float> RightPinkyFingerWeights;
	TArray<float> RightThumbWeights;

	TArray<float> RightMiddleKnuckleWeights;

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

	//TArray<float> w_biprime;
	
	/** Number of descriptor points */
	UPROPERTY(EditAnywhere, Category = "Vertices")
	uint32 NumDescriptorPoints;

	UPROPERTY(EditAnywhere, Category = "Vertices")
	bool bDrawPoints;

	UPROPERTY(EditAnywhere, Category = "Vertices")
	bool bDrawRightHandPoints;

	UPROPERTY(EditAnywhere, Category = "Vertices")
	bool bDrawLeftHandPoints;

	//bool bAreDPset;
	TArray<uint32>* pDPIndices;

	/** Vertices indices for the descriptor points */
	TArray<uint32> DPIndices;

	float sum_w_biprime;

protected:

	//Function for accesing the triangles information
	void AccessTriVertices(const UStaticMeshComponent* InStaticMeshComponent, TArray<FVector>& DescriptorPointsArray);

	//Random initial descriptor points
	void InitDescriptionPoints(uint32 NumDP, uint32 NumVertices);

	//Draw the descriptor points
	void DrawDescriptionPoints(TArray<FVector>& DescriptorPointsArray);

	//Weights calculation
	void WeightsComputation(TArray<float>& WeightsArray, TArray<float>& TransformationArray, FVector p_j);

	//TArray<float> WeightsComputation(FVector p_j);

	/** Calculate a new joint position with the descriptor points weights */
	FVector NewJointPosition(TArray<float>& WeightsArray, TArray<float>& TransformationArray, TArray<FVector>& DescriptorPointsArray);

protected:

	UPROPERTY(EditAnywhere, Category = "Hand", meta = (BlueprintProtected = "true"))
	bool bAreDPsActive;

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
		
	/** Left Index finger P & O */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector LeftIndexFingerPosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPLeftIndexFingerPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator LeftIndexFingerOrientation;

	/** Left Middle finger P & O */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector LeftMiddleFingerPosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPLeftMiddleFingerPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator LeftMiddleFingerOrientation;

	/** Left ring finger P & O */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector LeftRingFingerPosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPLeftRingFingerPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator LeftRingFingerOrientation;

	/** Left pinky P & O */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector LeftPinkyFingerPosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPLeftPinkyFingerPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator LeftPinkyFingerOrientation;

	/** Left thumb P & O */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector LeftThumbPosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPLeftThumbPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FRotator LeftThumbOrientation;
	//
	
	// Left hand knuckles
	
	/* Middle Index finger knuckle P & O*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector LeftMiddleKnucklePosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPLeftMiddleKnucklePosition;

	// Right hand knuckles

	/* Middle Index finger knuckle P & O*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector RightMiddleKnucklePosition;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand", meta = (BlueprintProtected = "true"))
	FVector DPRightMiddleKnucklePosition;

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
	FVector RightMiddleKnuckleSensorOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = (BlueprintProtected = "true"))
	float SensorDelayRangeLow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = (BlueprintProtected = "true"))
	float SensorDelayRangeHigh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = (BlueprintProtected = "true"))
	bool bIsDelayActive;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/* The spawned object */
	AInteractionObject* SpawnedObject;

	// Calibration completed
	void CalibrateSystem(FVector AxisTranslation);

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

	bool bAreDPset;

	bool bHasObjectSizeChanged;
	
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
	FVector GetLeftMiddleKnucklePosition();

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
	FVector GetRightMiddleKnucklePosition();

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
	
};
