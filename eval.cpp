#include "eval.h"
#include "types.h"
#include <algorithm>

using namespace std;

enum Tracing { NO_TRACE, TRACE };

enum Term { PASSED = 6, STRUCTURE, TERM_NB };

Score scores[TERM_NB][2];


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
Score pawnProtection[PT_NB] = {};
Score rookOpen = SCORE_ZERO;
Score rookSemiOpen = SCORE_ZERO;
Score scoreBishopPair = SCORE_ZERO;
Score scoreBishopBad = SCORE_ZERO;

Score outsideFile[PT_NB] = {};
Score outsideRank[PT_NB] = {};

const int chance[PT_NB] = { 3, 1, 2, 3, 3, 0 };
Score bonus[PT_NB][RANK_NB][FILE_NB] = {};
Value bonusMax[PT_NB][RANK_NB][FILE_NB] = {};

//Score contempt = SCORE_ZERO;
Score tempo = SCORE_ZERO;

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

static inline int PassedRank(Rank rank) {
	return (rank - 1) * (rank - 1);
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

static Value ScoreToValue(Score score) {
	int mgWeight = phase;
	int egWeight = 24 - mgWeight;
	return (Value)(mgWeight * (int)Mg(score) + egWeight * (int)Eg(score)) / 24;
}

void InitEval() {
	tempo = S(options.tempo, options.tempo/2);
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
		mg = GetVal(split, (pt - 1) * 2);
		eg = GetVal(split, (pt - 1) * 2 + 1);
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
	SplitInt(options.pawnProtection, split, ' ');
	for (int pt = PAWN; pt < PT_NB; pt++) {
		mg = GetVal(split, pt * 2);
		eg = GetVal(split, pt * 2 + 1);
		pawnProtection[pt] = S(mg, eg);
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

	SplitInt(options.bishop, split, ' ');
	mg = GetVal(split, 0);
	eg = GetVal(split, 1);
	scoreBishopPair = S(mg, eg);
	mg = GetVal(split, 2);
	eg = GetVal(split, 3);
	scoreBishopBad = S(mg, eg);

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

static Score TotalScore(int c) {
	Score score = SCORE_ZERO;
	for (int n = 0; n < TERM_NB; n++)
		score += scores[n][c];
	return score;
}

//template<Tracing T>
static Score Eval(Position& pos, SEvalSide& esUs, SEvalSide& esEn) {
	int cw = 0;
	Color color = esUs.color;
	Bitboard bbAll = pos.AllPieces();
	Bitboard bbPawnsUs = pos.piece_bb[MakePiece(color, PAWN)];
	Bitboard bbPawnsEn = pos.piece_bb[MakePiece(~color, PAWN)];
	Direction north = RelativeDir(color, NORTH);
	Direction south = RelativeDir(color, SOUTH);
	const Bitboard bbDefense = PawnAttacks(color, bbPawnsUs);
	const Bitboard bbAttack = PawnAttacks(~color, bbPawnsEn);
	Bitboard bbConnected = bbDefense | Shift(south, bbDefense);
	bbConnected |= Shift(south, bbConnected);
	const Bitboard bbSpan = Span(~color, bbAttack);
	const Bitboard bbOutpostRanks = color ? Rank5BB | Rank4BB | Rank3BB : Rank4BB | Rank5BB | Rank6BB;
	Bitboard bbOutpost = (~bbSpan) & bbOutpostRanks;
	//PrintBitboard(bbOutpost);
	for (PieceType pt = PAWN; pt < PT_NB; ++pt) {
		Piece piece = MakePiece(color, pt);
		Bitboard copy = pos.piece_bb[piece];
		while (copy) {
			esUs.piece[pt]++;
			cw += chance[pt];
			phase += aPhase[pt];
			const Square sq = pop_lsb(&copy);
			const Rank r = RankOf(sq);
			const Rank rank = RelativeRank(color, r);
			const File file = FileOf(sq);
			scores[pt][color] += bonus[pt][rank][file];
			const Bitboard bbPiece = 1ULL << sq;
			if (bbDefense & bbPiece)
				scores[pt][color] += pawnProtection[pt];
			if (pt == PAWN) {
				//passed pawns
				if (!(bbPassedPawnMask[color][sq] & bbPawnsEn)) {
					Value passed = passedFile * OutsideFile(file);
					passed += passedRank * PassedRank(rank);
					if (Shift(RelativeDir(color, NORTH), bbPiece) & pos.AllPieces(~color))
						passed += passedBlocked;
					Square sq2 = Square(sq + (color == WHITE ? 8 : -8));
					passed += passedKU * Distance(esUs.king, sq2);
					passed += passedKE * Distance(esEn.king, sq2);
					scores[PASSED][color] += S(passed >> 1, passed);
				}
				//structure pawns
				Score structure = SCORE_ZERO;
				if (bbPawnsUs & bbForwardFiles[color][sq])
					structure += pawnDoubled;
				if (bbPiece & bbConnected) {
					structure += pawnConnected;
				}
				else {
					Bitboard bb = bbPawnsUs & bbAdjacentFiles[file];
					if (!bb)
						structure += pawnIsolated;
					else {
						bb = Shift(north, bbDefense) & bbAdjacentFiles[file];
						if (bb & bbPawnsUs && bb & bbPawnsEn)
							structure += pawnBackward;
					}
				}
				scores[STRUCTURE][color] += structure;
			}
			else if (pt == KING) {
				if (file < 3 || file>4) {
					Bitboard bbShield1 = Shift(north, bbPiece);
					bbShield1 |= Shift(EAST, bbShield1) | Shift(WEST, bbShield1);
					Bitboard bbShield2 = Shift(north, bbShield1);
					Value v1 = kingShield1 * SparsePopCount(bbShield1 & bbPawnsUs);
					Value v2 = kingShield2 * SparsePopCount(bbShield2 & bbPawnsUs);
					scores[pt][color] += S(v1 + v2, 0);
				}
			}
			else {
				scores[pt][color] += mobility[pt] * PopCount(attacks(pt, sq, bbAll) & ~bbAttack);
				if (pt == ROOK) {
					const Bitboard bbFile = 0x101010101010101ULL << file;
					if (!(bbFile & bbPawnsUs)) {
						if (!(bbFile & bbPawnsEn))
							scores[pt][color] += rookOpen;
						else
							scores[pt][color] += rookSemiOpen;
					}
				}
				else  if ((pt == KNIGHT) || (pt == BISHOP)) {
					if (bbOutpost & bbPiece)
						scores[pt][color] += outpost[pt == BISHOP][bbDefense && bbPiece] * 2;
					else if (bbOutpost & attacks(pt, sq, pos.AllPieces()))
						scores[pt][color] += outpost[pt == BISHOP][bbDefense && bbPiece];
				}
			}
		}
	}
	if (esUs.piece[BISHOP]) {
		Piece piece = MakePiece(color, BISHOP);
		Bitboard bbPieces = pos.piece_bb[piece];
		bool bw = bbPieces & bbLight;
		bool bb = bbPieces & bbDark;
		if (bw && bb)
			scores[BISHOP][color] += scoreBishopPair;
		else {
			if (bw)
				scores[BISHOP][color] += scoreBishopBad * SparsePopCount(bbPawnsUs & bbLight);
			else
				scores[BISHOP][color] += scoreBishopBad * SparsePopCount(bbPawnsUs & bbDark);
		}
	}
	esUs.chance = cw > 2;
	Score score = esUs.chance ? S(100, 100) : SCORE_ZERO;
	return score + TotalScore(color);
}

//template<typename T> void PrintE(T t) {cout << left << setw(8) << setfill(' ') << t;}

static string ShowScore(string result) {
	int len = 16 - result.length();
	if (len < 0)
		len = 0;
	result.append(len, ' ');
	return result;
}

static string ShowScore(Score s) {
	Value v = ScoreToValue(s);
	return ShowScore(to_string(v) + " (" + to_string(Mg(s)) + " " + to_string(Eg(s)) + ")");
}

/*static void ShowScore(string name, Score sw, Score sb) {
	int len = 16 - name.length();
	if (len < 0)
		len = 0;
	name.append(len, ' ');
	std::cout << name << ShowScore(sw) << " " << ShowScore(sb) << " " << ShowScore(sw - sb);
	PrintE(ScoreToValue(sw - sb));
	std::cout << endl;
}*/

static void PrintTerm(string name, int idx) {
	Score sw = scores[idx][WHITE];
	Score sb = scores[idx][BLACK];
	std::cout << ShowScore(name) << ShowScore(sw) << " " << ShowScore(sb) << " " << ShowScore(sw - sb) << endl;
}

template<Tracing T>
Value Trace(Position& pos) {
	std::memset(scores, 0, sizeof(scores));
	phase = 0;
	SEvalSide esW = {};
	SEvalSide esB = {};
	esW.color = WHITE;
	esB.color = BLACK;
	esW.king = bsf(pos.piece_bb[WHITE_KING]);
	esB.king = bsf(pos.piece_bb[BLACK_KING]);
	Score sw = Eval(pos, esW, esB);
	Score sb = Eval(pos, esB, esW);
	if (phase > 24)
		phase = 24;
	if (!esW.chance && !esB.chance)
		return VALUE_ZERO;
	//Score score = tempo + sw - sb + contempt;
	Score score = sw - sb +(pos.ColorWhite() ? tempo : -tempo);
	Value v = ScoreToValue(score);
	if ((!esW.chance && score > 0) || (!esB.chance && score < 0))
		return VALUE_ZERO;
	v = (pos.ColorWhite() ? v : -v);
	if (T) {
		//pos.Phase();
		Picker picker;
		pos.MoveList(pos.ColorUs(), picker.mList, picker.count);
		picker.Fill();
		std::cout << "moves:" << endl;
		for (int n = 0; n < picker.count; n++) {
			PickerE pe = picker.Pick(n);
			cout << pe.move << " " << pe.value << " " << See(pe.move) << endl;
		}
		pos.PrintBoard();
		PrintTerm("Pawn", PAWN);
		PrintTerm("Knight", KNIGHT);
		PrintTerm("Bishop", BISHOP);
		PrintTerm("Rook", ROOK);
		PrintTerm("Queen", QUEEN);
		PrintTerm("King", KING);
		PrintTerm("Passed", PASSED);
		PrintTerm("Structure", STRUCTURE);
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
	g_pos.SetFen("1k6/1pp1R1p1/4P3/4b1P1/5p2/3q4/1P2R1PK/8 b - - 0 1");
	return (Trace<TRACE>(g_pos));
}

Value Eval() {
	//if(sd.nodes<80)
	//cout << "node " << sd.nodes << endl;
	//if (sd.nodes > 57 && sd.nodes < 65) return (Trace<TRACE>(g_pos)); else
	return (Trace<NO_TRACE>(g_pos));
}