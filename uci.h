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
	int hash = 10;
	int elo = 2500;
	int eloMin = 0;
	int eloMax = 2500;
	int lmr = 50;
	int lmrMin = 0;
	int lmrMax = 100;
	int futility = 50;
	int futilityMin = 0;
	int futilityMax = 100;
	int razoring = 50;
	int razoringMin = 0;
	int razoringMax = 100;
	int centrality = 50;
	int centralityMin = 0;
	int centralityMax = 100;
	// size of the aspiration window ( val-ASPITATION, val+ASPIRATION )
	Score aspiration = 50;
};
extern SOptions options;

void UciCommand(string str);
void UciLoop();
