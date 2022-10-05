#pragma once

#include "MusicService.h"

// TODO move this sort of global costume state somewhere logical...
static bool altMode = false;

void setupButtons(MusicService** iMusicService);
void checkButtons();
