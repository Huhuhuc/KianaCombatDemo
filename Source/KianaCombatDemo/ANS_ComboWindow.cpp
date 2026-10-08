#include "ANS_ComboWindow.h"
#include "KianaCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UANS_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (AKianaCharacter* Kiana = Cast<AKianaCharacter>(MeshComp ? MeshComp->GetOwner() : nullptr))
	{
		Kiana->OnComboWindowBegin();
	}
}

void UANS_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (AKianaCharacter* Kiana = Cast<AKianaCharacter>(MeshComp ? MeshComp->GetOwner() : nullptr))
	{
		Kiana->OnComboWindowEnd();
	}
}

