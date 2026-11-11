using UnrealBuildTool;

public class CapiRun : ModuleRules
{
	public CapiRun(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore",
			"GeometryScriptingCore", "DynamicMesh", "GeometryFramework"
		});
	}
}
