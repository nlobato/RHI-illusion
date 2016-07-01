// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "CalibrationBox.generated.h"

UCLASS()
class HAND_ANIMATION_API ACalibrationBox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACalibrationBox();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UFUNCTION(BlueprintCallable, Category = "Calibration")
	FVector GetAxisTranslation();

	UFUNCTION(BlueprintCallable, Category = "Calibration")
	FVector GetMeshScale();

	UFUNCTION(BlueprintCallable, Category = "Calibration")
	bool GetSystemCalibrationState();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Calibration", meta = (AllowProtectedAccess = "true"))
	FVector AxisTranslation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Calibration", meta = (AllowProtectedAccess = "true"))
	FVector MeshScale;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Calibration", meta = (AllowProtectedAccess = "true"))
	bool bIsSystemCalibrated;

private:
	// Box component that the character must overlap to begin the calibration
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (AllowPrivateAccess = "true"))
	class USceneComponent* DefaultComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Calibration", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CalibrationVolume;
	
};
