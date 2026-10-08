using UnrealBuildTool;
using System.Collections.Generic;

public class KianaCombatDemoTarget : TargetRules
{
	public KianaCombatDemoTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		CppStandard = CppStandardVersion.Cpp20;
		ExtraModuleNames.Add("KianaCombatDemo");
	}
}

