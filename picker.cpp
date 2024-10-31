#include "picker.h"

void Picker::Fill() {
	index = 0;
	for (int n = 0; n < count; n++) {
		Move m = mList[n];
		pList[n].move = m;
		pList[n].score = Eval(m,pList[n].see);
	}
}

PickerE Picker::Pick(int index) {
	int bstI = index;
	Value bstS = pList[bstI].score;
	for (int n = index + 1; n < count; n++) {
		Value curS = pList[n].score;
		if (bstS < curS) {
			bstS = curS;
			bstI = n;
		}
	}
	if (index != bstI) {
		PickerE e = pList[index];
		pList[index] = pList[bstI];
		pList[bstI] = e;
	}
	return pList[index];
}

void Picker::SetBest(int index) {
	PickerE e = pList[index];
	for (int i = index; i > 0; i--)
		pList[i] = pList[i - 1];
	pList[0] = e;
}

void Picker::Sort() {
	for (int n = 0; n < count - 1; n++)
		Pick(n);
}

int Picker::GetIndex(Move m) {
	for (int n = 0; n < count; n++)
		if (pList[n].move == m)
			return n;
	return -1;
}

bool Picker::SetMove(Move m) {
	int i = GetIndex(m);
	sd.moveSet++;
	if (i < 0)
		return false;
	sd.moveOk++;
	index++;
	for (int n = i; n > 0; n--)
		pList[n] = pList[n - 1];
	pList[0].move = m;
	return true;
}