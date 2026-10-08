#pragma once
#include "Blueprint/UserWidget.h"
#include "CombatHUDWidget.generated.h"
class UProgressBar;
class UTextBlock;
class UButton;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCombatDamage, FVector, float);
UCLASS()
class KIANACOMBATDEMO_API UCombatHUDWidget : public UUserWidget
{
 GENERATED_BODY()
public:
 static FOnCombatDamage OnCombatDamage;
 void SpawnDamageNumber(FVector WorldPos, float Amount);
protected:
 virtual FReply NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
 UFUNCTION() void StartBattle();
 UFUNCTION() void SelectEasy();
 UFUNCTION() void SelectHard();
 UPROPERTY() TObjectPtr<UTextBlock> DifficultyText;
 int32 SelectedEnemyCount = 1;
 UPROPERTY() TObjectPtr<class UCanvasPanel> WelcomePanel;
 bool bWaitingToStart = true;
 bool bWelcomeInitialized = false;
 UPROPERTY() TSubclassOf<class AStrikeJaeger> BattleEnemyClass;
 FVector BattleEnemyOrigin = FVector::ZeroVector;
 bool bEnemiesSpawned = false;
 virtual TSharedRef<SWidget> RebuildWidget() override;
 virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
 UFUNCTION() void Retry();
 UPROPERTY() TObjectPtr<UProgressBar> PlayerBar;
 UPROPERTY() TObjectPtr<UProgressBar> EnemyBar;
 UPROPERTY() TObjectPtr<UTextBlock> PlayerText;
 UPROPERTY() TObjectPtr<UTextBlock> EnemyText;
 UPROPERTY() TObjectPtr<UTextBlock> SkillText;
 UPROPERTY() TObjectPtr<UTextBlock> ResultText;
 UPROPERTY() TObjectPtr<UButton> RetryButton;
 UPROPERTY() TObjectPtr<UProgressBar> SkillBar;
 bool bFinished = false;
 TMap<TWeakObjectPtr<class AStrikeJaeger>,UProgressBar*> EnemyBars;
 struct FActiveDamageNumber
 {
  UTextBlock* Text = nullptr;
  FVector WorldPos = FVector::ZeroVector;
  float Age = 0.f;
  float Lifetime = 0.9f;
 };
 TArray<FActiveDamageNumber> ActiveDamageNumbers;
};
