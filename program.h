#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <algorithm>
#include <vector> 

#include "position.h"
#include "tables.h"
#include "types.h"

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
	int futility = 748;
	int lmr = 183;
	int nullMove = 941;
	int razoring = 529;
	string bishop = "32 55 -36 -4";
	string defense = "11 14 11 20 -6 18 -3 13 -62 13 -46 20";
	string king = "52 39";
	string material = "-27 13 22 -34 33";
	string mobility = "8 5 3 7 3 5 3 2";
	string outFile = "2 -6 -3 -5 -58 -4 -6 -8 -4 -1 12 -15";
	string outpost = "80 8 11 4";
	string outRank = "1 57 -17 5 -17 2 3 5 -10 11 16 -22";
	string passed = "-5 8 -49 -4 4";
	string pawn = "3 7 -28 -26 -8 -21 -10 3";
	string rook = "72 1 30 12";
	string tempo = "16 8";


};
extern SOptions options;

void UciCommand(string str);
void UciLoop();

//search.cpp
struct sSearchDriver {
	int moveSet = 0;
	int moveOk = 0;
	int multiPV = 1;
	Depth depth = DEPTH_ZERO;
	Value bestScore = VALUE_ZERO;
	U64 nodes = 0;
	U64 nodesq = 0;
	Color colorUs = WHITE;
	Move bestMove = MOVE_NONE;
	Move ponderMove = MOVE_NONE;
	std::chrono::steady_clock::time_point timeStart{};

	void Restart() {
		moveSet = 0;
		moveOk = 0;
		depth = DEPTH_ZERO;
		bestScore = VALUE_ZERO;
		nodes = 0;
		nodesq = 0;
		bestMove = 0;
		ponderMove = 0;
		timeStart = chrono::steady_clock::now();
		colorUs = g_pos.ColorUs();
	}

	//Time in miliseconds
	U64 Ms() const {
		chrono::steady_clock::time_point timeNow = chrono::steady_clock::now();
		chrono::milliseconds duration = chrono::duration_cast<chrono::milliseconds>(timeNow - timeStart);
		return duration.count();
	}
};
extern sSearchDriver sd;

void InitSearch();
void SearchIterate();