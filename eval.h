#pragma once

#include "position.h"
#include "program.h"
#include "types.h"
#include "uci.h"

struct SEval {
	DScore scorePawnPassed = 0;
	DScore scorePawnBlocked = 0;
	DScore scorePawnDoubled = 0;
	DScore scoreBishopPair = 0;
	DScore scoreKingShelter = 0;
	DScore scorePiece[6] = {};
};

Score Eval();
Score Eval(Move m,Score &see);
void InitEval();
Score See(Move m);
Score ShowEval();
SEval Eval(Position &pos, Color color, Square kpUs, Square kpEn);
