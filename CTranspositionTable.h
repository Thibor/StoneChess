#pragma once

#include <cstdint>
#include <ostream>

#include "types.h"

enum RecType :U8 {
	NODE_PV = 0,
	NODE_CUT = 1,
	NODE_ALL = 2
};

struct CRec {
	U64 hash;//8
	U16 move;//2
	S16 score;//2
	U8 depth;//1
	RecType type;//1 
	U16 age;//2

	void SetRec(U64 h, S16 s, U16 m, RecType t, U8 d, U16 a) {
		hash = h;
		score = s;
		move = m;
		type = t;
		depth = d;
		age = a;
	}
};

class CTranspositionTable
{
	U64 used;
	U64 size;
	U64 mask;
	CRec* tt;

public:
	U16 age;

	CTranspositionTable() {
		Resize(10);
	}

	~CTranspositionTable() {
		delete tt;
	}

	void Clear() {
		used = 0;
		std::memset(tt, 0, sizeof(CRec) * size);
	}

	int Permill()
	{
		return (int)(used * 1000ul / size);
	}

	void Resize(uint64_t mbSize)
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

	bool SetRec(uint64_t hash, int16_t score, U16 move, RecType type, int depth) {
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

	CRec* GetRec(U64 hash) {
		U64 index = hash & mask;
		CRec* enP = &tt[index];
		if (enP->hash == hash)
			return enP;
		return nullptr;
	}

};

extern CTranspositionTable tt;

