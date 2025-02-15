// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileDefault.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AProjectileDefault::AProjectileDefault()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BulletCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));
	
	BulletCollisionSphere->SetSphereRadius(16.f);

	BulletCollisionSphere->OnComponentHit.AddDynamic(this, &AProjectileDefault::BulletCollisionSphereHit);
	BulletCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AProjectileDefault::BulletCollisionSphereBeginOverlap);
	BulletCollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AProjectileDefault::BulletCollisionSphereEndOverlap);

	BulletCollisionSphere->bReturnMaterialOnMove = true; //hit event return physMat
	BulletCollisionSphere->SetCanEverAffectNavigation(false); //collision not affect on navigation

	RootComponent = BulletCollisionSphere;

	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet projectile mash"));
	BulletMesh->SetupAttachment(RootComponent);
	BulletMesh->SetCanEverAffectNavigation(false);
	
	BulletFXLeg = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BulletLeg FX"));
	BulletFXLeg -> SetupAttachment(RootComponent);
	
	BulletFXNi = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BulletNi FX"));
	BulletFXNi->SetupAttachment(RootComponent);
	

	BulletProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Bullet Projectilemovement"));
	BulletProjectileMovement->UpdatedComponent = RootComponent;
	BulletProjectileMovement->InitialSpeed = 1.f;
	BulletProjectileMovement->MaxSpeed = 0.f;

	BulletProjectileMovement->bRotationFollowsVelocity = true;
	BulletProjectileMovement->bShouldBounce = true;
}

// Called when the game starts or when spawned
void AProjectileDefault::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProjectileDefault::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AProjectileDefault::InitProjectile(FProjectileInfo InitParam)
{
	
	BulletProjectileMovement->InitialSpeed = InitParam.ProjectileInitSpeed;
	BulletProjectileMovement->MaxSpeed = InitParam.ProjectileInitSpeed;
	this->SetLifeSpan(InitParam.ProjectileLifeTime);
	
	ProjectileSetting = InitParam;

	if (InitParam.ProjectileStaticMesh )
		BulletMesh->SetStaticMesh(InitParam.ProjectileStaticMesh);
	else if(BulletMesh->GetStaticMesh() == nullptr)
		BulletMesh->DestroyComponent();
	
	if(InitParam.TrailFXNi || InitParam.TrailFXLeg)
	{
		if (InitParam.TrailFXNi)
			BulletFXNi->SetAsset(InitParam.TrailFXNi);
		else
			BulletFXLeg->SetTemplate(InitParam.TrailFXLeg);
	}
		
	if(BulletFXLeg == nullptr)
		BulletFXLeg->DestroyComponent();
	if(BulletFXNi == nullptr)
		BulletFXNi->DestroyComponent();
}

void AProjectileDefault::BulletCollisionSphereHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
						UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && Hit.PhysMaterial.IsValid())
	{
		EPhysicalSurface mySurfacetype = UGameplayStatics::GetSurfaceType(Hit);
		
			if (ProjectileSetting.HitDecals.Contains(mySurfacetype))
			{
				UMaterialInterface* myMaterial = ProjectileSetting.HitDecals[mySurfacetype];

				if (myMaterial && OtherComp)
					UGameplayStatics::SpawnDecalAttached(myMaterial, FVector(20.0f), OtherComp, NAME_None,
						Hit.ImpactPoint, Hit.Normal.Rotation(), EAttachLocation::KeepWorldPosition, 10.0f);
			}
		if (ProjectileSetting.HitFXLeg.Contains(mySurfacetype) || ProjectileSetting.HitFXNi.Contains(mySurfacetype))
		{
			if (!ProjectileSetting.HitFXNi.IsEmpty())
			{
				UNiagaraSystem* myParticle = ProjectileSetting.HitFXNi[mySurfacetype];
				if(myParticle)
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),
						myParticle, Hit.ImpactPoint, FRotator::ZeroRotator);
			}
			
			if (!ProjectileSetting.HitFXLeg.IsEmpty())
			{
				UParticleSystem* myParticle = ProjectileSetting.HitFXLeg[mySurfacetype];
				if (myParticle)
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), myParticle, FTransform(Hit.ImpactNormal.Rotation(), Hit.ImpactPoint, FVector(1.0f)));
			}
		}
	}
	if (ProjectileSetting.HitSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), ProjectileSetting.HitSound, Hit.ImpactPoint);
	}
	UGameplayStatics::ApplyDamage(OtherActor, ProjectileSetting.ProjectileDamage, GetInstigatorController(), this, NULL);
	ImpactProjectile();
}

void AProjectileDefault::BulletCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void AProjectileDefault::BulletCollisionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AProjectileDefault::ImpactProjectile()
{
	this->Destroy();
}

