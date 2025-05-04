#include "picker.h"

void Picker::Fill() {
	best = 0;
	for (int n = 0; n < count; n++) {
		Move m = mList[n];
		pList[n].move = m;
		pList[n].value = Eval(m);
	}
}

PickerE Picker::Pick(int index) {
	if (index >= best) {
		int bestI = index;
		Value bestV = pList[bestI].value;
		for (int n = index + 1; n < count; n++) {
			Value curV = pList[n].value;
			if (bestV < curV) {
				bestV = curV;
				bestI = n;
			}
		}
		if (index != bestI) {
			PickerE e = pList[index];
			pList[index] = pList[bestI];
			pList[bestI] = e;
		}
	}
	return pList[index];
}

bool Picker::SetBest(Move m) {
	if (m == MOVE_NONE)
		return false;
	sd.moveSet++;
	for (int n = best; n < count; n++)
		if (pList[n].move == m) {
			PickerE pe = pList[n];
			pList[n]=pList[best];
			pList[best++]=pe;
			sd.moveOk++;
			return true;
		}
	return false;
}