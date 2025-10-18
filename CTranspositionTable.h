#pragma once

#include <cstdint>
#include <ostream>

#include "types.h"
#include "move.h"

enum Bound :U8 {
	BOUND_NONE,
	BOUND_UPPER,
	BOUND_LOWER,
	BOUND_EXACT = BOUND_UPPER | BOUND_LOWER
};

struct CRec {
	Hash hash;//8
	Move move;//2
	S16 score;//2
	S8 depth;//1
	Bound bound;//1 
	U16 age;//2

	Value GetValue() const { return (Value)score; }

	void SetRec(Hash h, Value s, Move m, Bound t, Depth d, U16 a) {
		hash = h;
		score = (S16)s;
		move = m;
		bound = t;
		depth = d;
		age = a;
	}
};

class CTranspositionTable
{
	U64 records = 0;
	U64 mask = 0;
	CRec* rt = NULL;
	int limit = 2;
public:
	U16 age = limit;
	~CTranspositionTable();
	void Clear();
	int Permill() const;
	void Resize(U64 mbSize);
	bool SetRec(Hash hash, Value score, Move move, Bound type, Depth depth);
	CRec* GetRec(Hash hash);

};

extern CTranspositionTable tt;

