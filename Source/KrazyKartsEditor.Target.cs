// Copyright © 2021 Nikita Kuznetsov. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class KrazyKartsEditorTarget : TargetRules
{
	public KrazyKartsEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("KrazyKarts");
	}
}
