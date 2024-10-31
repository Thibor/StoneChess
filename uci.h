#pragma once

#include "chronos.h"
#include "CTranspositionTable.h"
#include "position.h"
#include "search.h"
#include "util.h"

struct SOptions
{
	bool ponder = true;
	Value contempt = 0;
	int hash = 10;
	int elo = 2500;
	int eloMin = 0;
	int eloMax = 2500;
	int lmr = 0;
	int futility = 0;
	int razoring = 0;
	int nullMove = 0;
	int rfp = 0;
	// size of the aspiration window ( val-ASPITATION, val+ASPIRATION )
	Value aspiration = 50;
	string materialDel = "0 0 0 0 0";
	string outpost = "0 0 0 0 0 0 0 0";
	string outsideRank = "0 0 0 0 0 0 0 0 0 0 0 0";
	string outsideFile = "0 0 0 0 0 0 0 0 0 0 0 0";
	string mobility = "0 0 0 0 0 0 0 0 0 0 0 0";
	string passed = "0 0 0 0 0";
	string pawn = "0 0 0 0 0 0";
	string rook = "0 0 0 0";
	string king = "0 0";
	string pair = "0 0 0 0 0 0";
	string tempo = "0 0";
};
extern SOptions options;

void UciCommand(string str);
void UciLoop();
