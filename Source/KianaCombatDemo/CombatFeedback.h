#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatFeedback.generated.h"
class UNiagaraComponent;
UCLASS()
class KIANACOMBATDEMO_API ACombatFeedback : public AActor
{
 GENERATED_BODY()
public:
 ACombatFeedback();
 virtual void Tick(float DeltaSeconds) override;
 UFUNCTION() void Finished(UNiagaraComponent* Component);
 float EmissionTime=0.f;
 float StopEmissionAt=.12f;
 bool bEmissionStopped=false;
 UPROPERTY() TArray<TObjectPtr<class UNiagaraSystem>> PreloadedSystems;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UNiagaraComponent> Effect;
 static void Spawn(UWorld* World, FName Kind, const FVector& Position, const FRotator& Rotation, float Scale);
 static void Hit(AActor* Target, AActor* Source);
};
