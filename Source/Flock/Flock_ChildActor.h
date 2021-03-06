﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "BirdActor.h"
#include "Flock_ChildActor.generated.h"

USTRUCT()
struct FlockMemberData_new {
	GENERATED_BODY()

	UPROPERTY()
		int32 InstanceIndex;
	UPROPERTY()
		FVector Velocity;
	UPROPERTY()
		FVector WanderPosition;
	UPROPERTY()
		FTransform Transform;
	UPROPERTY()
		float ElapsedTimeSinceLastWander;
	UPROPERTY()
		bool bIsFlockLeader;

	FlockMemberData_new() {
		InstanceIndex = 0;
		Velocity = FVector(0, 0, 0);
		ElapsedTimeSinceLastWander = 0.0f;
		WanderPosition = FVector(0, 0, 0);
		bIsFlockLeader = false;
	};
};

UCLASS()
class FLOCK_API AFlock_ChildActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFlock_ChildActor();
	UPROPERTY(BlueprintReadWrite, Category = Flock)
		int NumFlockInstance;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = Flock)
		USphereComponent* pSphereComponent;
	UPROPERTY(BlueprintReadOnly, Category = Flock)
		int32 NumFlock;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		float FlockRadius = 1000.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		float FlockMinSpeed = 10.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		float FlockMaxSpeed = 100.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		float FlockWanderUpdateRate = 2000.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		float FlockMinWanderDistance = 50.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		float FlockMateAwarnessRadius = 400.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		float FlockEnemyAwarnessRadius = 400.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		float FollowScale = 1.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		float AlignScale = 0.4f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		float CohesionScale = 1.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		float SeperationScale = 1.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		float FleeScale = 1.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		float SeperationRadius = 10.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		float FlockMaxSteeringForce = 100.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		float FlockMateRotationRate = 0.3f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		bool DrawSteeringRadius = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		bool DrawSteeringAlign = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		bool DrawSteeringFollow = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		bool DrawSteeringSeparate = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		bool DrawSteeringCohesion = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		bool DrawSteeringFlee = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		bool DrawSteeringFleeThreat = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		bool DrawLeaderTarget = false;
	TArray<FlockMemberData_new> mFlockMemberData_new;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		TArray<AActor*> Threats;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Flock)
		TSubclassOf<ABirdActor> BirdActor;
	TArray<AActor*> BirdArr;




protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable, Category = Flock)
		void AddFlockMemberWorldSpace(const FTransform& WorldTransform);
	FVector GetRandomWanderLocation();
	TArray<int32> GetNearbyFlockMates(int32 flockmember);
	FVector SteeringWander(FlockMemberData_new& flocker);
	FVector SteeringAlign(FlockMemberData_new& flocker, TArray<int32>& flockMates);
	FVector SteeringSeperate(FlockMemberData_new& flocker, TArray<int32>& flockMates);
	FVector SteeringCohesion(FlockMemberData_new& flocker, TArray<int32>& flockMates);
	FVector SteeringFollow(FlockMemberData_new& flocker, int32 flockleader);
	FVector SteeringFlee(FlockMemberData_new& flocker);
	FRotator FindLookAtRotation(FVector start, FVector end);
};
