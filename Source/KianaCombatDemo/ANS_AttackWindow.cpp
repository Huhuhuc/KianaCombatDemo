#include "ANS_AttackWindow.h"
#include "KianaCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UANS_AttackWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (AKianaCharacter* Kiana = Cast<AKianaCharacter>(MeshComp ? MeshComp->GetOwner() : nullptr))
	{
		Kiana->OnAttackWindowBegin();
	}
}

void UANS_AttackWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (AKianaCharacter* Kiana = Cast<AKianaCharacter>(MeshComp ? MeshComp->GetOwner() : nullptr))
	{
		Kiana->OnAttackWindowEnd();
	}
}

