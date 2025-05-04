#include "CTranspositionTable.h"

CTranspositionTable tt;

CTranspositionTable::~CTranspositionTable() {
	delete rt;
}

void CTranspositionTable::Clear() {
	age = limit;
	std::memset(rt, 0, sizeof(CRec) * records);
}

CRec* CTranspositionTable::GetRec(Hash hash) {
	U64 index = hash & mask;
	CRec* rec = &rt[index];
	if ((rec->hash == hash) && (rec->age > age - limit))
		return rec;
	return nullptr;
}

int CTranspositionTable::Permill() const
{
	int pm = 0;
	for (int n = 0; n < 1000; n++) {
		CRec* rec = &rt[n];
		if (rec->age > age - limit)
			pm++;
	}
	return pm;
}

bool CTranspositionTable::SetRec(Hash hash, Value score, Move move, Bound type, Depth depth) {
	U64 index = hash & mask;
	CRec* rec = &rt[index];
	if (rec->age <= age - limit){
		rec->SetRec(hash, score, move, type, depth, age);
		return true;
	}
	else if (
		rec->age != age
		|| (rec->depth < depth)
		|| (rec->depth == depth && (rec-> bound != BOUND_EXACT || type == BOUND_EXACT ))
		//|| type == BOUND_EXACT
		//|| (rec->bound != BOUND_EXACT && rec->depth <= depth)
		) {
		if (move == MOVE_NONE)move = rec->move;
		rec->SetRec(hash, score, move, type, depth, age);
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