#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <algorithm>
#include <vector> 

#include "position.h"
#include "tables.h"
#include "types.h"
#include "search.h"

#define NAME "StoneChess"
#define VERSION "2025-09-30"

struct PickerE {
	Move move = MOVE_NONE;
	Value value = VALUE_ZERO;
};

class Picker {
public:
	int best = 0;
	int count = 0;
	Move mList[228];
	PickerE pList[228];
	void Fill();
	PickerE Pick(int index);
	bool SetBest(Move);
};

struct SEvalSide {
	bool chance = false;
	int piece[PT_NB] = {};
	Color color = COLOR_NB;
	Square king = SQUARE_NB;
};

enum etimef {
	FTIME = 1,
	FINC = 2,
	FMOVESTOGO = 4,
	FDEPTH = 8,
	FNODES = 16,
	FMATE = 32,
	FMOVETIME = 64,
	FINFINITE = 128
};

struct schronos {
	bool gameOver = false;
	bool ponder = false;
	bool post = true;
	int multiPv = 1;
	int time[2] = {};
	int inc[2] = {};
	int movestogo = 0;
	int depth = 0;
	int nodes = 0;
	int movetime = 0;
	int flags = 0;
	std::vector<Move> rootMoves;

	void Reset() {
		gameOver = false;
		ponder = false;
		post = true;
		multiPv = 1;
		movestogo = 0;
		depth = 0;
		nodes = 0;
		movetime = 0;
		flags = 0;
		rootMoves = {};
	}

};

extern schronos chronos;

void InitEval();
Value Eval();
Value Eval(Move m);
Value ShowEval();

extern string engineName;

std::string trim(const std::string& s);
void SplitString(const std::string& txt, std::vector<std::string>& vStr, char ch);
void SplitInt(const std::string& txt, std::vector<int>& vInt, char ch);
std::string thousandSeparator(uint64_t n);
std::string StrToLower(std::string s);

void PrintSummary(uint64_t time, uint64_t nodes);

bool GetInput(std::string& s);
int InitImput();

//transposition.cpp
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

//uci.cpp
struct SOptions {
	bool ponder = true;
	int hash = 32;
	int elo = 2500;
	int eloMin = 0;
	int eloMax = 2500;
	int multiPV = 1;

	int aspiration = 36;
	int rfp = 120;
	int futility = 748;
	int lmr = 183;
	int nullMove = 941;
	int razoring = 530;
	string bishop = "32 55 -36 -3";
	string defense = "11 14 11 19 -6 18 -3 12 -62 12 -50 21";
	string king = "52 39";
	string material = "-27 13 22 -38 32";
	string mobility = "8 5 2 7 3 5 3 2";
	string outFile = "2 -5 -3 -5 -26 -4 -6 -8 -4 1 12 -15";
	string outpost = "81 6 11 4";
	string outRank = "1 57 -17 5 -17 1 3 4 -10 11 16 -22";
	string passed = "-5 8 -49 -4 5";
	string pawn = "3 7 -28 -25 -8 -21 -10 3";
	string rook = "76 9 30 11";
	string tempo = "20 8";


};
extern SOptions options;

void UciCommand(string str);
void UciLoop();