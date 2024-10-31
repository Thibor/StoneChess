#pragma once

#include "types.h"
#include "eval.h"

struct PickerE {
	Move move;
	Value score = 0;
	Value see = 0;
};

class Picker {
public:
	int index = 0;
	int count = 0;
	Move mList[228];
	PickerE pList[228];
	void Fill();
	int GetIndex(Move m);
	PickerE Pick(int index);
	void SetBest(int index);
	bool SetMove(Move m);
	void Sort();
};
