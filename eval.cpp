#include "eval.h"

int32_t tabSouMg[6 * 64]
{ //PAWN MG
		  100,  100,  100,  100,  100,  100,  100,  100,
		  176,  214,  147,  194,  189,  214,  132,   77,
		   82,   88,  106,  113,  150,  146,  110,   73,
		   67,   93,   83,   95,   97,   92,   99,   63,
		   55,   74,   80,   89,   94,   86,   90,   55,
		   55,   70,   68,   69,   76,   81,  101,   66,
		   52,   84,   66,   60,   69,   99,  117,   60,
		  100,  100,  100,  100,  100,  100,  100,  100,
		  //KNIGHT MG
		  116,  228,  271,  270,  338,  213,  278,  191,
		  225,  247,  353,  331,  321,  360,  300,  281,
		  258,  354,  343,  362,  389,  428,  375,  347,
		  300,  332,  325,  360,  349,  379,  339,  333,
		  298,  322,  325,  321,  337,  332,  332,  303,
		  287,  297,  316,  319,  327,  320,  327,  294,
		  276,  259,  300,  304,  308,  322,  296,  292,
		  208,  290,  257,  274,  296,  284,  293,  284,
		  //BISHOP MG
		  292,  338,  254,  283,  299,  294,  337,  323,
		  316,  342,  319,  319,  360,  385,  343,  295,
		  342,  377,  373,  374,  368,  392,  385,  363,
		  332,  338,  356,  384,  370,  380,  337,  341,
		  327,  354,  353,  366,  373,  346,  345,  341,
		  335,  350,  351,  347,  352,  361,  350,  344,
		  333,  354,  354,  339,  344,  353,  367,  333,
		  309,  341,  342,  325,  334,  332,  302,  313,
		  //ROOK MG
		  493,  511,  487,  515,  514,  483,  485,  495,
		  493,  498,  529,  534,  546,  544,  483,  508,
		  465,  490,  499,  497,  483,  519,  531,  480,
		  448,  464,  476,  495,  484,  506,  467,  455,
		  442,  451,  468,  470,  476,  472,  498,  454,
		  441,  461,  468,  465,  478,  481,  478,  452,
		  443,  472,  467,  476,  483,  500,  487,  423,
		  459,  463,  470,  479,  480,  480,  446,  458,
		  //QUEEN MG
		  865,  902,  922,  911,  964,  948,  933,  928,
		  886,  865,  903,  921,  888,  951,  923,  940,
		  902,  901,  907,  919,  936,  978,  965,  966,
		  881,  885,  897,  894,  898,  929,  906,  915,
		  907,  884,  899,  896,  904,  906,  912,  911,
		  895,  916,  900,  902,  904,  912,  924,  917,
		  874,  899,  918,  908,  915,  924,  911,  906,
		  906,  899,  906,  918,  898,  890,  878,  858,
		  //KING MG
		  -11,   70,   55,   31,  -37,  -16,   22,   22,
		   37,   24,   25,   36,   16,    8,  -12,  -31,
		   33,   26,   42,   11,   11,   40,   35,   -2,
			0,   -9,    1,  -21,  -20,  -22,  -15,  -60,
		  -25,   16,  -27,  -67,  -81,  -58,  -40,  -62,
			7,   -2,  -37,  -77,  -79,  -60,  -23,  -26,
		   12,   15,  -13,  -72,  -56,  -28,   15,   17,
		   -6,   44,   29,  -58,    8,  -25,   34,   28,
};

int32_t tabSouEg[6 * 64]
{ //PAWN EG
  100,  100,  100,  100,  100,  100,  100,  100,
  277,  270,  252,  229,  240,  233,  264,  285,
  190,  197,  182,  168,  155,  150,  180,  181,
  128,  117,  108,  102,   93,  100,  110,  110,
  107,  101,   89,   85,   86,   83,   92,   91,
   96,   96,   85,   92,   88,   83,   85,   82,
  107,   99,   97,   97,  100,   89,   89,   84,
  100,  100,  100,  100,  100,  100,  100,  100,
  //KNIGHT EG
  229,  236,  269,  250,  257,  249,  219,  188,
  252,  274,  263,  281,  273,  258,  260,  229,
  253,  264,  290,  289,  278,  275,  263,  243,
  267,  280,  299,  301,  299,  293,  285,  264,
  263,  273,  293,  301,  296,  293,  284,  261,
  258,  276,  278,  290,  287,  274,  260,  255,
  241,  259,  270,  277,  276,  262,  260,  237,
  253,  233,  258,  264,  261,  260,  234,  215,
  //BISHOP EG
  288,  278,  287,  292,  293,  290,  287,  277,
  289,  294,  301,  288,  296,  289,  294,  281,
  292,  289,  296,  292,  296,  300,  296,  293,
  293,  302,  305,  305,  306,  302,  296,  297,
  289,  293,  304,  308,  298,  301,  291,  288,
  285,  294,  304,  303,  306,  294,  290,  280,
  285,  284,  291,  299,  300,  290,  284,  271,
  277,  292,  286,  295,  294,  288,  290,  285,
  //ROOK EG
  506,  500,  508,  502,  504,  507,  505,  503,
  505,  506,  502,  502,  491,  497,  506,  501,
  504,  503,  499,  500,  500,  495,  496,  496,
  503,  502,  510,  500,  502,  504,  500,  505,
  505,  509,  509,  506,  504,  503,  496,  495,
  500,  503,  500,  505,  498,  498,  499,  489,
  496,  495,  502,  505,  498,  498,  491,  499,
  492,  497,  498,  496,  493,  493,  497,  480,
  //QUEEN EG
  918,  937,  943,  945,  934,  926,  924,  942,
  907,  945,  946,  951,  982,  933,  928,  912,
  896,  921,  926,  967,  963,  937,  924,  915,
  926,  944,  939,  962,  983,  957,  981,  950,
  893,  949,  942,  970,  952,  956,  953,  936,
  911,  892,  933,  928,  934,  942,  934,  924,
  907,  898,  883,  903,  903,  893,  886,  888,
  886,  887,  890,  872,  916,  890,  906,  879,
  //KING EG
  -74,  -43,  -23,  -25,  -11,   10,    1,  -12,
  -18,    6,    4,    9,    7,   26,   14,    8,
   -3,    6,   10,    6,    8,   24,   27,    3,
  -16,    8,   13,   20,   14,   19,   10,   -3,
  -25,  -14,   13,   20,   24,   15,    1,  -15,
  -27,  -10,    9,   20,   23,   14,    2,  -12,
  -32,  -17,    4,   14,   15,    5,  -10,  -22,
  -55,  -40,  -23,   -6,  -20,   -8,  -28,  -47,
};


int32_t tabPositionMg[2][6][64];
int32_t tabPositionEg[2][6][64];
int32_t tabPosition[29][2][6][64];

int EloRnd(int range) {
	return rand() % (range + 1) - (range >> 1);
}

void EvalInit() {
	srand(time(NULL));
	int elo = options.elo;
	if (elo < options.eloMin)
		elo = options.eloMin;
	if (elo > options.eloMax)
		elo = options.eloMax;
	elo -= options.eloMin;
	int eloRange = options.eloMax - options.eloMin;
	int range = 800 - (elo * 800) / eloRange;
	for (int r = 0; r < 6; r++)
		for (int y = 0; y < 8; y++)
			for (int x = 0; x < 8; x++) {
				int iw = (7 - y) * 8 + x;
				int ib = y * 8 + x;
				int shift = r * 64 + ib;
				tabPositionMg[0][r][iw] = tabSouMg[shift] + EloRnd(range);
				tabPositionEg[0][r][iw] = tabSouEg[shift];
				tabPositionMg[1][r][ib] = tabSouMg[shift] + EloRnd(range);
				tabPositionEg[1][r][ib] = tabSouEg[shift];
			}
	for(int p =0;p<29;p++)
		for(int c=0;c<2;c++)
			for(int r =0;r<6;r++)
				for (int s = 0; s < 64; s++) {
					int32_t mg = tabPositionMg[c][r][s];
					int32_t eg = tabPositionEg[c][r][s];
					tabPosition[p][c][r][s] = (mg * p + eg * (28 - p)) / 28;
				}
}

/*int32_t Eval(Move m) {
	int fr = m.from();
	int to = m.to();
	int flags = m.flags();
	Piece pfr = position.board[fr];
	int frCol = color_of(pfr);
	int frRank = type_of(pfr);
	int frMg = tabPositionMg[frCol][frRank][fr];
	int frEg = tabPositionEg[frCol][frRank][fr];
	int32_t scoreMg = -frMg;
	int32_t scoreEg = -frEg;
	if (flags & MoveFlags::CAPTURE) {
		Piece pto = position.board[to];
		int toRank = type_of(pto);
		scoreMg += tabPositionMg[frCol ^ 1][toRank][to];
		scoreEg += tabPositionEg[frCol ^ 1][toRank][to];
	}
	if ((flags & MoveFlags::PROMOTION_CAPTURES) == MoveFlags::PROMOTION_CAPTURES)
		frRank = 1 + flags & 3;
	scoreMg += tabPositionMg[frCol][frRank][to];
	scoreEg += tabPositionEg[frCol][frRank][to];
	int phase = position.Phase() - 2;
	return (scoreMg * phase + scoreEg * (30 - phase)) / 30;
}*/

S32 Eval(Move m,bool q) {
	Square fr = m.from();
	Square to = m.to();
	int flags = m.flags();
	Piece pfr = position.board[fr];
	int frCol = color_of(pfr);
	int frRank = type_of(pfr);
	int frSc = tabPosition[position.phase][frCol][frRank][fr];
	S32 score = -frSc;
	if (flags & MoveFlags::CAPTURE) {
		Piece pto = position.board[to];
		int toRank = type_of(pto);
		score += tabPosition[position.phase][frCol ^ 1][toRank][to];// -tabPosition[position.phase][frCol][frRank][to];
		if(q && position.Attackers(to))
			score -= tabPosition[position.phase][frCol][frRank][to];
	}
	if (flags & MoveFlags::PROMOTION)
		frRank = 1 + flags & 3;
	return score + tabPosition[position.phase][frCol][frRank][to];
}

/*int32_t Eval() {
	int32_t score = 0;
	/*Bitboard bb = position.piece_bb[WHITE_PAWN];
	if (bb > 0)
		score += sparse_pop_count(bb) * 100;
	bb = position.piece_bb[BLACK_PAWN];
	if (bb > 0)
		score -= sparse_pop_count(bb) * 100;
	bb = position.piece_bb[WHITE_KNIGHT];
	if (bb > 0)
		score += sparse_pop_count(bb) * 300;
	bb = position.piece_bb[BLACK_KNIGHT];
	if (bb > 0)
		score -= sparse_pop_count(bb) * 300;
	bb = position.piece_bb[WHITE_BISHOP];
	if (bb > 0)
		score += sparse_pop_count(bb) * 300;
	bb = position.piece_bb[BLACK_BISHOP];
	if (bb > 0)
		score -= sparse_pop_count(bb) * 300;
	bb = position.piece_bb[WHITE_ROOK];
	if (bb > 0)
		score += sparse_pop_count(bb) * 500;
	bb = position.piece_bb[BLACK_ROOK];
	if (bb > 0)
		score -= sparse_pop_count(bb) * 500;
	bb = position.piece_bb[WHITE_QUEEN];
	if (bb > 0)
		score += sparse_pop_count(bb) * 800;
	bb = position.piece_bb[BLACK_QUEEN];
	if (bb > 0)
		score -= sparse_pop_count(bb) * 800;
	return position.ColorUs() == WHITE ? score : -score;*/
		/*MoveList<WHITE> listw(position);
		score += listw.size();
		MoveList<BLACK> listb(position);
		score -= listb.size();
		int count = listw.size() + listb.size();*/

/*	int col, rank;
	int32_t scoreWMg = 0;
	int32_t scoreWEg = 0;
	int32_t scoreBMg = 0;
	int32_t scoreBEg = 0;
	int pieceW[6]={};
	int pieceB[6]={};
	for (int n = 0; n < 64; n++)
	{
		Piece p = position.board[n];
		if (p == NO_PIECE)
			continue;
		col = color_of(p);
		rank = type_of(p);
		int32_t valMg = tabPositionMg[col][rank][n];
		int32_t valEg = tabPositionEg[col][rank][n];
		if (col == 0) {
			scoreWMg += valMg;
			scoreWEg += valEg;
			pieceW[rank]++;
		}
		else {
			scoreBMg += valMg;
			scoreBEg += valEg;
			pieceB[rank]++;
		}
	}
	bool insufficientW = !pieceW[0] && !pieceW[3] && !pieceW[4];
	bool insufficientB = !pieceB[0] && !pieceB[3] && !pieceB[4];
	if (insufficientW)
		insufficientW = pieceW[1] + pieceW[2] * 2 < 3;
	if (insufficientB)
		insufficientB = pieceB[1] + pieceB[2] * 2 < 3;
	if (insufficientW && insufficientB)
		return 0;
	if (insufficientW)
		scoreBEg <<= 1;
	if (insufficientB)
		scoreWEg <<= 1;
	int32_t scoreMg = scoreWMg - scoreBMg;
	int32_t scoreEg = scoreWEg - scoreBEg;
	score = (scoreMg * position.phase + scoreEg * (28 - position.phase)) / 28;
	return position.ColorUs() == WHITE ? score : -score;
}
*/

S32 Eval() {
	int col, rank;
	S32 scoreW = 0;
	S32 scoreB = 0;
	int pieceW[6] = {};
	int pieceB[6] = {};
	//Bitboard bb = position.AllPieces();
	//while (bb) {
		//Square s = pop_lsb(&bb);
	//}
	for (int s = 0; s < 64; s++)
	{
		Piece p = position.board[s];
		if (p == NO_PIECE)
			continue;
		col = color_of(p);
		rank = type_of(p);
		int32_t val = tabPosition[position.phase][col][rank][s];
		if (col == 0) {
			scoreW += val;
			pieceW[rank]++;
		}
		else {
			scoreB += val;
			pieceB[rank]++;
		}
	}
	bool insufficientW = !pieceW[0] && !pieceW[3] && !pieceW[4];
	bool insufficientB = !pieceB[0] && !pieceB[3] && !pieceB[4];
	if (insufficientW)
		insufficientW = pieceW[1] + pieceW[2] * 2 < 3;
	if (insufficientB)
		insufficientB = pieceB[1] + pieceB[2] * 2 < 3;
	if (insufficientW && insufficientB)
		return 0;
	if (insufficientW)
		scoreB <<= 1;
	if (insufficientB)
		scoreW <<= 1;
	return position.ColorUs() == WHITE ? scoreW - scoreB : scoreB - scoreW;
}