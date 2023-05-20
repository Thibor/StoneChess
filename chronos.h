#pragma once

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
	bool gameOver;
	bool ponder;
	bool post;
	int time[2];
	int inc[2];
	int movestogo;
	int depth;
	int nodes;
	int movetime;
	int flags;

	void Reset() {
		gameOver = false;
		ponder = false;
		post = true;
		movestogo = 0;
		depth = 0;
		nodes = 0;
		movetime = 0;
		flags = 0;
	}
};
extern schronos chronos;
