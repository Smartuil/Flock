// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "BirdActor.h"
#include "FlockManager.generated.h"

USTRUCT()
struct FFlockMemberData
{
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

	FFlockMemberData()
	{
		InstanceIndex = 0;
		Velocity = FVector(0, 0, 0);
		ElapsedTimeSinceLastWander = 0.0f;
		WanderPosition = FVector(0, 0, 0);
		bIsFlockLeader = false;
	};
};

UCLASS()
class FLOCK_API AFlockManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFlockManager();
	
	UPROPERTY(BlueprintReadWrite, Category = Flock)
	int NumFlockInstance;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = Flock)
	USphereComponent* SphereComponent;
	
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
	
	TArray<FFlockMemberData> FlockMemberData;
	
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
	
	FVector GetRandomWanderLocation() const;
	
	TArray<int32> GetNearbyFlockMates(const int32 Flockmember);
	
	FVector SteeringWander(FFlockMemberData& Flocker) const;
	
	FVector SteeringAlign(FFlockMemberData& Flocker, TArray<int32>& FlockMates);
	
	FVector SteeringSeperate(const FFlockMemberData& Flocker, TArray<int32>& FlockMates);
	
	FVector SteeringCohesion(const FFlockMemberData& Flocker, TArray<int32>& FlockMates);
	
	FVector SteeringFollow(const FFlockMemberData& Flocker, int32 Flockleader);
	
	FVector SteeringFlee(const FFlockMemberData& Flocker);

	static FRotator FindLookAtRotation(FVector Start, FVector End);
};
