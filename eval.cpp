#include "eval.h"
#include "types.h"
#include <algorithm>

using namespace std;

enum Tracing { NO_TRACE, TRACE };


Score mobility[PT_NB] = {};
Value kingShield1 = VALUE_ZERO;
Value kingShield2 = VALUE_ZERO;
const Value materialValOrg[PT_NB] = { Value(100), Value(320), Value(330), Value(500), Value(900), VALUE_ZERO };
Score material[PT_NB] = {};
Value materialMax[PT_NB] = {};
Score outpost[2][2] = {};
Value passedFile = VALUE_ZERO;
Value passedRank = VALUE_ZERO;
Value passedBlocked = VALUE_ZERO;
Value passedKU = VALUE_ZERO;
Value passedKE = VALUE_ZERO;
Score pawnConnected = SCORE_ZERO;
Score pawnDoubled = SCORE_ZERO;
Score pawnIsolated = SCORE_ZERO;
Score pawnBackward = SCORE_ZERO;
Score rookOpen = SCORE_ZERO;
Score rookSemiOpen = SCORE_ZERO;
Score scorePair[PT_NB] = {};
Score scoreTropism[PT_NB] = {};

const Score outsideFileOrg[PT_NB] = {D(-1,-8), D(-6, -2), D(-3,-6), D(-4, -2), D(-4, -1), D(-1, 7)};
Score outsideFile[PT_NB] = {};
const Score outsideRankOrg[PT_NB] = { D(0,8), D(-16, -1), D(-16, -6), D(5, 4), D(-10,10), D(2, 16) };
Score outsideRank[PT_NB] = {};

const int chance[PT_NB] = { 3, 1, 2, 3, 3, 0 };
Score bonus[PT_NB][RANK_NB][FILE_NB] = {};
Value bonusMax[PT_NB][RANK_NB][FILE_NB] = {};

Score contempt = SCORE_ZERO;
Value tempo = VALUE_ZERO;

int phase = 0;
int aPhase[PT_NB] = { 0,1,1,2,4,0 };

inline static Piece GetCapturedPiece(Move m) {
	return g_pos.Board(m.To());
}

inline static Piece GetMovingPiece(Move m) {
	return g_pos.Board(m.From());
}

static int GetVal(vector<int> v, int i) {
	if (i >= 0 && i < v.size())
		return v[i];
	return 0;
}

static inline int OutsideFile(File file) {
	return abs(file * 2 - 7) / 2 - 2;
}

static inline int OutsideRank(Rank rank) {
	return abs(rank * 2 - 7) / 2 - 2;
}

static inline int Centrality(Rank rank, File file) {
	return 3 - abs(rank * 2 - 7) / 2 - abs(file * 2 - 7) / 2;
}

static int Centrality(Square sq) {
	return Centrality(RankOf(sq), FileOf(sq));
}

static int Distance(Square s1, Square s2) {
	return max(abs(FileOf(s1) - FileOf(s2)), abs(RankOf(s1) - RankOf(s2))) - 4;
};

static int Tropism(Square sq1, Square sq2)
{
	return 7 - (abs(RankOf(sq1) - RankOf(sq2)) + abs(FileOf(sq1) - FileOf(sq2)));
}

static Value ValueMax(Score score) {
	return max(Mg(score), Eg(score));
}

void InitEval() {
	int mg, eg;
	int v, d;
	srand(time(NULL));
	int elo = options.elo;
	if (elo < options.eloMin)
		elo = options.eloMin;
	if (elo > options.eloMax)
		elo = options.eloMax;
	elo -= options.eloMin;
	int eloRange = options.eloMax - options.eloMin;
	int eloMod = 600 - (600 * elo) / eloRange;
	vector<int> split{};

	SplitInt(options.mobility, split, ' ');
	for (PieceType pt = KNIGHT; pt < KING; ++pt) {
		mg = GetVal(split, (pt-1) * 2);
		eg = GetVal(split, (pt-1) * 2 + 1);
		mobility[pt] = S(mg, eg);
	}

	SplitInt(options.outFile, split, ' ');
	for (PieceType pt = PAWN; pt < PT_NB; ++pt) {
		mg = GetVal(split, pt * 2);
		eg = GetVal(split, pt * 2 + 1);
		outsideFile[pt] = S(mg, eg);
	}
	SplitInt(options.outRank, split, ' ');
	for (PieceType pt = PAWN; pt < PT_NB; ++pt) {
		mg = GetVal(split, pt * 2);
		eg = GetVal(split, pt * 2 + 1);
		outsideRank[pt] = S(mg, eg);
	}

	SplitInt(options.king, split, ' ');
	kingShield1 = (Value)GetVal(split, 0);
	kingShield2 = (Value)GetVal(split, 1);

	SplitInt(options.material, split, ' ');
	for (PieceType pt = PAWN; pt < KING; ++pt) {
		int md = GetVal(split, pt);
		mg = materialValOrg[pt] + md - eloMod;
		eg = materialValOrg[pt] - md;
		material[pt] = S(mg, eg);
		materialMax[pt] = ValueMax(material[pt]);
	}

	SplitInt(options.outpost, split, ' ');
	for (int s = 0; s < 2; s++) {
		mg = GetVal(split, s * 2);
		eg = GetVal(split, s * 2 + 1);
		Score score = S(mg, eg);
		outpost[0][s] = score * 2;
		outpost[1][s] = score;
	}

	SplitInt(options.tropism, split, ' ');
	for (PieceType pt = KNIGHT; pt < KING; ++pt) {
		v = GetVal(split, (pt - 1) * 2);
		d = GetVal(split, (pt - 1) * 2 + 1);
		scoreTropism[pt] = D(v, d);
	}

	SplitInt(options.pair, split, ' ');
	for (int n = 0; n < 3; n++) {
		v = GetVal(split, n * 2);
		d = GetVal(split, n * 2 + 1);
		scorePair[n + 1] = D(v, d);
	}

	SplitInt(options.passed, split, ' ');
	passedFile = (Value)GetVal(split, 0);
	passedRank = (Value)GetVal(split, 1);
	passedBlocked = (Value)GetVal(split, 2);
	passedKU = (Value)GetVal(split, 3);
	passedKE = (Value)GetVal(split, 4);

	SplitInt(options.pawn, split, ' ');
	mg = GetVal(split, 0);
	eg = GetVal(split, 1);
	pawnConnected = S(mg, eg);
	mg = GetVal(split, 2);
	eg = GetVal(split, 3);
	pawnDoubled = S(mg, eg);
	mg = GetVal(split, 4);
	eg = GetVal(split, 5);
	pawnIsolated = S(mg, eg);
	mg = GetVal(split, 6);
	eg = GetVal(split, 7);
	pawnBackward = S(mg, eg);

	SplitInt(options.rook, split, ' ');
	mg = GetVal(split, 0);
	eg = GetVal(split, 1);
	rookOpen = S(mg, eg);
	mg = GetVal(split, 2);
	eg = GetVal(split, 3);
	rookSemiOpen = S(mg, eg);

	for (PieceType pt = PAWN; pt < PT_NB; ++pt)
		for (Rank r = RANK_1; r < RANK_NB; ++r)
			for (File f = FILE_A; f < FILE_NB; ++f)
			{
				bonus[pt][r][f] = material[pt];
				bonus[pt][r][f] += outsideFile[pt] * OutsideFile(f);
				if (pt == PAWN) {
					bonus[pt][r][f] += outsideRank[pt] * (r - 4);
				}
				else
				{
					bonus[pt][r][f] += outsideRank[pt] * OutsideRank(r);
				}
				bonusMax[pt][r][f] = ValueMax(bonus[pt][r][f]);
			}
}

static Bitboard GetLeastValuablePiece(Bitboard attadef, Color bySide, Piece& piece) {
	int maskColor = bySide << 3;
	for (int n = PAWN; n <= KING; n++) {
		piece = Piece(maskColor | n);
		const Bitboard subset = attadef & g_pos.bitboard_of(piece);
		if (subset)
			return subset & -subset;    // single bit
	}
	return 0;    // empty set
}

static Value See(Move m) {
	//if (!m.IsCapture())return VALUE_ZERO;
	Square sqFrom = m.From();
	Square sqTo = m.To();
	MoveFlags flags = m.Flags();
	Piece  capturedPiece = GetCapturedPiece(m);
	Piece  capturingPiece = GetMovingPiece(m);
	Color attacker = ColorOf(capturingPiece);
	Value gain[32]{};
	int d = 0;
	Bitboard fromSet = SQUARE_BB[sqFrom];
	Bitboard occ = g_pos.AllPieces();
	Bitboard sqBB = SQUARE_BB[sqTo];
	Bitboard bishopsQueens;
	Bitboard rooksQueens;
	rooksQueens = bishopsQueens = g_pos.bitboard_of(WHITE_QUEEN) | g_pos.bitboard_of(BLACK_QUEEN);
	rooksQueens |= g_pos.bitboard_of(WHITE_ROOK) | g_pos.bitboard_of(BLACK_ROOK);
	bishopsQueens |= g_pos.bitboard_of(WHITE_BISHOP) | g_pos.bitboard_of(BLACK_BISHOP);
	Bitboard fixed = ((Shift(NORTH_WEST, sqBB) | Shift(NORTH_EAST, sqBB)) & g_pos.bitboard_of(BLACK_PAWN))
		| ((Shift(SOUTH_WEST, sqBB) | Shift(SOUTH_EAST, sqBB)) & g_pos.bitboard_of(WHITE_PAWN))
		| (PSEUDO_LEGAL_ATTACKS[KNIGHT][sqTo] & (g_pos.bitboard_of(WHITE_KNIGHT) | g_pos.bitboard_of(BLACK_KNIGHT)))
		| (PSEUDO_LEGAL_ATTACKS[KING][sqTo] & (g_pos.bitboard_of(WHITE_KING) | g_pos.bitboard_of(BLACK_KING)));
	Bitboard attadef = (fixed | ((GetBishopAttacks(sqTo, occ) & bishopsQueens) | (GetRookAttacks(sqTo, occ) & rooksQueens)));
	if (m.IsCapture())
		gain[d] = materialMax[TypeOf(capturedPiece)];
	else
		gain[d] = VALUE_ZERO;
	do {
		d++;
		attacker = ~attacker;
		gain[d] = materialMax[TypeOf(capturingPiece)] - gain[d - 1];
		if (-gain[d - 1] < 0 && gain[d] < 0)
			break;    // pruning does not influence the result
		attadef ^= fromSet;    // reset bit in set to traverse
		occ ^= fromSet;
		attadef |=
			occ & ((GetBishopAttacks(sqTo, occ) & bishopsQueens) | (GetRookAttacks(sqTo, occ) & rooksQueens));
		fromSet = GetLeastValuablePiece(attadef, attacker, capturingPiece);
	} while (fromSet);
	while (--d) {
		gain[d - 1] = -(-gain[d - 1] > gain[d] ? -gain[d - 1] : gain[d]);
	}
	return gain[0];
}

static Value Eval(Color color, Square sq, PieceType pt) {
	Rank rank = RelativeRank(color, RankOf(sq));
	File file = FileOf(sq);
	return bonusMax[pt][rank][file];
}

Value Eval(Move m) {
	Square fr = m.From();
	Square to = m.To();
	MoveFlags flags = m.Flags();
	Piece piece = g_pos.Board(fr);
	PieceType pt = TypeOf(piece);
	Color color = ColorOf(piece);
	Value value = -Eval(color, fr, pt);
	value += See(m);
	if (flags & MoveFlags::PROMOTION)
		pt = (PieceType)(1 + flags & 3);
	return value + Eval(color, to, pt);
}

static void Eval(Position& pos, SEvalSide& esUs, SEvalSide& esEn) {
	int cw = 0;
	Color color = esUs.color;
	Bitboard bbAll = pos.AllPieces();
	Bitboard bbPawnsUs = pos.piece_bb[MakePiece(color,PAWN)];
	Bitboard bbPawnsEn = pos.piece_bb[MakePiece(~color,PAWN)];
	Direction north = RelativeDir(color, NORTH);
	Direction south = RelativeDir(color, SOUTH);
	const Bitboard bbProtected = PawnAttacks(color, bbPawnsUs);
	const Bitboard bbAtacked = PawnAttacks(~color, bbPawnsEn);
	Bitboard bbConnected = bbProtected | Shift(south, bbProtected);
	bbConnected |= Shift(south, bbConnected);
	const Bitboard bbSpan = Span(~color, bbAtacked);
	const Bitboard bbOutpostRanks = color ? Rank5BB | Rank4BB | Rank3BB : Rank4BB | Rank5BB | Rank6BB;
	for (PieceType pt = PAWN; pt < PT_NB; ++pt) {
		Bitboard copy = pos.piece_bb[MakePiece(color,pt)];
		while (copy) {
			esUs.piece[pt]++;
			cw += chance[pt];
			phase += aPhase[pt];
			const Square sq = pop_lsb(&copy);
			const Rank r = RankOf(sq);
			const Rank rank = RelativeRank(color, r);
			const File file = FileOf(sq);
			esUs.scorePiece[pt] += bonus[pt][rank][file];
			const Bitboard bbPiece = 1ULL << sq;
			if (pt == PAWN) {
				//passed pawn
				if (!(bbPassedPawnMask[color][sq] & bbPawnsEn)) {
					Value v = passedFile * OutsideFile(file);
					v += floor(pow(((int)rank - 1.0) / 5.0, 2.0) * (int)passedRank);
					if (Shift(RelativeDir(color, NORTH), bbPiece) & pos.AllPieces(~color))
						v += passedBlocked;
					v += passedKU * (rank - 1) * Distance(esUs.king, sq);
					v += passedKE * (rank - 1) * Distance(esEn.king, sq);
					esUs.scorePawnPassed += S(v >> 1, v);
				}
				if (bbPawnsUs & bbForwardFiles[color][sq])
					esUs.scorePawnDoubled += pawnDoubled;
				if (bbPiece & bbConnected)
					esUs.scorePawnConnected += pawnConnected;
				else {
					Bitboard bb = bbPawnsUs & bbAdjacentFiles[file];
					if (!bb)
						esUs.scorePawnIsolated += pawnIsolated;
					else if (bb & bbForwardRank[color][r])
						esUs.scorePawnBackward += pawnBackward;
				}
			}
			else if (pt == KING) {
				esUs.king = sq;
				Bitboard bbShield1 = Shift(north, bbPiece);
				bbShield1 |= Shift(EAST, bbShield1) | Shift(WEST, bbShield1);
				Bitboard bbShield2 = Shift(north, bbShield1);
				Value v1 = kingShield1 * SparsePopCount(bbShield1 & bbPawnsUs);
				Value v2 = kingShield2 * SparsePopCount(bbShield2 & bbPawnsUs);
				esUs.scorePiece[pt] += S(v1, 0);
				esUs.scorePiece[pt] += S(v2, 0);
			}
			else {
				int count = PopCount(attacks(pt, sq, bbAll) & ~bbAtacked);
				esUs.scoreMobility += mobility[pt] * count;
				esUs.scoreTropism += scoreTropism[pt] * Tropism(sq, esEn.king);
				if (pt == ROOK) {
					const Bitboard bbFile = 0x101010101010101ULL << file;
					if (!(bbFile & bbPawnsUs)) {
						if (!(bbFile & bbPawnsEn))
							esUs.scorePiece[pt] += rookOpen;
						else
							esUs.scorePiece[pt] += rookSemiOpen;
					}
				}
				else  if ((pt == KNIGHT) || (pt == BISHOP)) {
					Bitboard bbOutpost = (~bbSpan & bbOutpostRanks);
					if (bbOutpost & bbPiece)
						esUs.scorePiece[pt] += outpost[pt == BISHOP][bbProtected && bbPiece] * 2;
					else if (bbOutpost & attacks(pt, sq, pos.AllPieces()))
						esUs.scorePiece[pt] += outpost[pt == BISHOP][bbProtected && bbPiece];
				}
			}
		}
	}
	for (PieceType pt = KNIGHT; pt < QUEEN; ++pt)if (esUs.piece[pt] > 1)esUs.scorePair += scorePair[pt];
	esUs.chance = cw > 2;
	if (esUs.chance)esUs.score += S(100, 100);
}


static Score TotalScore(SEvalSide es) {
	Score result = es.score + es.scorePawnPassed + es.scorePawnConnected + es.scorePawnDoubled + es.scorePawnIsolated + es.scorePawnBackward + es.scoreMobility + es.scorePair + es.scoreTropism;
	for (Score s : es.scorePiece)
		result += s;
	return result;
}

template<typename T> void PrintE(T t) {
	cout << left << setw(8) << setfill(' ') << t;
}

static string ShowScore(Score s) {
	string result = std::format("({} {})", (int)Mg(s), (int)Eg(s));
	int len = 16 - result.length();
	if (len < 0)
		len = 0;
	result.append(len, ' ');
	return result;
}

static void ShowScore(string name, Score sw, Score sb) {
	int len = 16 - name.length();
	if (len < 0)
		len = 0;
	name.append(len, ' ');
	std::cout << name << ShowScore(sw) << " " << ShowScore(sb) << " " << ShowScore(sw - sb) << endl;
}

template<Tracing T>
Value Trace(Position& pos) {
	phase = 0;
	SEvalSide esW = {};
	SEvalSide esB = {};
	esW.color = WHITE;
	esB.color = BLACK;
	esW.king = bsf(pos.piece_bb[WHITE_KING]);
	esB.king = bsf(pos.piece_bb[BLACK_KING]);
	Eval(pos, esW, esB);
	Eval(pos, esB, esW);
	if (phase > 24)
		phase = 24;
	if (!esW.chance && !esB.chance)
		return VALUE_ZERO;
	Score score = TotalScore(esW) - TotalScore(esB) + contempt;
	//pos.phase = 4;
	int mgWeight = phase;
	int egWeight = 24 - mgWeight;
	int i = (mgWeight * (int)Mg(score) + egWeight * (int)Eg(score)) / 24;
	Value v = (Value)((i * (100 - pos.move50)) / 100);
	if ((!esW.chance && score > 0) || (!esB.chance && score < 0))
		return VALUE_ZERO;
	v = (pos.ColorWhite() ? v : -v) + tempo;
	if (T) {
		//pos.Phase();
		Picker picker;
		pos.MoveList(pos.ColorUs(), picker.mList, picker.count);
		picker.Fill();
		std::cout << "moves:" << endl;
		for (int n = 0; n < picker.count; n++) {
			PickerE pe = picker.Pick(n);
			cout << pe.move << right << setfill(' ') << setw(8) << pe.value << setw(8) << See(pe.move) << endl;
		}
		pos.PrintBoard();
		ShowScore("pawn connected", esW.scorePawnConnected, esB.scorePawnConnected);
		ShowScore("pawn isolated", esW.scorePawnIsolated, esB.scorePawnIsolated);
		ShowScore("pawn doubled", esW.scorePawnDoubled, esB.scorePawnDoubled);
		ShowScore("pawn backward", esW.scorePawnBackward, esB.scorePawnBackward);
		ShowScore("pawn passed", esW.scorePawnPassed, esB.scorePawnPassed);
		ShowScore("pawn", esW.scorePiece[0], esB.scorePiece[0]);
		ShowScore("knight", esW.scorePiece[1], esB.scorePiece[1]);
		ShowScore("bishop", esW.scorePiece[2], esB.scorePiece[2]);
		ShowScore("rook", esW.scorePiece[3], esB.scorePiece[3]);
		ShowScore("queen", esW.scorePiece[4], esB.scorePiece[4]);
		ShowScore("king", esW.scorePiece[5], esB.scorePiece[5]);
		ShowScore("mobility", esW.scoreMobility, esB.scoreMobility);
		ShowScore("pair", esW.scorePair, esB.scorePair);
		ShowScore("tropism", esW.scoreTropism, esB.scoreTropism);
		ShowScore("total", TotalScore(esW), TotalScore(esB));
		std::cout << "phase " << phase << endl;
		std::cout << "score " << v << endl;
	}
	return v;
}

Value ShowEval() {
	//position.SetFen("1b1rr1k1/3q1pp1/8/NP1pPb1p/1B1PP1n1/PQR1P1P1/2n1B1nP/5RK1 w - - 0 1");
		//position.SetFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		//position.SetFen("4r1k1/2pp1pp1/2b2q1p/rp2p3/4P3/P1PPQN1P/5PP1/R4RK1 w - - 0 20");
	//position.SetFen("3k4/5Q2/3Np1p1/1pPp4/6n1/P3PN2/5PPP/R3K2R b KQ - 0 25");
	g_pos.SetFen("7K/8/8/8/4Q3/2k5/8/8 b - - 0 20");
	return (Trace<TRACE>(g_pos));
}

Value Eval() {
	//if(sd.nodes<80)
	//cout << "node " << sd.nodes << endl;
	//if (sd.nodes > 57 && sd.nodes < 65) return (Trace<TRACE>(g_pos)); else
	return (Trace<NO_TRACE>(g_pos));
}