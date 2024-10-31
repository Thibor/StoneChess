#pragma once

#include <cstdint>
#include <ostream>

#include "types.h"

enum RecType :U8 {
	NODE_PV = 0,
	NODE_BETA = 1,
	NODE_ALPHA = 2
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
	U64 records;
	U64 mask;
	CRec* rt;
public:
	U16 age;
	CTranspositionTable();
	~CTranspositionTable();
	void Clear();
	int Permill() const;
	void Resize(U64 mbSize);
	bool SetRec(Hash hash, Value score, U16 move, RecType type, Depth depth);
	CRec* GetRec(Hash hash);

};

extern CTranspositionTable tt;

