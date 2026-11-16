using UnrealBuildTool;
using System.Collections.Generic;

public class JamGameEditorTarget : TargetRules
{
	public JamGameEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("JamGame");
	}
}
