#pragma once

#include "position.h"
#include "program.h"
#include "types.h"
#include "uci.h"

S32 Eval();
S32 Eval(Move m,bool q=false);
void EvalInit();
