#pragma once

#include <iostream>
#include <chrono>

#include "eval.h"
#include "chronos.h"
#include "picker.h"
#include "position.h"
#include "program.h"
#include "types.h"
#include "uci.h"

struct sSearchDriver {
	int moveSet=0;
	int moveOk=0;
	int32_t depth=0;
	int32_t bstScore=0;
	uint64_t nodes=0;
	uint64_t nodesq=0;
	Move bstMove=0;
	Move ponderMove=0;
	std::chrono::steady_clock::time_point timeStart{};

	void Restart() {
		moveSet = 0;
		moveOk = 0;
		depth = 0;
		bstScore =0;
		nodes=0;
		nodesq = 0;
		bstMove = 0;
		ponderMove = 0;
		timeStart = chrono::steady_clock::now();
	}

	//Time in miliseconds
	uint64_t Ms() {
		chrono::steady_clock::time_point timeNow = chrono::steady_clock::now();
		chrono::milliseconds duration = chrono::duration_cast<chrono::milliseconds>(timeNow - timeStart);
		return duration.count();
	}
};
extern sSearchDriver sd;

void SearchInit();
void SearchIterate();
