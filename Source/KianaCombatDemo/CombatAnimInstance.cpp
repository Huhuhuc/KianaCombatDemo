#include "CombatAnimInstance.h"
#include "Animation/AnimSingleNodeInstanceProxy.h"

struct FCombatAnimProxy : FAnimSingleNodeInstanceProxy
{
	FCombatAnimProxy(UAnimInstance* Instance) : FAnimSingleNodeInstanceProxy(Instance) {}
	TArray<FTransform> LastPose;
	TArray<FTransform> BlendFrom;
	float BlendTime = 1.f;
	virtual void SetAnimationAsset(UAnimationAsset* Asset, USkeletalMeshComponent* Mesh, bool Looping, float Rate) override
	{
		BlendFrom = LastPose;
		BlendTime = 0.f;
		FAnimSingleNodeInstanceProxy::SetAnimationAsset(Asset, Mesh, Looping, Rate);
	}
	virtual void UpdateAnimationNode(const FAnimationUpdateContext& Context) override
	{
		BlendTime += Context.GetDeltaTime();
		FAnimSingleNodeInstanceProxy::UpdateAnimationNode(Context);
	}
	virtual bool Evaluate(FPoseContext& Output) override
	{
		const bool Result = FAnimSingleNodeInstanceProxy::Evaluate(Output);
		const int32 Count = Output.Pose.GetNumBones();
		const float T = FMath::Clamp(BlendTime / 0.12f, 0.f, 1.f);
		const float Alpha = T * T * (3.f - 2.f * T);
		LastPose.SetNum(Count);
		for (int32 I = 0; I < Count; ++I)
		{
			FTransform& Bone = Output.Pose[FCompactPoseBoneIndex(I)];
			if (BlendFrom.Num() == Count && T < 1.f)
			{
				FTransform Blended;
				Blended.Blend(BlendFrom[I], Bone, Alpha);
				Bone = Blended;
			}
			LastPose[I] = Bone;
		}
		return Result;
	}
};

FAnimInstanceProxy* UCombatAnimInstance::CreateAnimInstanceProxy() { return new FCombatAnimProxy(this); }
void UCombatAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* Proxy) { delete Proxy; }
