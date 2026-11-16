using UnrealBuildTool;

public class JamGame : ModuleRules
{
	public JamGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Niagara",
			"RenderCore",
			"SlateCore"
		});

		// NVIDIA DLSS Super Resolution (Plugins/DLSS): its Blueprint library tells whether the card runs DLSS and turns it on.
		PrivateDependencyModuleNames.Add("DLSSBlueprint");

		PublicIncludePaths.AddRange(new string[] {
			"JamGame"
		});
	}
}
