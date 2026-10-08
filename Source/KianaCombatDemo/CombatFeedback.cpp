#include "CombatFeedback.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/PrimitiveComponent.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
static TAutoConsoleVariable<int32> CVarCombatVFX(TEXT("combat.VFX"),1,TEXT("Enable minimal combat feedback."));
ACombatFeedback::ACombatFeedback()
{
 PrimaryActorTick.bCanEverTick=true;
 Effect=CreateDefaultSubobject<UNiagaraComponent>(TEXT("Effect"));
 SetRootComponent(Effect);
 Effect->SetAutoActivate(false);
 SetActorEnableCollision(false);
 if(HasAnyFlags(RF_ClassDefaultObject)) for(const TCHAR* Kind:{TEXT("Hit"),TEXT("Skill"),TEXT("Dodge")})
 {
  const FString Path=FString::Printf(TEXT("/Game/CombatDemo/VFX/NS_%s.NS_%s"),Kind,Kind);
  PreloadedSystems.Add(LoadObject<UNiagaraSystem>(nullptr,*Path));
 }
}
void ACombatFeedback::Finished(UNiagaraComponent*) { Destroy(); }
void ACombatFeedback::Tick(float DeltaSeconds)
{
 Super::Tick(DeltaSeconds);
 EmissionTime+=DeltaSeconds;
 if(!bEmissionStopped && EmissionTime>=StopEmissionAt){bEmissionStopped=true;Effect->Deactivate();}
}
void ACombatFeedback::Spawn(UWorld* World,FName Kind,const FVector& Position,const FRotator& Rotation,float Scale)
{
 if(!World || CVarCombatVFX.GetValueOnGameThread()==0) return;
 int32 Count=0;
 for(TActorIterator<ACombatFeedback> It(World);It;++It) if(++Count>=24) return;
 const auto* Defaults=GetDefault<ACombatFeedback>();
 const int32 Index=Kind==TEXT("Hit")?0:(Kind==TEXT("Skill")?1:2);
 auto* System=Defaults->PreloadedSystems[Index].Get();
 if(!System) return;
 auto* FX=World->SpawnActor<ACombatFeedback>(Position,Rotation);
 if(!FX) return;
 FX->SetActorScale3D(FVector(Scale));
 FX->Effect->SetAsset(System);
 FX->StopEmissionAt=Kind==TEXT("Skill")?.2f:.08f;
 FX->Effect->OnSystemFinished.AddDynamic(FX,&ACombatFeedback::Finished);
 FX->Effect->Activate(true);
 FX->SetLifeSpan(5.f); // watchdog only; normal destruction follows particle completion
}
void ACombatFeedback::Hit(AActor* Target,AActor* Source)
{
 if(!IsValid(Target)) return;
 FVector Position=Target->GetActorLocation();
 FVector Direction=IsValid(Source)?(Position-Source->GetActorLocation()).GetSafeNormal():FVector::UpVector;
 // The damage system uses overlaps. Use the collision surface as an approximate
 // contact, rather than claiming an exact weapon/bone impact point.
 if(IsValid(Source)) if(auto* Shape=Cast<UPrimitiveComponent>(Target->GetRootComponent()))
 { FVector Closest; if(Shape->GetClosestPointOnCollision(Source->GetActorLocation(),Closest)>0.f) Position=Closest; }
 Spawn(Target->GetWorld(),TEXT("Hit"),Position,Direction.Rotation(),0.25f);
}
