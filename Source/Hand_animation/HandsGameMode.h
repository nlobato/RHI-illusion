// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "GameFramework/GameMode.h"
#include "Hands_Character.h"
#include "HandsGameMode.generated.h"

// Enum to store the current state of gameplay
UENUM(BlueprintType)
enum class EExperimentPlayState
{
	EExperimentInitiated,
	ERHIExperimentInProgress,
	EDPExperimentInProgress,
	EExperimentFinished,
	EUnknown
};

// Enum of the messages to display
UENUM(BlueprintType)
enum class EMessages
{
	EWelcomeMessage,
	ECalibrationInstructions1,
	ECalibrationInstructions2,
	ECalibrationReady,
	ERHIExperimentInstructions,
	ERHINewObject,
	EDPAlgorithmInstructions,
	EDPAlgorithmQuestion,
	EExperimentFinishedMessage,
	EUnknown
};

UCLASS()
class HAND_ANIMATION_API AHandsGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:

	AHandsGameMode();
	
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
	
	/** Returns the current playing state */
	UFUNCTION(BlueprintPure, Category = "Experiment setup")
	EExperimentPlayState GetCurrentState() const;
	
	/** Sets a new playing state */
	void SetCurrentState(EExperimentPlayState NewState);

	/** Returns the current message to be displayed */
	UFUNCTION(BlueprintPure, Category = "Experiment setup")
	EMessages GetCurrentMessage() const;
	
	UPROPERTY(BlueprintReadOnly)
	bool bDisplayMessage;

	UPROPERTY(BlueprintReadOnly)
	bool bDisplayQuestion;

	UFUNCTION(BlueprintCallable, Category = "Calibration")
	void CalibrateSystem();
		
protected:

	UPROPERTY(EditAnywhere, Category = "Experiment log")
	int32 ParticipantCounter;

	UPROPERTY(EditAnywhere, Category = "Experiment log")
	FString SaveDirectory;

	UPROPERTY(EditAnywhere, Category = "Experiment log")
	FString FileName;

	UPROPERTY(EditAnywhere, Category = "Experiment setup")
	bool bIsExperimentForDPAlgorithm;

	UPROPERTY(EditAnywhere, Category = "Experiment setup")
	bool bIsExperimentForRHIReplication;

	UPROPERTY(EditAnywhere, Category = "Experiment log")
	FString LoadDirectory;

	UPROPERTY(EditAnywhere, Category = "Experiment log")
	FString DenseCorrespondanceIndicesFileName;

	UPROPERTY(EditAnywhere, Category = "Experiment log")
	FString DenseCorrespondanceVerticesFileName;

	UPROPERTY(EditAnywhere, Category = "Experiment log")
	FString OriginalMeshVerticesCoordinatesFromObjFileName;

	UPROPERTY(EditAnywhere, Category = "Experiment log")
	FString SecondMeshVerticesCoordinatesFromObjFileName;

	/*UPROPERTY(EditAnywhere, Category = "Experiment log")
	bool AllowOverwriting;*/

	UPROPERTY(EditAnywhere, Category = "Experiment setup")
	float SensorsSourceHeight;

	// Length of the experiment in minutes
	UPROPERTY(EditAnywhere, Category = "RHI Replication Experiment")
	float RHIExperimentDurationTime;

	// Rubber Hand Illusion replication experiment
	
	UPROPERTY(EditAnywhere, Category = "RHI Replication Experiment")
	bool bIsSynchronousActive;

	// The objects will be spawn one at a time
	UPROPERTY(EditAnywhere, Category = "RHI Replication Experiment")
	bool bSpawnObjectsWithTimer;

	// Life time (in seconds) that each spawned object will have
	UPROPERTY(EditAnywhere, Category = "RHI Replication Experiment")
	float SpawnedObjectLifeTime;

	UPROPERTY(EditAnywhere, Category = "DP Algorithm Experiment")
	float IllusionExperimentDurationTime;

	UPROPERTY(EditAnywhere, Category = "DP Algorithm Experiment")
	float VirtualObjectChangesDurationTime;	

	// Activate/Deactivate the Descriptor Points algorithm for this experiment
	UPROPERTY(EditAnywhere, Category = "DP Algorithm Experiment")
	bool bAreDPsActive;

	// The mesh of the spawned object will be changing
	UPROPERTY(EditAnywhere, Category = "DP Algorithm Experiment")
	bool bIsMeshToChange;

	UPROPERTY(EditAnywhere, Category = "DP Algorithm Experiment")
	bool bIsSizeToChange;
	
	// Number of cases for the mesh/shape experiment
	UPROPERTY(EditAnywhere, Category = "DP Algorithm Experiment")
	uint32 AmountOfChangesInObject;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Experiment", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> HUDWidgetClass;

	UPROPERTY()
	class UUserWidget* CurrentWidget;	

	UPROPERTY(EditAnywhere, Category = "Meshes path")
	UStaticMesh* SecondMesh;

	/*UPROPERTY(EditAnywhere, Category = "Meshes path")
	FString Mesh2Path;

	UPROPERTY(EditAnywhere, Category = "Meshes path")
	FString Mesh3Path;

	UPROPERTY(EditAnywhere, Category = "Meshes path")
	FString Mesh4Path;*/

private:
	/** Keeps track of the current playing state */
	EExperimentPlayState CurrentState;

	EMessages MessageToDisplay;
		
	FVector AxisTranslation;

	bool bIsObject1Spawned;

	FTimerHandle ExperimentDurationTimerHandle;

	FTimerHandle SpawnedObjectTimerHandle;

	FTimerHandle ObjectModificationTimerHandle;

	FTimerHandle CalibrationTimerHandle;

	FTimerHandle MessagesTimerHandle;
	
	void HasTimeRunOut();

	void ToggleMessage();

	void SpawnNewObject();

	/** handle any functions calls that rely upon changing the playing state of our game */
	void HandleNewState(EExperimentPlayState NewState);

	void ChangeMeshObject();

	void ChangeSizeObject();

	TArray<uint32> ObjectIndex;

	uint32 TimesObjectHasSpawnedCounter;

	TArray<FVector> ObjectSizeChangesArray;

	bool bHasRealSizeObjectIndexBeenSet;

	uint32 RealSizeObjectIndex;

	uint32 RealSizeObjectIndexCounter;
		
	void SetObjectNewScale();

	void SpawnObjectsForDecision();

	//AInteractionObject* DecisionObject;

	TSubclassOf<class AInteractionObject>* PointerToObjectSpawnedByCharacter;

	FVector RootLocation;

	void DecisionEvaluation(int32 ObjectChosen);

	bool bHasAnswerBeenGiven;

	bool bIsSystemCalibrated;

	bool bIsShoulderCalibrated;

	void ReadTextFile(FString AbsolutePathToFile, TArray<FVector>& TargetCoordinatesArray);

	void ReadTextFile(FString AbsolutePathToFile, TArray<int32>& TargetIndicesArray);
	
	void DPExperimentFirstPartOver();

	bool bIsOriginalMesh;

	void InitializeArrays();

	UStaticMesh* OriginalMesh;
	
	TArray<int32>* PtrDenseCorrespondenceIndices;

	TArray<FVector> DenseCorrespondenceCoordinates;

	TArray<FVector> OriginalMeshVerticesCoordinatesFromObjFile;

	TArray<FVector> SecondMeshVerticesCoordinatesFromObjFile;

	TArray<FVector>* PtrOriginalMeshVerticesCoordinatesFromUE4Asset;

	TArray<FVector>* PtrOriginalMeshVerticesNormalsFromUE4Asset;

	TArray<FVector>* PtrOriginalMeshVerticesTangentsFromUE4Asset;

	TArray<FVector>* PtrOriginalMeshVerticesBinormalsFromUE4Asset;

	TArray<FVector>* PtrSecondMeshVerticesCoordinatesFromUE4Asset;

	TArray<FVector>* PtrSecondMeshVerticesNormalsFromUE4Asset;

	TArray<FVector>* PtrSecondMeshVerticesTangentsFromUE4Asset;

	TArray<FVector>* PtrSecondMeshVerticesBinormalsFromUE4Asset;

	TArray<FArrayForStoringIndices>* PtrMesh2Mesh1Correspondences;

	//TArray<FArrayForStoringIndices> Mapping2ndAssetToObj;

	TArray<int32> Mapped1stMeshCorrespondences;

	TArray<int32> Mapped2ndMeshCorrespondences;

	TArray<int32>* PtrOriginalMeshIndices;

	TArray<int32>* PtrSecondMeshIndices;

	//FArrayOfint32Arrays* PtrMappingBetweenMeshes;

	void AccessMeshVertices(UStaticMesh* MeshToAccess, TArray<FVector>& ObjFile, TArray<FVector>& ArrayToStoreCoordinates, TArray<FVector>& ArrayToStoreNormals, TArray<FVector>& ArrayToStoreTangents, TArray<FVector>& ArrayToStoreBinormals, TArray<int32>& ArrayToStoreIndices);

	void Map2ndMeshCorrespondences(TArray<FVector>& UE4Asset, TArray<FVector>& ObjFile, TArray<int32>& MappingAssetToObj);

	void MappingBetweenMeshes(TArray<FVector>& OriginalUE4Asset, TArray<FVector>& OriginalObjFile, TArray<FArrayForStoringIndices>& MappedCorrespondences);
};
