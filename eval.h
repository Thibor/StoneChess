#pragma once

#include "position.h"
#include "program.h"
#include "types.h"
#include "uci.h"
#include "move.h"

struct SEvalSide {
	bool chance = false;
	int piece[PT_NB] = {};
	Color color = COLOR_NB;
	Square king = SQUARE_NB;
};

void InitEval();
Value Eval();
Value Eval(Move m);
Value ShowEval();
