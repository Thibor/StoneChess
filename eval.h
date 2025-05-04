#pragma once

#include "position.h"
#include "program.h"
#include "types.h"
#include "uci.h"
#include "move.h"

extern Score contempt;
extern Value tempo;

struct SEvalSide {
	bool chance = false;
	Color color = COLOR_NB;
	Square king = SQUARE_NB;
	int piece[PT_NB] = {};
	Score score = SCORE_ZERO;
	Score scorePawnConnected = SCORE_ZERO;
	Score scorePawnDoubled = SCORE_ZERO;
	Score scorePawnIsolated = SCORE_ZERO;
	Score scorePawnBackward = SCORE_ZERO;
	Score scorePawnPassed = SCORE_ZERO;
	Score scoreMobility = SCORE_ZERO;
	Score scorePair = SCORE_ZERO;
	Score scoreTropism = SCORE_ZERO;
	Score scorePiece[PT_NB] = {};
};

void InitEval();
Value Eval();
Value Eval(Move m);
Value ShowEval();
