#include "CTranspositionTable.h"

#include "types.h"
#include "uci.h"

SOptions options;
CTranspositionTable tt;

CTranspositionTable::CTranspositionTable() {
	Resize(options.hash);
}

CTranspositionTable::~CTranspositionTable() {
	delete rt;
}

void CTranspositionTable::Clear() {
	used = 0;
	std::memset(rt, 0, sizeof(CRec) * records);
}

CRec* CTranspositionTable::GetRec(Hash hash) {
	U64 index = hash & mask;
	CRec* enP = &rt[index];
	if (enP->hash == hash)
		return enP;
	return nullptr;
}

int CTranspositionTable::Permill() const
{
	return (int)(used * 1000ul / records);
}

bool CTranspositionTable::SetRec(Hash hash, Value score, U16 move, RecType type, Depth depth) {
	U64 index = hash & mask;
	CRec* enP = &rt[index];
	if (!enP->hash) {
		enP->SetRec(hash, score, move, type, depth, age);
		used++;
		return true;
	}
	else if (
		enP->age != age
		|| !enP->move
		|| type == NODE_PV
		|| (enP->type != NODE_PV && enP->depth <= depth)) {
		if (!move)move = enP->move;
		enP->SetRec(hash, score, move, type, depth, age);
		return true;
	}
	return false;
}

void CTranspositionTable::Resize(U64 mbSize)
{
	if (mbSize < 1)
		mbSize = 1;
	records = 1;
	U64 recSize = sizeof(CRec);
	while (records * recSize <= mbSize * 1000000)
		records <<= 1;
	mask = records - 1;
	free(rt);
	rt = (CRec*)calloc(records, recSize);
	Clear();
}