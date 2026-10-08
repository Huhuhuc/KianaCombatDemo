using UnrealBuildTool;
using System.Collections.Generic;

public class KianaCombatDemoEditorTarget : TargetRules
{
	public KianaCombatDemoEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		CppStandard = CppStandardVersion.Cpp20;
		ExtraModuleNames.Add("KianaCombatDemo");
	}
}

