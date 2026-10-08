using UnrealBuildTool;

public class KianaCombatDemo : ModuleRules
{
	public KianaCombatDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AnimGraphRuntime",
			"GameplayTasks",
			"NavigationSystem",
			"AIModule",
			"UMG",
			"Slate",
			"SlateCore",
            "Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}

