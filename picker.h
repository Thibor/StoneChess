#pragma once

#include "types.h"
#include "eval.h"

struct PickerE {
	Move move;
	S16 score=0;
};

class Picker {
public:
	int count = 0;
	Move list[228];
	PickerE scores[228];
	void Fill(bool q=false);
	int GetIndex(Move m);
	PickerE Pick(int index);
	void SetBest(int index);
	bool SetMove(Move m);
	void Sort();
};
