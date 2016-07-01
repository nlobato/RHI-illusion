// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "HandsGameMode.generated.h"

/**
 * 
 */

// Enum to store the current state of gameplay
//UENUM(BlueprintType)
//enum class EBatteryPlayState
//{
//	EPlaying,
//	EGameOver,
//	EWon,
//	EUnknown
//};

UCLASS()
class HAND_ANIMATION_API AHandsGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:

	///** Returns the current playing state */
	//UFUNCTION(BlueprintPure, Category = "Power")
	//EBatteryPlayState GetCurrentState() const;

	///** Sets a new playing state */
	//void SetCurrentState(EBatteryPlayState NewState);
	
	
};
