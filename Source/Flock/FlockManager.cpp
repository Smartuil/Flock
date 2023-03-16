// Fill out your copyright notice in the Description page of Project Settings.

#include "FlockManager.h"
#include "Public/DrawDebugHelpers.h"
#include "Engine/World.h"

// Sets default values
AFlockManager::AFlockManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	if (SphereComponent)
	{
		SphereComponent->InitSphereRadius(FlockRadius);
		RootComponent = SphereComponent;
	}
	SetActorTickEnabled(true);
}

// Called when the game starts or when spawned
void AFlockManager::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AFlockManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	for (int32 i = 0; i < FlockMemberData.Num(); i++)
	{
		FFlockMemberData& Flocker = FlockMemberData[i];
		FVector Pos = Flocker.Transform.GetLocation();
		FVector NewVelocity = FVector(0, 0, 0);
		if (Flocker.bIsFlockLeader)
		{
			NewVelocity += SteeringWander(Flocker);
			Flocker.ElapsedTimeSinceLastWander += DeltaTime;
			if (DrawLeaderTarget)
			{
				DrawDebugSphere(GetWorld(), Flocker.WanderPosition, 20.0f, 16, FColor::Black, false, 0.1f);
				DrawDebugDirectionalArrow(GetWorld(), Pos, Flocker.WanderPosition, 10.0f, FColor::Cyan);
			}
		}
		else
		{
			FVector FollowVec = FVector(0, 0, 0);
			FVector CohesionVec = FVector(0, 0, 0);
			FVector AlignmentVec = FVector(0, 0, 0);
			FVector SeperationVec = FVector(0, 0, 0);
			FVector FleeVec = FVector(0, 0, 0);
			if (FollowScale > 0.0f)
			{
				FollowVec = (SteeringFollow(Flocker, 0)*FollowScale);
				if (DrawSteeringFollow)
				{
					DrawDebugDirectionalArrow(GetWorld(), Pos, Pos + FollowVec, 10.0f, FColor::Cyan);
				}
			}
			TArray<int32> Mates = GetNearbyFlockMates(i);
			if (CohesionScale > 0.0f)
			{
				CohesionVec = (SteeringCohesion(Flocker, Mates)*CohesionScale);
				if (DrawSteeringCohesion)
				{
					DrawDebugDirectionalArrow(GetWorld(), Pos, Pos + CohesionVec, 10.0f, FColor::Green);
				}
			}
			if (AlignScale > 0.0f)
			{
				AlignmentVec = (SteeringAlign(Flocker, Mates) * AlignScale);
				if (DrawSteeringAlign)
				{
					DrawDebugDirectionalArrow(GetWorld(), Pos, Pos + AlignmentVec, 10.0f, FColor::Green);
				}
			}
			if (SeperationScale > 0.0f)
			{
				SeperationVec = (SteeringSeperate(Flocker, Mates)*SeperationScale);
				if (DrawSteeringSeparate)
				{
					DrawDebugDirectionalArrow(GetWorld(), Pos, Pos + SeperationVec, 10.0f, FColor::Red);
				}
			}
			if (FleeScale > 0.0f)
			{
				FleeVec = (SteeringFlee(Flocker)*FleeScale);
				if (DrawSteeringFlee)
				{
					DrawDebugDirectionalArrow(GetWorld(), Pos, Pos + FleeVec, 10.0f, FColor::Cyan);
				}
			}
			NewVelocity += FleeVec;
			if (FleeVec.SizeSquared() <= 0.1f)
			{
				NewVelocity += FollowVec;
				NewVelocity += CohesionVec;
				NewVelocity += AlignmentVec;
				NewVelocity += SeperationVec;
			}
		}
		
		NewVelocity = NewVelocity.GetClampedToSize(0.0f, FlockMaxSteeringForce);
		FVector TargetVelocity = Flocker.Velocity + NewVelocity;
		FRotator Rot = FindLookAtRotation(Flocker.Transform.GetLocation(), Flocker.Transform.GetLocation() + TargetVelocity);
		FRotator Final = FMath::RInterpTo(Flocker.Transform.Rotator(), Rot, DeltaTime, FlockMateRotationRate);//interp
		Flocker.Transform.SetRotation(Final.Quaternion());
		FVector Forward = Flocker.Transform.GetUnitAxis(EAxis::X);
		Forward.Normalize();
		Flocker.Velocity = Forward * TargetVelocity.Size();
		if (Flocker.Velocity.Size() > FlockMaxSpeed)
		{
			Flocker.Velocity = Flocker.Velocity.GetSafeNormal() * FlockMaxSpeed;
		}
		if (Flocker.Velocity.Size() < FlockMinSpeed)
		{
			Flocker.Velocity = Flocker.Velocity.GetSafeNormal() * FlockMinSpeed;
		}
		Flocker.Transform.SetLocation(Flocker.Transform.GetLocation() + Flocker.Velocity * DeltaTime);
		if (DrawSteeringRadius)
		{
			DrawDebugSphere(GetWorld(), Flocker.Transform.GetLocation(), FlockMateAwarnessRadius, 8, FColor::Yellow);
		}
		
		if(ABirdActor* BirdActor = Cast<ABirdActor>(BirdArr[i]))
		{
			BirdActor->SetActorTransform(Flocker.Transform);
		}
	}
	if (DrawSteeringFleeThreat)
	{
		for (int i = 0; i < Threats.Num(); i++)
		{
			DrawDebugSphere(GetWorld(), Threats[i]->GetActorLocation(), FlockEnemyAwarnessRadius, 8, FColor::Magenta);
		}
	}
}

void AFlockManager::AddFlockMemberWorldSpace(const FTransform& WorldTransform)
{
	const FActorSpawnParameters SpawnParameters;
	BirdArr.Add(GetWorld()->SpawnActor<ABirdActor>(BirdActor, WorldTransform.GetLocation(), WorldTransform.Rotator(), SpawnParameters));
	FFlockMemberData Flocker;
	Flocker.InstanceIndex = NumFlock;
	Flocker.Transform = WorldTransform;
	Flocker.WanderPosition = GetRandomWanderLocation();
	Flocker.Velocity = SteeringWander(Flocker);
	if (NumFlock == 0)
	{
		Flocker.bIsFlockLeader = true;
	}
	FlockMemberData.Add(Flocker);
	NumFlock++;
}

FVector AFlockManager::GetRandomWanderLocation() const
{
	return FMath::VRand() * (FMath::FRand() * FlockRadius);
}

TArray<int32> AFlockManager::GetNearbyFlockMates(const int32 Flockmember)
{
	TArray<int32> Mates;
	if (Flockmember > FlockMemberData.Num())
		return Mates;
	
	if (Flockmember < 0)
		return Mates;
	
	for (int32 i = 0; i < FlockMemberData.Num(); i++)
	{
		if (i != Flockmember)
		{
			FVector Diff = FlockMemberData[i].Transform.GetLocation() - FlockMemberData[Flockmember].Transform.GetLocation();
			if (FMath::Abs(Diff.Size()) < FlockMateAwarnessRadius)
			{
				Mates.Add(i);
			}
		}
	}
	return Mates;
}

FVector AFlockManager::SteeringWander(FFlockMemberData & Flocker) const
{
	FVector NewVec = Flocker.WanderPosition - Flocker.Transform.GetLocation();
	if (Flocker.ElapsedTimeSinceLastWander >= FlockWanderUpdateRate || NewVec.Size() <= FlockMinWanderDistance)
	{
		Flocker.WanderPosition = GetRandomWanderLocation() + GetActorLocation();
		Flocker.ElapsedTimeSinceLastWander = 0.0f;
		NewVec = Flocker.WanderPosition - Flocker.Transform.GetLocation();
	}
	return NewVec;
}

FVector AFlockManager::SteeringAlign(FFlockMemberData& Flocker, TArray<int32>& FlockMates)
{
	FVector Vel = FVector(0, 0, 0);
	if (FlockMates.Num() == 0)
	{
		return Vel;
	}
	for (int32 i = 0; i < FlockMates.Num(); i++)
	{
		Vel += FlockMemberData[FlockMates[i]].Velocity;
	}
	
	Vel /= static_cast<float>(FlockMates.Num());
	return Vel;
}

FVector AFlockManager::SteeringSeperate(const FFlockMemberData& Flocker, TArray<int32>& FlockMates)
{
	FVector Force = FVector(0, 0, 0);
	if (FlockMates.Num() == 0)
		return Force;
	
	for (int32 i = 0; i < FlockMates.Num(); i++)
	{
		FVector Diff = Flocker.Transform.GetLocation() - FlockMemberData[FlockMates[i]].Transform.GetLocation();
		const float Scale = Diff.Size();
		Diff.Normalize();
		Diff = Diff * (SeperationRadius / Scale);
		Force += Diff;
	}
	
	return Force;
}

FVector AFlockManager::SteeringCohesion(const FFlockMemberData& Flocker, TArray<int32>& FlockMates)
{
	FVector Avgpos = FVector(0, 0, 0);
	if (FlockMates.Num() == 0)
		return Avgpos;
	
	for (int32 i = 0; i < FlockMates.Num(); i++)
	{
		Avgpos += FlockMemberData[FlockMates[i]].Transform.GetLocation();
	}
	
	Avgpos /= static_cast<float>(FlockMates.Num());
	return Avgpos - Flocker.Transform.GetLocation();
}

FVector AFlockManager::SteeringFollow(const FFlockMemberData& Flocker, const int32 Flockleader)
{
	FVector NewVec = FVector(0, 0, 0);
	if (Flockleader <= FlockMemberData.Num() && Flockleader >= 0)
	{
		NewVec = FlockMemberData[Flockleader].Transform.GetLocation() - Flocker.Transform.GetLocation();
		NewVec.Normalize();
		NewVec *= FlockMaxSpeed;
		NewVec -= Flocker.Velocity;
		if (NewVec.Size() > FlockMateAwarnessRadius)
		{
			NewVec = FVector(0, 0, 0);
		}
	}
	return NewVec;
}

FVector AFlockManager::SteeringFlee(const FFlockMemberData& Flocker)
{
	FVector NewVec = FVector(0, 0, 0);
	for (int i = 0; i < Threats.Num(); i++)
	{
		if (Threats[i] != nullptr && Threats[i]->IsValidLowLevel())
		{
			FVector FromEnemy = Flocker.Transform.GetLocation() - Threats[i]->GetActorLocation();
			const float DistanceToEnemy = FromEnemy.Size();
			FromEnemy.Normalize();
			if (DistanceToEnemy < FlockEnemyAwarnessRadius)
			{
				NewVec += FromEnemy * ((FlockEnemyAwarnessRadius / DistanceToEnemy) * FleeScale);
			}
		}
	}
	return NewVec;
}

FRotator AFlockManager::FindLookAtRotation(FVector Start, FVector End)
{
	return FRotationMatrix::MakeFromX(End - Start).Rotator();
}
