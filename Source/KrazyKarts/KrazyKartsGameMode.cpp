// Copyright © 2021 Nikita Kuznetsov. All Rights Reserved.

#include "KrazyKartsGameMode.h"
#include "KrazyKartsPawn.h"
#include "KrazyKartsHud.h"

AKrazyKartsGameMode::AKrazyKartsGameMode()
{
	DefaultPawnClass = AKrazyKartsPawn::StaticClass();
	HUDClass = AKrazyKartsHud::StaticClass();
}
