#pragma once

#include "chronos.h"
#include "CTranspositionTable.h"
#include "position.h"
#include "search.h"
#include "util.h"

struct SOptions
{
	bool ponder = true;
	int contempt = 0;
	int aspiration = 50;  // size of the aspiration window ( val-ASPITATION, val+ASPIRATION )
	int elo = 2500;
	int eloMin = 0;
	int eloMax = 2500;
};
extern SOptions options;
extern SOptions optionsOrg;

void UciCommand(string str);
void UciLoop();
