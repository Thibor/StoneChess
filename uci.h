#pragma once

#include "chronos.h"
#include "CTranspositionTable.h"
#include "position.h"
#include "search.h"
#include "util.h"

struct SOptions
{
	bool ponder = true;
	Score contempt = 0;
	int elo = 2500;
	int eloMin = 0;
	int eloMax = 2500;
	int lmr = 100;
	int lmrMin = 50;
	int lmrMax = 150;
	// size of the aspiration window ( val-ASPITATION, val+ASPIRATION )
	Score aspiration = 50; 
};
extern SOptions options;
extern SOptions optionsOrg;

void UciCommand(string str);
void UciLoop();
