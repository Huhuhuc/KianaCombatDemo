#pragma once
#include "Animation/AnimSingleNodeInstance.h"
#include "CombatAnimInstance.generated.h"

// Keeps the existing sequence timing while smoothing transitions in local pose space.
UCLASS(Transient)
class KIANACOMBATDEMO_API UCombatAnimInstance : public UAnimSingleNodeInstance
{
	GENERATED_BODY()
public:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* Proxy) override;
};
