#pragma once

#include <iostream>
#include <chrono>

#include "input.h"
#include "move.h"
#include "eval.h"
#include "chronos.h"
#include "picker.h"
#include "position.h"
#include "program.h"
#include "types.h"
#include "uci.h"

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
