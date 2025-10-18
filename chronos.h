#pragma once

#include <vector>

#include "types.h"
#include "move.h"

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
	int multiPv =1;
	int time[2] = {};
	int inc[2] = {};
	int movestogo = 0;
	int depth = 0;
	int nodes = 0;
	int movetime = 0;
	int flags =0;
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
