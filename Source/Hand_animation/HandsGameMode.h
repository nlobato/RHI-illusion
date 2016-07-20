// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "GameFramework/GameMode.h"
#include "HandsGameMode.generated.h"

// Enum to store the current state of gameplay
UENUM(BlueprintType)
enum class EExperimentPlayState
{
	EExperimentInitiated,
	EExperimentInProgress,
	EExperimentFinished,
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
	UFUNCTION(BlueprintPure, Category = "Power")
	EExperimentPlayState GetCurrentState() const;

	/** Sets a new playing state */
	void SetCurrentState(EExperimentPlayState NewState);

	/*UFUNCTION(BlueprintCallable, Category = "Timer")
	void StartExperiment(bool bShouldExperimentStart);*/

protected:

	// Length of the experiment in minutes
	UPROPERTY(EditAnywhere, Category = "Experiment setup")
	float ExperimentDurationTime;

	// Rubber Hand Illusion replication experiment
	UPROPERTY(EditAnywhere, Category = "RHI Replication Experiment")
	bool bIsExperimentForRHIReplication;

	UPROPERTY(EditAnywhere, Category = "RHI Replication Experiment")
	bool bIsSynchronousActive;

	// The objects will be spawn one at a time
	UPROPERTY(EditAnywhere, Category = "RHI Replication Experiment")
	bool bSpawnObjectsWithTimer;

	// Life time (in seconds) that each spawned object will have
	UPROPERTY(EditAnywhere, Category = "RHI Replication Experiment")
	float SpawnedObjectLifeTime;

	// Descriptor Points Algorithm experiment
	UPROPERTY(EditAnywhere, Category = "DP Algorithm Experiment")
	bool bIsExperimentForDPAlgorithm;

	// Activate/Deactivate the Descriptor Points algorithm for this experiment
	UPROPERTY(EditAnywhere, Category = "DP Algorithm Experiment")
	bool bAreDPsActive;

	// The mesh of the spawned object will be changing
	UPROPERTY(EditAnywhere, Category = "DP Algorithm Experiment")
	bool bIsMeshToChange;

	UPROPERTY(EditAnywhere, Category = "DP Algorithm Experiment")
	bool bIsSizeToChange;

	// Time (in seconds) that has to elapse before the objects changes shape or size
	UPROPERTY(EditAnywhere, Category = "DP Algorithm Experiment")
	float ObjectModificationLifeTime;

	// Number of cases for the mesh/shape experiment
	UPROPERTY(EditAnywhere, Category = "DP Algorithm Experiment")
	uint32 AmountOfChangesInObject;

private:
	/** Keeps track of the current playing state */
	EExperimentPlayState CurrentState;
	
	FVector AxisTranslation;

	bool bIsObject1Spawned;

	FTimerHandle ExperimentDurationTimerHandle;

	FTimerHandle SpawnedObjectTimerHandle;

	FTimerHandle ObjectModificationTimerHandle;
	
	void HasTimeRunOut();

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
};
