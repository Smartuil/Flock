// Fill out your copyright notice in the Description page of Project Settings.

#include "FlockActor.h"
#include "Public/DrawDebugHelpers.h"

// Sets default values
AFlockActor::AFlockActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	pSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	if (pSphereComponent) {
		pSphereComponent->InitSphereRadius(FlockRadius);
		RootComponent = pSphereComponent;
	}
	pStaticMeshInstanceComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedMeshs"));
	if (pStaticMeshInstanceComponent)
		pStaticMeshInstanceComponent->AttachTo(RootComponent);
	SetActorTickEnabled(true);
}

void AFlockActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

// Called when the game starts or when spawned
void AFlockActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFlockActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!pStaticMeshInstanceComponent)
		return;
	for (int32 i = 0; i < mFlockMemberData.Num(); i++) {
		FlockMemberData& flocker = mFlockMemberData[i];
		FVector pos = flocker.Transform.GetLocation();
		if (flocker.InstanceIndex > pStaticMeshInstanceComponent->GetInstanceCount()) {
			continue;
		}
		FVector newVelocity = FVector(0, 0, 0);
		if (flocker.bIsFlockLeader) {
			newVelocity += SteeringWander(flocker);
			flocker.ElapsedTimeSinceLastWander += DeltaTime;
			if (DrawLeaderTarget) {
				DrawDebugSphere(GetWorld(), flocker.WanderPosition, 20.0f, 16, FColor::Black, false, 0.1f);
				DrawDebugDirectionalArrow(GetWorld(), pos, flocker.WanderPosition, 10.0f, FColor::Cyan);
			}
		}
		else {
			FVector followVec = FVector(0, 0, 0);
			FVector cohesionVec = FVector(0, 0, 0);
			FVector alignmentVec = FVector(0, 0, 0);
			FVector seperationVec = FVector(0, 0, 0);
			FVector fleeVec = FVector(0, 0, 0);
			if (FollowScale > 0.0f) {
				followVec = (SteeringFollow(flocker, 0)*FollowScale);
				if (DrawSteeringFollow) {
					DrawDebugDirectionalArrow(GetWorld(), pos, pos + followVec, 10.0f, FColor::Cyan);
				}
			}
			TArray<int32> mates = GetNearbyFlockMates(i);
			if (CohesionScale > 0.0f) {
				cohesionVec = (SteeringCohesion(flocker, mates)*CohesionScale);
				if (DrawSteeringCohesion) {
					DrawDebugDirectionalArrow(GetWorld(), pos, pos + cohesionVec, 10.0f, FColor::Green);
				}
			}
			if (AlignScale > 0.0f)
			{
				alignmentVec = (SteeringAlign(flocker, mates) * AlignScale);
				if (DrawSteeringAlign) {
					DrawDebugDirectionalArrow(GetWorld(), pos, pos + alignmentVec, 10.0f, FColor::Green);
				}
			}
			if (SeperationScale > 0.0f) {
				seperationVec = (SteeringSeperate(flocker, mates)*SeperationScale);
				if (DrawSteeringSeparate) {
					DrawDebugDirectionalArrow(GetWorld(), pos, pos + seperationVec, 10.0f, FColor::Red);
				}
			}
			if (FleeScale > 0.0f) {
				fleeVec = (SteeringFlee(flocker)*FleeScale);
				if (DrawSteeringFlee) {
					DrawDebugDirectionalArrow(GetWorld(), pos, pos + fleeVec, 10.0f, FColor::Cyan);
				}
			}
			newVelocity += fleeVec;
			if (fleeVec.SizeSquared() <= 0.1f) {
				newVelocity += followVec;
				newVelocity += cohesionVec;
				newVelocity += alignmentVec;
				newVelocity += seperationVec;
			}
		}
		newVelocity = newVelocity.GetClampedToSize(0.0f, FlockMaxSteeringForce);
		FVector targetVelocity = flocker.Velocity + newVelocity;
		FRotator rot = FindLookAtRotation(flocker.Transform.GetLocation(), flocker.Transform.GetLocation() + targetVelocity);
		FRotator final = FMath::RInterpTo(flocker.Transform.Rotator(), rot, DeltaTime, FlockMateRotationRate);
		flocker.Transform.SetRotation(final.Quaternion());
		FVector forward = flocker.Transform.GetUnitAxis(EAxis::X);
		forward.Normalize();
		flocker.Velocity = forward * targetVelocity.Size();
		if (flocker.Velocity.Size() > FlockMaxSpeed) {
			flocker.Velocity = flocker.Velocity.GetSafeNormal()*FlockMaxSpeed;
		}
		if (flocker.Velocity.Size() < FlockMinSpeed) {
			flocker.Velocity = flocker.Velocity.GetSafeNormal()*FlockMinSpeed;
		}
		flocker.Transform.SetLocation(flocker.Transform.GetLocation() + flocker.Velocity*DeltaTime);
		if (DrawSteeringRadius) {
			DrawDebugSphere(GetWorld(), flocker.Transform.GetLocation(), FlockMateAwarnessRadius, 8, FColor::Yellow);
		}
		pStaticMeshInstanceComponent->UpdateInstanceTransform(flocker.InstanceIndex, flocker.Transform, true, true);
		//UE_LOG(LogTemp, Warning, TEXT("%d"),flocker.Transform.GetLocation().X);
	}
	if (DrawSteeringFleeThreat) {
		for (int i = 0; i < Threats.Num(); i++) {
			DrawDebugSphere(GetWorld(), Threats[i]->GetActorLocation(), FlockEnemyAwarnessRadius, 8, FColor::Magenta);
		}
	}
	pStaticMeshInstanceComponent->UpdateBounds();
}

void AFlockActor::AddFlockMember(const FTransform & InstanceTransform)
{
	pStaticMeshInstanceComponent->AddInstance(InstanceTransform);
}

void AFlockActor::AddFlockMemberWorldSpace(const FTransform & WorldTransform)
{
	pStaticMeshInstanceComponent->AddInstanceWorldSpace(WorldTransform);
	FlockMemberData flocker;
	flocker.InstanceIndex = NumFlock;
	flocker.Transform = WorldTransform;
	flocker.WanderPosition = GetRandomWanderLocation();
	flocker.Velocity = SteeringWander(flocker);
	if (NumFlock == 0) {
		flocker.bIsFlockLeader = true;
	}
	mFlockMemberData.Add(flocker);
	NumFlock++;
}

FVector AFlockActor::GetRandomWanderLocation()
{
	return FMath::VRand() * (FMath::FRand() * FlockRadius);
}

TArray<int32> AFlockActor::GetNearbyFlockMates(int32 flockmember)
{
	TArray<int32> mates;
	if (flockmember > mFlockMemberData.Num())
		return mates;
	if (flockmember < 0)
		return mates;
	for (int32 i = 0; i < mFlockMemberData.Num(); i++) {
		if (i != flockmember) {
			FVector diff = mFlockMemberData[i].Transform.GetLocation() - mFlockMemberData[flockmember].Transform.GetLocation();
			if (FMath::Abs(diff.Size()) < FlockMateAwarnessRadius) {
				mates.Add(i);
			}
		}
	}
	return mates;
}

FVector AFlockActor::SteeringWander(FlockMemberData & flocker)
{
	FVector newVec = flocker.WanderPosition - flocker.Transform.GetLocation();
	if (flocker.ElapsedTimeSinceLastWander >= FlockWanderUpdateRate || newVec.Size() <= FlockMinWanderDistance) {
		flocker.WanderPosition = GetRandomWanderLocation() + GetActorLocation();
		flocker.ElapsedTimeSinceLastWander = 0.0f;
		newVec = flocker.WanderPosition - flocker.Transform.GetLocation();
	}
	return newVec;
}

FVector AFlockActor::SteeringAlign(FlockMemberData & flocker, TArray<int32>& flockMates)
{
	FVector vel = FVector(0, 0, 0);
	if (flockMates.Num() == 0) {
		return vel;
	}
	for (int32 i = 0; i < flockMates.Num(); i++) {
		vel += mFlockMemberData[flockMates[i]].Velocity;
	}
	vel /= (float)flockMates.Num();
	return vel;
}

FVector AFlockActor::SteeringSeperate(FlockMemberData & flocker, TArray<int32>& flockMates)
{
	FVector force = FVector(0, 0, 0);
	if (flockMates.Num() == 0) 
		return force;
	for (int32 i = 0; i < flockMates.Num(); i++)
	{
		FVector diff = flocker.Transform.GetLocation() - mFlockMemberData[flockMates[i]].Transform.GetLocation();
		float scale = diff.Size();
		diff.Normalize();
		diff = diff * (SeperationRadius / scale);
		force += diff;
	}
	return force;
}

FVector AFlockActor::SteeringCohesion(FlockMemberData & flocker, TArray<int32>& flockMates)
{
	FVector avgpos = FVector(0, 0, 0);
	if (flockMates.Num() == 0) 
		return avgpos;
	for (int32 i = 0; i < flockMates.Num(); i++){
		avgpos += mFlockMemberData[flockMates[i]].Transform.GetLocation();
	}
	avgpos /= (float)flockMates.Num();
	return avgpos - flocker.Transform.GetLocation();
}

FVector AFlockActor::SteeringFollow(FlockMemberData & flocker, int32 flockleader)
{
	FVector newVec = FVector(0, 0, 0);
	if (flockleader <= mFlockMemberData.Num() && flockleader >= 0){
		newVec = mFlockMemberData[flockleader].Transform.GetLocation() - flocker.Transform.GetLocation();
		newVec.Normalize();
		newVec *= FlockMaxSpeed;
		newVec -= flocker.Velocity;
		if (newVec.Size() > FlockMateAwarnessRadius){
			newVec = FVector(0, 0, 0);
		}
	}
	return newVec;
}

FVector AFlockActor::SteeringFlee(FlockMemberData & flocker)
{
	FVector newVec = FVector(0, 0, 0);
	for (int i = 0; i < Threats.Num(); i++){
		if (Threats[i] != NULL && Threats[i]->IsValidLowLevel()){
			FVector fromEnemy = flocker.Transform.GetLocation() - Threats[i]->GetActorLocation();
			float distanceToEnemy = fromEnemy.Size();
			fromEnemy.Normalize();
			if (distanceToEnemy < FlockEnemyAwarnessRadius){
				newVec += fromEnemy * ((FlockEnemyAwarnessRadius / distanceToEnemy) * FleeScale);
			}
		}
	}
	return newVec;
}

FRotator AFlockActor::FindLookAtRotation(FVector start, FVector end)
{
	return FRotationMatrix::MakeFromX(end - start).Rotator();
}
