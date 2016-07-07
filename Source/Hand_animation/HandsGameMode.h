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
	UPROPERTY(EditAnywhere, Category = "Experiment")
	float ExperimentDurationTime;

private:
	/** Keeps track of the current playing state */
	EExperimentPlayState CurrentState;

	/** handle any functions calls that rely upon changing the playing state of our game */
	void HandleNewState(EExperimentPlayState NewState);

	FTimerHandle ExperimentDurationTimerHandle;
	
	void HasTimeRunOut();

};
