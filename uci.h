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

	/*int aspiration = 37;
	int contempt = 29;
	int futility = 509;
	int lmr = 184;
	int nullMove = 926;
	int razoring = 610;
	int tempo = 21;
	string king = "52 39";
	string material = "46 11 22 -34 33";
	string mobility = "6 4 8 5 -3 -7 -2 4";
	string outFile = "-1 -8 -6 -2 -3 -6 -3 -2 -4 -1 16 -16";
	string outpost = "8 0 11 4";
	string outRank = "0 8 -17 -1 -16 -6 5 4 -10 11 16 -16";
	string pair = "15 -11 65 -23 20 15";
	string passed = "-5 122 -49 13 -7";
	string pawn = "4 7 -25 -30 -8 -21 -10 -2";
	string rook = "45 2 29 11";
	string tropism = "4 -3 1 -1 2 -2 2 -2";
	*/

	int aspiration = 36;
	int contempt = 31;
	int futility = 748;
	int lmr = 183;
	int nullMove = 941;
	int razoring = 529;
	int tempo = 28;
	string king = "52 39";
	string material = "50 3 22 -33 89";
	string mobility = "2 4 10 5 -3 -7 -2 4";
	string outFile = "-1 -8 -6 -2 5 -10 -3 -6 -4 -1 15 -17";
	string outpost = "80 -8 11 4";
	string outRank = "0 8 -19 -3 -17 -7 5 4 -9 11 16 -16";
	string pair = "18 -11 65 -23 20 15";
	string passed = "-5 121 -49 14 -7";
	string pawn = "3 7 -25 -30 -8 -21 -10 -1";
	string rook = "45 69 29 11";
	string tropism = "4 2 1 -1 6 -2 1 -2";

};
extern SOptions options;

void UciCommand(string str);
void UciLoop();
