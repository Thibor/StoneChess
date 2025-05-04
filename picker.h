#pragma once

#include "types.h"
#include "eval.h"

struct PickerE {
	Move move = MOVE_NONE;
	Value value = VALUE_ZERO;
};

class Picker {
public:
	int best = 0;
	int count = 0;
	Move mList[228];
	PickerE pList[228];
	void Fill();
	PickerE Pick(int index);
	bool SetBest(Move);
};
