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

	// The objects will be spawn one at a time
	UPROPERTY(EditAnywhere, Category = "Experiment setup")
	bool bSpawnObjectsWithTimer;

	// Life time (in seconds) that each spawned object will have
	UPROPERTY(EditAnywhere, Category = "Experiment setup")
	float SpawnedObjectLifeTime;

	// Synchronous or asynchronous experiment
	UPROPERTY(EditAnywhere, Category = "Experiment setup")
	bool bIsExperimentSynchronous;

	// Synchronous or asynchronous experiment
	UPROPERTY(EditAnywhere, Category = "Experiment setup")
	bool bIsExperimentForDPAlgorithm;

	// Activate/Deactivate the Descriptor Points algorithm for this experiment
	UPROPERTY(EditAnywhere, Category = "Experiment setup")
	bool bAreDPsActive;

	// The mesh of the spawned object will be changing
	UPROPERTY(EditAnywhere, Category = "Experiment setup")
	bool bIsMeshToChange;

	UPROPERTY(EditAnywhere, Category = "Experiment setup")
	bool bIsSizeToChange;

	// Time (in seconds) that has to elapse before the objects changes shape or size
	UPROPERTY(EditAnywhere, Category = "Experiment setup")
	float ObjectModificationLifeTime;



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

	TArray<uint32> ObjectIndex;

	uint32 ObjectCount;
};
