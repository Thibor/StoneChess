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
	int tempo = 0;
	int hash = 32;
	int elo = 2500;
	int eloMin = 0;
	int eloMax = 2500;
	int lmr = 0;
	int futility = 0;
	int razoring = 0;
	int nullMove = 0;
	int rfp = 0;
	int multiPV = 1;
	int aspiration = 0;
	string materialDel = "0 0 0 0 0";
	string outpost = "0 0 0 0 0 0 0 0";
	string centrality = "0 0 0 0 0 0 0 0 0 0 0 0";
	string tropism = "0 0 0 0 0 0 0 0";
	string passed = "0 0 0 0 0";
	string pawn = "0 0 0 0 0 0 0 0";
	string rook = "0 0 0 0";
	string king = "0 0";
	string pair = "0 0 0 0 0 0";
};
extern SOptions options;

void UciCommand(string str);
void UciLoop();
