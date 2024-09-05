#include "picker.h"

void Picker::Fill() {
	index = 0;
	for (int n = 0; n < count; n++) {
		Move m = list[n];
		scores[n].move = m;
		scores[n].score = Eval(m,scores[n].see);
	}
}

PickerE Picker::Pick(int index) {
	int bstI = index;
	S16 bstS = scores[bstI].score;
	for (int n = index + 1; n < count; n++) {
		S16 curS = scores[n].score;
		if (bstS < curS) {
			bstS = curS;
			bstI = n;
		}
	}
	if (index != bstI) {
		PickerE e = scores[index];
		scores[index] = scores[bstI];
		scores[bstI] = e;
	}
	return scores[index];
}

void Picker::SetBest(int index) {
	PickerE e = scores[index];
	for (int i = index; i > 0; i--)
		scores[i] = scores[i - 1];
	scores[0] = e;
}

void Picker::Sort() {
	for (int n = 0; n < count - 1; n++)
		Pick(n);
}

int Picker::GetIndex(Move m) {
	for (int n = 0; n < count; n++)
		if (scores[n].move == m)
			return n;
	return -1;
}

bool Picker::SetMove(Move m) {
	int i = GetIndex(m);
	sd.moveSetTry++;
	if (i < 0)
		return false;
	sd.moveSetOk++;
	index++;
	for (int n = i; n > 0; n--)
		scores[n] = scores[n - 1];
	scores[0].move = m;
	return true;
}