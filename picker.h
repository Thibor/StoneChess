#pragma once

#include "types.h"
#include "eval.h"

struct PickerE {
	Move move;
	Score score = 0;
	Score see = 0;
};

class Picker {
public:
	int index = 0;
	int count = 0;
	Move list[228];
	PickerE scores[228];
	void Fill(int phase);
	int GetIndex(Move m);
	PickerE Pick(int index);
	void SetBest(int index);
	bool SetMove(Move m);
	void Sort();
};
