#include "CTranspositionTable.h"

#include "types.h"

CTranspositionTable tt;

CTranspositionTable::CTranspositionTable() {
	Resize(10);
}

CTranspositionTable::~CTranspositionTable() {
	delete tt;
}

void CTranspositionTable::Clear() {
	used = 0;
	memset(tt, 0, sizeof(CRec) * size);
}

CRec* CTranspositionTable:: GetRec(Hash hash) {
	U64 index = hash & mask;
	CRec* enP = &tt[index];
	if (enP->hash == hash)
		return enP;
	return nullptr;
}

int CTranspositionTable::Permill()
{
	return (int)(used * 1000ul / size);
}

bool CTranspositionTable::SetRec(Hash hash, Score score, U16 move, RecType type, Depth depth) {
	U64 index = hash & mask;
	CRec* enP = &tt[index];
	if (!enP->hash) {
		enP->SetRec(hash, score, move, type, depth, age);
		used++;
		return true;
	}
	else if (
		enP->age != age
		|| type == NODE_PV
		|| (enP->type != NODE_PV && enP->depth <= depth)) {
		enP->SetRec(hash, score, move, type, depth, age);
		return true;
	}
	return false;
}

void CTranspositionTable::Resize(U64 mbSize)
{
	size = 1;
	while (size <= mbSize)
		size <<= 1;
	size = static_cast<uint64_t>(size << 20) / sizeof(CRec);
	mask = size - 1;
	free(tt);
	tt = (CRec*)calloc(size, sizeof(CRec));
	Clear();
}