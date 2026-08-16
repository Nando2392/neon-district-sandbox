// Copyright Neon District Sandbox. Public benchmark repo — original content only.

#include "Combat/NDWeaponProjectile.h"
#include "AI/NDNPCCharacter.h"
#include "Core/NDGameInstance.h"
#include "Audio/NDAudioManager.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
	const TCHAR* SphereMeshPath = TEXT("/Engine/BasicShapes/Sphere.Sphere");
	const TCHAR* BasicShapeMatPath = TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial");
}

ANDWeaponProjectile::ANDWeaponProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(13.0f);
	Collision->SetCollisionProfileName(TEXT("PhysicsActor"));
	Collision->SetNotifyRigidBodyCollision(true);
	Collision->SetSimulatePhysics(true);
	Collision->SetLinearDamping(0.04f);
	Collision->SetAngularDamping(0.1f);

	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(Collision);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visual->SetRelativeScale3D(FVector(0.22f));
	Visual->bCastDynamicShadow = true;
}

void ANDWeaponProjectile::BeginPlay()
{
	Super::BeginPlay();
	Collision->OnComponentHit.AddDynamic(this, &ANDWeaponProjectile::HandleHit);

	if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, SphereMeshPath))
	{
		Visual->SetStaticMesh(Sphere);
	}
	if (UMaterialInterface* Mat = LoadObject<UMaterialInterface>(nullptr, BasicShapeMatPath))
	{
		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Mat, this);
		MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.10f, 0.85f, 1.0f));
		MID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.10f, 0.85f, 1.0f));
		MID->SetVectorParameterValue(TEXT("NeonColor"), FLinearColor(0.10f, 0.85f, 1.0f));
		MID->SetScalarParameterValue(TEXT("EmissiveStrength"), 6.0f);
		Visual->SetMaterial(0, MID);
	}
}

void ANDWeaponProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	LifeSeconds -= DeltaSeconds;
	if (LifeSeconds <= 0.0f)
	{
		Destroy();
	}
}

void ANDWeaponProjectile::Launch(const FVector& Direction, AActor* InInstigatorActor)
{
	InstigatorActor = InInstigatorActor;
	if (Collision)
	{
		Collision->SetPhysicsLinearVelocity(Direction.GetSafeNormal() * LaunchSpeed);
	}
}

void ANDWeaponProjectile::HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == InstigatorActor)
	{
		return;
	}

	UGameplayStatics::ApplyPointDamage(OtherActor, Damage, GetVelocity().GetSafeNormal(), Hit,
		InstigatorActor ? InstigatorActor->GetInstigatorController() : nullptr, this, nullptr);

	if (OtherComp && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity().GetSafeNormal() * ImpulseStrength, Hit.ImpactPoint);
	}
	else if (ACharacter* HitCharacter = Cast<ACharacter>(OtherActor))
	{
		HitCharacter->LaunchCharacter(GetVelocity().GetSafeNormal2D() * 420.0f + FVector(0.0f, 0.0f, 90.0f), true, true);
	}

	if (UNDGameInstance* GI = GetGameInstance<UNDGameInstance>())
	{
		if (UNDAudioManager* Audio = GI->GetAudioManager())
		{
			Audio->PlayImpact(OtherActor);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("NeonDistrict: weapon projectile hit actor=%s impulse=(%.0f,%.0f,%.0f)"),
		*OtherActor->GetName(), GetVelocity().X, GetVelocity().Y, GetVelocity().Z);
	Destroy();
}
