#pragma once

#include "chronos.h"
#include "CTranspositionTable.h"
#include "position.h"
#include "search.h"
#include "util.h"

struct SOptions{
	bool ponder = true;
	int hash = 32;
	int elo = 2500;
	int eloMin = 0;
	int eloMax = 2500;
	int multiPV = 1;

	int aspiration = 36;
	int futility = 748;
	int lmr = 183;
	int nullMove = 941;
	int razoring = 529;
	int tempo = 16;

	string bishop = "32 56 -4 -4";
	string king = "52 39";
	string material = "-27 13 22 -34 33";
	string mobility = "8 5 7 7 3 5 3 2";
	string outFile = "2 -6 -3 -5 6 -4 -6 -2 -4 -1 12 -15";
	string outpost = "80 8 11 4";
	string outRank = "1 -7 -17 1 -17 2 5 5 -10 11 16 -22";
	string passed = "-5 8 -49 -4 4";
	string pawn = "3 7 -24 -26 -8 -21 -10 -1";
	string pawnProtection = "11 14 11 20 -6 18 -3 13 -5 17 -46 20";
	string rook = "72 1 30 12";


};
extern SOptions options;

void UciCommand(string str);
void UciLoop();
