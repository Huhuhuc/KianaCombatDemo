#include "CombatHUDWidget.h"
#include "KianaCharacter.h"
#include "StrikeJaeger.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "InputCoreTypes.h"

FOnCombatDamage UCombatHUDWidget::OnCombatDamage;

TSharedRef<SWidget> UCombatHUDWidget::RebuildWidget()
{
 if (WidgetTree->RootWidget) { return Super::RebuildWidget(); }
 auto* Root=WidgetTree->ConstructWidget<UCanvasPanel>();
 WidgetTree->RootWidget=Root;
 auto Place=[Root](UWidget* W,FVector2D Anchor,FVector2D Position,FVector2D Size)
 {
  auto* S=Root->AddChildToCanvas(W); S->SetAnchors(FAnchors(Anchor.X,Anchor.Y));
  S->SetPosition(Position); S->SetSize(Size);
 };
 auto Text=[&](FString Value,FVector2D Anchor,FVector2D Pos,FVector2D Size)
 {
  auto* T=WidgetTree->ConstructWidget<UTextBlock>(); T->SetText(FText::FromString(Value));
  auto Font=T->GetFont(); Font.Size=20; T->SetFont(Font);
  T->SetColorAndOpacity(FSlateColor(FLinearColor::White));
  T->SetShadowOffset(FVector2D(1,1)); Place(T,Anchor,Pos,Size); return T;
 };
 auto* Panel=WidgetTree->ConstructWidget<UBorder>();
 Panel->SetBrushColor(FLinearColor(0.015f,0.02f,0.025f,0.92f));
 Place(Panel,{0,0},{24,24},{454,94});
 auto* Portrait=WidgetTree->ConstructWidget<UImage>();
 Portrait->SetBrushFromTexture(LoadObject<UTexture2D>(nullptr,TEXT("/Game/CombatDemo/UI/T_Portrait.T_Portrait")));
 Place(Portrait,{0,0},{31,31},{80,80});
 PlayerText=Text(TEXT("KIANA"),{0,0},{124,34},{340,32});
 PlayerBar=WidgetTree->ConstructWidget<UProgressBar>();
 PlayerBar->SetFillColorAndOpacity(FLinearColor(0.5f,1.f,0.03f));
 Place(PlayerBar,{0,0},{124,77},{336,20});
 EnemyText=Text(TEXT("STRIKE JAEGER"),{0,0},{0,0},{240,32});
 EnemyText->SetJustification(ETextJustify::Center);
 EnemyBar=WidgetTree->ConstructWidget<UProgressBar>();
 EnemyBar->SetFillColorAndOpacity(FLinearColor(0.95f,0.28f,0.2f));
 Place(EnemyBar,{0,0},{0,0},{180,12});
 for(int32 I=0; I<3; ++I)
 {
  auto* Disc=WidgetTree->ConstructWidget<UBorder>();
  FSlateBrush Brush; Brush.DrawAs=ESlateBrushDrawType::RoundedBox;
  Brush.TintColor=FSlateColor(FLinearColor(0.015f,0.02f,0.025f,0.94f));
  Brush.OutlineSettings.CornerRadii=FVector4(40,40,40,40);
  Brush.OutlineSettings.RoundingType=ESlateBrushRoundingType::FixedRadius;
  Brush.OutlineSettings.Width=3; Brush.OutlineSettings.Color=FSlateColor(I==2 ? FLinearColor(1.f,0.75f,0.02f) : FLinearColor(0.65f,0.7f,0.72f));
  Disc->SetBrush(Brush); Place(Disc,{1,1},{-324.f+I*100,-142},{80,80});
  auto* Icon=WidgetTree->ConstructWidget<UImage>();
  const TCHAR* Paths[]={TEXT("/Game/CombatDemo/UI/T_Attack.T_Attack"),TEXT("/Game/CombatDemo/UI/T_Dodge.T_Dodge"),TEXT("/Game/CombatDemo/UI/T_Skill.T_Skill")};
  Icon->SetBrushFromTexture(LoadObject<UTexture2D>(nullptr,Paths[I]));
  Place(Icon,{1,1},{-310.f+I*100,-128},{52,52});
 }
 SkillText=Text(TEXT("READY"),{1,1},{-144,-183},{120,30});
 SkillText->SetJustification(ETextJustify::Center);
 SkillBar=WidgetTree->ConstructWidget<UProgressBar>();
 SkillBar->SetFillColorAndOpacity(FLinearColor(1.f,0.75f,0.02f));
 Place(SkillBar,{1,1},{-124,-56},{80,6});
 Text(TEXT("LMB/X    Space/A    E/Y"),{1,1},{-324,-40},{310,30});
 ResultText=Text(TEXT(""),{0.5f,0.5f},{-200,-70},{400,45});
 ResultText->SetJustification(ETextJustify::Center);
 RetryButton=WidgetTree->ConstructWidget<UButton>();
 auto* Label=WidgetTree->ConstructWidget<UTextBlock>();
 Label->SetText(FText::FromString(TEXT("RETRY"))); RetryButton->AddChild(Label);
 Place(RetryButton,{0.5f,0.5f},{-90,0},{180,48});
 RetryButton->OnClicked.AddDynamic(this,&UCombatHUDWidget::Retry);
 RetryButton->SetVisibility(ESlateVisibility::Collapsed);
 SetIsFocusable(true);
 WelcomePanel=WidgetTree->ConstructWidget<UCanvasPanel>();
 Place(WelcomePanel,{0,0.5f},{28,-250},{540,500});
 auto* Back=WidgetTree->ConstructWidget<UBorder>();
 Back->SetBrushColor(FLinearColor(0.015f,0.025f,0.04f,0.9f));
 auto* BS=WelcomePanel->AddChildToCanvas(Back);BS->SetSize(FVector2D(540,500));
 const TCHAR* Lines[]={TEXT("战斗准备"),TEXT("WASD / 左摇杆：移动"),TEXT("鼠标 / 右摇杆：视角"),TEXT("左键连点 / 按住 X：普通攻击"),TEXT("空格 / A：闪避   Shift / LB：跑步"),TEXT("E / Y：技能   Tab / R3：锁定"),TEXT("P / Enter / 手柄菜单键：开始战斗")};
 for(int32 I=0;I<7;++I)
 {
  auto* T=WidgetTree->ConstructWidget<UTextBlock>();T->SetText(FText::FromString(Lines[I]));
  auto F=T->GetFont();F.Size=I==0?30:20;T->SetFont(F);
  T->SetColorAndOpacity(FSlateColor(I==0?FLinearColor(0.6f,1.f,0.1f):FLinearColor::White));
  auto* S=WelcomePanel->AddChildToCanvas(T);S->SetPosition(FVector2D(24,20+I*43));S->SetSize(FVector2D(500,40));
 }
 auto* Start=WidgetTree->ConstructWidget<UButton>();
 DifficultyText=WidgetTree->ConstructWidget<UTextBlock>();
 DifficultyText->SetText(FText::FromString(TEXT("当前：简单 · 1 个敌人")));
 DifficultyText->SetColorAndOpacity(FSlateColor(FLinearColor(0.6f,1.f,0.1f)));
 auto* DS=WelcomePanel->AddChildToCanvas(DifficultyText);DS->SetPosition(FVector2D(24,325));DS->SetSize(FVector2D(490,30));
 for(int32 I=0;I<2;++I)
 {
  auto* Button=WidgetTree->ConstructWidget<UButton>();auto* DifficultyLabel=WidgetTree->ConstructWidget<UTextBlock>();
  DifficultyLabel->SetText(FText::FromString(I==0?TEXT("1 / ←  简单 · 1 人"):TEXT("2 / →  困难 · 4 人")));Button->AddChild(DifficultyLabel);
  auto* S=WelcomePanel->AddChildToCanvas(Button);S->SetPosition(FVector2D(24+I*250,365));S->SetSize(FVector2D(240,44));
  if(I==0){Button->OnClicked.AddDynamic(this,&UCombatHUDWidget::SelectEasy);}
  else{Button->OnClicked.AddDynamic(this,&UCombatHUDWidget::SelectHard);}
 }
 auto* LabelStart=WidgetTree->ConstructWidget<UTextBlock>();LabelStart->SetText(FText::FromString(TEXT("开始战斗")));Start->AddChild(LabelStart);
 auto* SS=WelcomePanel->AddChildToCanvas(Start);SS->SetPosition(FVector2D(24,430));SS->SetSize(FVector2D(490,48));
 Start->OnClicked.AddDynamic(this,&UCombatHUDWidget::StartBattle);
 OnCombatDamage.AddWeakLambda(this, [this](FVector WorldPos, float Amount) { SpawnDamageNumber(WorldPos, Amount); });
 return Super::RebuildWidget();
}
void UCombatHUDWidget::SpawnDamageNumber(FVector WorldPos, float Amount)
{
 if (!WidgetTree || !WidgetTree->RootWidget) { return; }
 UCanvasPanel* Root = Cast<UCanvasPanel>(WidgetTree->RootWidget);
 if (!Root) { return; }
 UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>();
 T->SetText(FText::FromString(FString::Printf(TEXT("-%.0f"), Amount)));
 FSlateFontInfo Font = T->GetFont();
 Font.Size = 26;
 T->SetFont(Font);
 T->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.35f, 0.15f)));
 T->SetShadowOffset(FVector2D(1.f, 1.f));
 auto* DmgSlot = Root->AddChildToCanvas(T);
 DmgSlot->SetAutoSize(true);
 FActiveDamageNumber Entry;
 Entry.Text = T;
 Entry.WorldPos = WorldPos;
 ActiveDamageNumbers.Add(Entry);
}

void UCombatHUDWidget::NativeTick(const FGeometry& Geometry,float DeltaTime)
{
 Super::NativeTick(Geometry,DeltaTime);
 auto* Player=Cast<AKianaCharacter>(GetOwningPlayerPawn());
 if(!Player)return;
 if(!bWelcomeInitialized)
 {
  TArray<AStrikeJaeger*> Existing;
  for(TActorIterator<AStrikeJaeger> It(GetWorld());It;++It){Existing.Add(*It);}
  for(auto* Enemy:Existing)
  {
   if(!BattleEnemyClass){BattleEnemyClass=Enemy->GetClass();BattleEnemyOrigin=Enemy->GetActorLocation();}
   Enemy->Destroy();
  }
  bWelcomeInitialized=true;auto* PC=GetOwningPlayer();PC->SetPause(true);PC->bShowMouseCursor=true;
  FInputModeUIOnly Mode;Mode.SetWidgetToFocus(TakeWidget());PC->SetInputMode(Mode);SetKeyboardFocus();
 }
 PlayerBar->SetPercent(Player->GetHealthPercent());
 PlayerText->SetText(FText::FromString(FString::Printf(TEXT("KIANA   %.0f / %.0f"),Player->GetCurrentHealth(),Player->GetMaximumHealth())));
 const float Cooldown=Player->GetSkillCooldown();
 SkillBar->SetPercent(Player->GetSkillReadyPercent());
 SkillText->SetColorAndOpacity(FSlateColor(Cooldown>0 ? FLinearColor(0.6f,0.65f,0.7f) : FLinearColor(1.f,0.8f,0.05f)));
 SkillText->SetText(FText::FromString(Cooldown>0 ? FString::Printf(TEXT("SKILL   %.1f s"),Cooldown) : TEXT("SKILL   READY")));
 AStrikeJaeger* Enemy=nullptr;
 int32 LivingEnemies=0;
 float NearestDistance=TNumericLimits<float>::Max();
 for(TActorIterator<AStrikeJaeger> It(GetWorld());It;++It)
 {
  if(It->IsDead())continue;
  ++LivingEnemies;
  const float Distance=FVector::DistSquared(It->GetActorLocation(),Player->GetActorLocation());
  if(Distance<NearestDistance){Enemy=*It;NearestDistance=Distance;}
 }
 // Each living enemy owns a stable bar; the old label is reserved for the lock.
 for(auto It=EnemyBars.CreateIterator();It;++It)
 { if(!It.Key().IsValid() || It.Key()->IsDead()){It.Value()->RemoveFromParent();It.RemoveCurrent();} }
 for(TActorIterator<AStrikeJaeger> It(GetWorld());It;++It)
 {
  if(It->IsDead())continue;
  UProgressBar*& Bar=EnemyBars.FindOrAdd(*It);
  if(!Bar)
  {
   Bar=WidgetTree->ConstructWidget<UProgressBar>();
   auto* BarSlot=Cast<UCanvasPanel>(WidgetTree->RootWidget)->AddChildToCanvas(Bar);
   BarSlot->SetSize(FVector2D(130,9));
   Bar->SetVisibility(ESlateVisibility::HitTestInvisible);
  }
  Bar->SetPercent(It->GetHealthPercent());
  Bar->SetFillColorAndOpacity(Player->GetLockedEnemy()==*It?FLinearColor(1,.8f,.1f):FLinearColor(.95f,.28f,.2f));
  FVector2D Pos;
  const bool OnScreen=UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayer(),It->GetActorLocation()+FVector(0,0,130),Pos,true);
  const FVector2D Size=Geometry.GetLocalSize();
  const bool InBounds=OnScreen && Pos.X>=65 && Pos.Y>=0 && Pos.X<=Size.X-65 && Pos.Y<=Size.Y;
  Bar->SetVisibility(InBounds?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);
  if(InBounds)Cast<UCanvasPanelSlot>(Bar->Slot)->SetPosition(Pos-FVector2D(65,0));
 }
 Enemy=Player->GetLockedEnemy();
 EnemyBar->SetPercent(Enemy ? Enemy->GetHealthPercent() : 0.f);
 FVector2D Screen;
 const bool Visible=Enemy && !Enemy->IsDead() && UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayer(),Enemy->GetActorLocation()+FVector(0,0,130),Screen,true);
 EnemyBar->SetVisibility(Visible?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);
 EnemyText->SetVisibility(Visible?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);
 if(Visible)
 {
  Cast<UCanvasPanelSlot>(EnemyBar->Slot)->SetPosition(Screen-FVector2D(90,0));
  Cast<UCanvasPanelSlot>(EnemyText->Slot)->SetPosition(Screen-FVector2D(120,30));
 }
 EnemyText->SetText(FText::FromString(Enemy ? FString::Printf(TEXT("STRIKE JAEGER   %.0f%%"),Enemy->GetHealthPercent()*100.f) : TEXT("NO TARGET")));
 EnemyBar->SetVisibility(ESlateVisibility::Collapsed);
 for (int32 I = ActiveDamageNumbers.Num() - 1; I >= 0; --I)
 {
  FActiveDamageNumber& D = ActiveDamageNumbers[I];
  D.Age += DeltaTime;
  FVector2D DamageScreen;
  if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayer(), D.WorldPos + FVector(0.f, 0.f, 60.f), DamageScreen, true) && D.Text)
  {
   if (auto* DmgSlot = Cast<UCanvasPanelSlot>(D.Text->Slot)) { DmgSlot->SetPosition(DamageScreen + FVector2D(0.f, -D.Age * 55.f)); }
   D.Text->SetOpacity(FMath::Clamp(1.f - D.Age / D.Lifetime, 0.f, 1.f));
  }
  if (D.Age >= D.Lifetime)
  {
   if (D.Text) { D.Text->RemoveFromParent(); }
   ActiveDamageNumbers.RemoveAt(I);
  }
 }
 const bool End=!bWaitingToStart && (Player->IsDead() || (bEnemiesSpawned && LivingEnemies==0));
 if(End && !bFinished)
 {
  bFinished=true; ResultText->SetText(FText::FromString(Player->IsDead()?TEXT("DEFEAT"):TEXT("VICTORY")));
  RetryButton->SetVisibility(ESlateVisibility::Visible);
  auto* PC=GetOwningPlayer(); Player->DisableInput(PC);
  PC->bShowMouseCursor=true; FInputModeUIOnly Mode; Mode.SetWidgetToFocus(RetryButton->TakeWidget()); PC->SetInputMode(Mode);
  RetryButton->SetKeyboardFocus();
 }
}
void UCombatHUDWidget::Retry()
{
 if(auto* PC=GetOwningPlayer()){PC->bShowMouseCursor=false;PC->SetInputMode(FInputModeGameOnly());}
 UGameplayStatics::OpenLevel(this,FName(*UGameplayStatics::GetCurrentLevelName(this,true)));
}

FReply UCombatHUDWidget::NativeOnKeyDown(const FGeometry& Geometry,const FKeyEvent& Event)
{
 const FKey Key=Event.GetKey();
 if(bWaitingToStart && (Key==EKeys::One || Key==EKeys::Left || Key==EKeys::Gamepad_DPad_Left))
 {SelectEasy();return FReply::Handled();}
 if(bWaitingToStart && (Key==EKeys::Two || Key==EKeys::Right || Key==EKeys::Gamepad_DPad_Right))
 {SelectHard();return FReply::Handled();}
 if(bWaitingToStart && (Key==EKeys::P || Key==EKeys::Enter || Key==EKeys::Gamepad_Special_Right))
 {StartBattle();return FReply::Handled();}
 return Super::NativeOnKeyDown(Geometry,Event);
}
void UCombatHUDWidget::StartBattle()
{
 if(!bWaitingToStart)return;
 if(!BattleEnemyClass)return;
 auto* Player=GetOwningPlayerPawn();
 if(!Player)return;
 const FVector Forward=Player->GetActorForwardVector();
 const FVector Right=Player->GetActorRightVector();
 TArray<AStrikeJaeger*> Spawned;
 for(int32 I=0;I<SelectedEnemyCount;++I)
 {
  FVector Location=Player->GetActorLocation()+Forward*(I<2?1000.f:1450.f)+Right*(I%2==0?-650.f:650.f);
  if(SelectedEnemyCount==1){Location=Player->GetActorLocation()+Forward*700.f;}
  Location.Z=BattleEnemyOrigin.Z;
  FActorSpawnParameters Params;
  Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
  auto* Enemy=GetWorld()->SpawnActor<AStrikeJaeger>(BattleEnemyClass,Location,(Player->GetActorLocation()-Location).Rotation(),Params);
  if(Enemy)Spawned.Add(Enemy);
 }
 if(Spawned.Num()!=SelectedEnemyCount)
 {
  for(auto* Enemy:Spawned)Enemy->Destroy();
  UE_LOG(LogTemp,Error,TEXT("Battle start failed: could not place selected enemies safely"));
  return;
 }
 bEnemiesSpawned=true;
 bWaitingToStart=false;WelcomePanel->SetVisibility(ESlateVisibility::Collapsed);
 if(auto* PC=GetOwningPlayer())
 {PC->FlushPressedKeys();PC->SetPause(false);PC->bShowMouseCursor=false;PC->SetInputMode(FInputModeGameOnly());}
}

void UCombatHUDWidget::SelectEasy()
{
 if(!bWaitingToStart)return;
 SelectedEnemyCount=1;DifficultyText->SetText(FText::FromString(TEXT("当前：简单 · 1 个敌人")));SetKeyboardFocus();
}
void UCombatHUDWidget::SelectHard()
{
 if(!bWaitingToStart)return;
 SelectedEnemyCount=4;DifficultyText->SetText(FText::FromString(TEXT("当前：困难 · 4 个敌人")));SetKeyboardFocus();
}
