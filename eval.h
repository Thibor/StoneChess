#pragma once

#include "position.h"
#include "program.h"
#include "types.h"
#include "uci.h"

struct SEvalSide {
	bool chance = false;
	Color color = COLOR_NB;
	Square king = SQUARE_NB;
	int piece[PT_NB] = {};
	Score score = SCORE_ZERO;
	Score scorePawnConnected = SCORE_ZERO;
	Score scorePawnDoubled = SCORE_ZERO;
	Score scorePawnIsolated = SCORE_ZERO;
	Score scorePawnPassed = SCORE_ZERO;
	Score scoreMobility = SCORE_ZERO;
	Score scorePair = SCORE_ZERO;
	Score scorePiece[PT_NB] = {};
};

extern Value materialMax[];
void InitEval();
Value Eval();
Value Eval(Move m,Value &see);
Value See(Move m);
Value ShowEval();
