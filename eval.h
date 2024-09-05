#pragma once

#include "position.h"
#include "program.h"
#include "types.h"
#include "uci.h"

struct SEval {
	int scorePawnPassed = 0;
	int scorePawnBlocked = 0;
	int scorePawnDoubled = 0;
	int scoreBishopPair = 0;
	int scoreKingShelter = 0;
	int scorePiece[6] = {};
};

const int materialMax[] = { 128,421,404, 596, 1271, 0 };
const int phases[] = { 0, 1, 1, 2, 4, 0 };

S32 Eval();
S32 Eval(Move m,Score &see);
void EvalInit();
S32 See(Move m);
S32 ShowEval();
SEval Eval(Position &pos, Color color, Square kpUs, Square kpEn);
