#include "eval.h"
#include "types.h"
#include <algorithm>
using namespace std;

int phase;

constexpr static Value Ag(Score score) {
	return max(Mg(score), Eg(score));
}

Value Pg(Score score) {
	return (Mg(score) * phase + Eg(score) * (24 - phase)) / 24;
}

const Score oursideRankOrg[PT_NB] = { S(-14, 16), S(-15, -15), S(-22, -6), S(7, 0), S(1, -20), S(20, -27) };
Score outsideRank[PT_NB] = {};
const Score outsideFileOrg[PT_NB] = { S(2, -6), S(-3, -5), S(6, -4), S(-6, -2), S(-4, -1), S(2, -2) };
Score outsideFile[PT_NB] = {};
const Score mobilityOrg[PT_NB] = { S(-4,128), S(8, 5), S(7, 7), S(3, 5), S(3, 2), S(-5, -1) };
Score mobility[PT_NB] = {};

const Score scorePairOrg[PT_NB] = { SCORE_ZERO, S(10, 20), S(33, 56), S(10, 20),SCORE_ZERO,SCORE_ZERO };
Score scorePair[PT_NB] = {};
const Score rookOpenOrg = S(72, 1);
Score rookOpen = SCORE_ZERO;
const Score rookSemiOpenOrg = S(30, 12);
Score rookSemiOpen = SCORE_ZERO;
const Value materialVal[PT_NB] = { 100, 320, 330, 500, 900, SCORE_ZERO };
const Value materialDelOrg[PT_NB] = { -20, 30, 20, -10, 40,SCORE_ZERO };
Value materialDel[PT_NB] = {};
Value materialMax[PT_NB] = {};
Score material[PT_NB] = {};
const int phases[PT_NB] = { 0, 1, 1, 2, 4, 0 };
const int chance[PT_NB] = { 3, 1, 2, 3, 3, 0 };
Score Bonus[PT_NB][RANK_NB][FILE_NB] = {};

constexpr Value passedRankOrg = 128;
Value passedRank = VALUE_ZERO;
constexpr Value passedFileOrg = -4;
Value passedFile = VALUE_ZERO;
constexpr Value passedBlockedOrg = -46;
Value passedBlocked = VALUE_ZERO;
constexpr Value passedKUOrg = 3;
Value passedKU = VALUE_ZERO;
constexpr Value passedKEOrg = -1;
Value passedKE = VALUE_ZERO;

const Score kingShieldOrg = S(24, -11);
Score kingShield = SCORE_ZERO;

constexpr Score pawnConnectedOrg = S(4, 8);
Score pawnConnected = SCORE_ZERO;
constexpr Score pawnDoubledOrg = S(-21, -27);
Score pawnDoubled = SCORE_ZERO;
constexpr Score pawnIsolatedOrg = S(-5, -15);
Score pawnIsolated = SCORE_ZERO;

constexpr Score tempoOrg = S(20, 10);
Score tempo = SCORE_ZERO;

// Outpost[knight/bishop][supported by pawn] contains bonuses for minor
// pieces if they occupy or can reach an outpost square, bigger if that
// square is supported by a pawn.
constexpr Score OutpostOrg[2][2] = {
  { S(22, 6), S(36,12) }, // Knight
  { S(9, 2), S(15, 5) }  // Bishop
};

Score Outpost[2][2] = {};


inline static Piece GetCapturedPiece(Move m) {
	return position.Board(m.To());
}

inline static Piece GetMovingPiece(Move m) {
	return position.Board(m.From());
}

static int GetVal(vector<int> v, int i) {
	if (i >= 0 && i < v.size())
		return v[i];
	return 0;
}

static inline int OutsideFile(File file) {
	return abs(file * 2 - 7) / 2;
}

static inline int OutsideRank(Rank rank) {
	return abs(rank * 2 - 7) / 2;
}

static inline int Centrality(Rank rank, File file) {
	return 3 - max(abs(rank * 2 - 7) / 2, abs(file * 2 - 7) / 2);
}

static int Centrality(Square sq) {
	return Centrality(RankOf(sq), FileOf(sq));
}

void InitEval() {
	Value mg, eg;
	srand(time(NULL));
	int elo = options.elo;
	if (elo < options.eloMin)
		elo = options.eloMin;
	if (elo > options.eloMax)
		elo = options.eloMax;
	elo -= options.eloMin;
	int eloRange = options.eloMax - options.eloMin;
	int range = 800 - (elo * 800) / eloRange;
	vector<int> split{};
	splitInt(options.materialDel, split, ' ');
	for (PieceType pt = PAWN; pt < KING; ++pt) {
		Value md = materialDelOrg[pt] + GetVal(split, pt);
		mg = materialVal[pt] + md;
		mg = (-500 * (eloRange - elo) + mg * elo) / eloRange;
		eg = materialVal[pt] - md;
		material[pt] = S(mg, eg);
		materialMax[pt] = max(mg, eg);
	}
	splitInt(options.outsideRank, split, ' ');
	for (PieceType pt = PAWN; pt < PT_NB; ++pt) {
		mg = Mg(oursideRankOrg[pt]) + GetVal(split, pt * 2);
		eg = Eg(oursideRankOrg[pt]) + GetVal(split, pt * 2 + 1);
		outsideRank[pt] = S(mg, eg);
	}
	splitInt(options.outsideFile, split, ' ');
	for (PieceType pt = PAWN; pt < PT_NB; ++pt) {
		mg = Mg(outsideFileOrg[pt]) + GetVal(split, pt * 2);
		eg = Eg(outsideFileOrg[pt]) + GetVal(split, pt * 2 + 1);
		outsideFile[pt] = S(mg, eg);
	}
	splitInt(options.mobility, split, ' ');
	for (PieceType pt = PAWN; pt < PT_NB; ++pt) {
		mg = Mg(mobilityOrg[pt]) + GetVal(split, pt * 2);
		eg = Eg(mobilityOrg[pt]) + GetVal(split, pt * 2 + 1);
		mobility[pt] = S(mg, eg);
	}
	splitInt(options.passed, split, ' ');
	passedFile = passedFileOrg + GetVal(split, 0);
	passedRank = passedRankOrg + GetVal(split, 1);
	passedBlocked = passedBlockedOrg + GetVal(split, 2);
	passedKU = passedKUOrg + GetVal(split, 3);
	passedKE = passedKEOrg + GetVal(split, 4);


	splitInt(options.pawn, split, ' ');
	mg = Mg(pawnConnectedOrg) + GetVal(split, 0);
	eg = Eg(pawnConnectedOrg) + GetVal(split, 1);
	pawnConnected = S(mg, eg);
	mg = Mg(pawnDoubledOrg) + GetVal(split, 2);
	eg = Eg(pawnDoubledOrg) + GetVal(split, 3);
	pawnDoubled = S(mg, eg);
	mg = Mg(pawnIsolatedOrg) + GetVal(split, 4);
	eg = Eg(pawnIsolatedOrg) + GetVal(split, 5);
	pawnIsolated = S(mg, eg);

	splitInt(options.rook, split, ' ');
	mg = Mg(rookOpenOrg) + GetVal(split, 0);
	eg = Eg(rookOpenOrg) + GetVal(split, 1);
	rookOpen = S(mg, eg);
	mg = Mg(rookSemiOpenOrg) + GetVal(split, 2);
	eg = Eg(rookSemiOpenOrg) + GetVal(split, 3);
	rookSemiOpen = S(mg, eg);

	splitInt(options.outpost, split, ' ');
	for (int p = 0; p < 2; p++)
		for (int s = 0; s < 2; s++) {
			int n = (p * 2 + s) * 2;
			mg = Mg(OutpostOrg[p][s]) + GetVal(split, n);
			eg = Eg(OutpostOrg[p][s]) + GetVal(split, n + 1);
			Outpost[p][s] = S(mg, eg);
		}
	splitInt(options.king, split, ' ');
	mg = Mg(kingShieldOrg) + GetVal(split, 0);
	eg = Eg(kingShieldOrg) + GetVal(split, 1);
	kingShield = S(mg, eg);

	splitInt(options.pair, split, ' ');
	for (int n = 0; n < 3; n++) {
		mg = Mg(scorePairOrg[n + 1]) + GetVal(split, n * 2);
		eg = Eg(scorePairOrg[n + 1]) + GetVal(split, n * 2 + 1);
		scorePair[n + 1] = S(mg, eg);
	}

	for (PieceType pt = PAWN; pt < PT_NB; ++pt)
		for (Rank r = RANK_1; r < RANK_NB; ++r)
			for (File f = FILE_A; f < FILE_NB; ++f)
			{
				Square sq = CreateSquare(f, r);
				Bitboard bbAtack = pt == PAWN ? WHITE_PAWN_ATTACKS[sq] : PSEUDO_LEGAL_ATTACKS[pt][sq];
				Bonus[pt][r][f] = Score(material[pt]*2);
				Bonus[pt][r][f] += Score(OutsideRank(r) * outsideRank[pt]);
				Bonus[pt][r][f] += Score(OutsideFile(f) * outsideFile[pt]);
				if (pt == PAWN) {
					Value mg = Mg(mobility[pt]) * OutsideFile(f);
					Value eg = Eg(mobility[pt]) * pow((r - 1.0) / 5.0, 2.0);
					Bonus[pt][r][f] += S((mg * 2 + eg) / 3, (mg + eg * 2) / 3);
					//if (f==3)cout <<mg<<" "<<eg<<" "<< Eg(Bonus[pt][r][f]) << endl;
				}
				else
					Bonus[pt][r][f] += Score(PopCount(PSEUDO_LEGAL_ATTACKS[pt][sq]) * mobility[pt]);
			}
	splitInt(options.tempo, split, ' ');
	mg = Mg(tempoOrg) + GetVal(split, 0);
	eg = Eg(tempoOrg) + GetVal(split, 1);
	tempo = S(mg, eg);
}

static Bitboard GetLeastValuablePiece(Bitboard attadef, Color bySide, Piece& piece) {
	int maskColor = bySide << 3;
	for (int n = PAWN; n <= KING; n++) {
		piece = Piece(maskColor | n);
		const Bitboard subset = attadef & position.bitboard_of(piece);
		if (subset)
			return subset & -subset;    // single bit
	}
	return 0;    // empty set
}

Value See(Move m) {
	if (!m.IsCapture())
		return 0;
	Square sqFrom = m.From();
	Square sqTo = m.To();
	MoveFlags flags = m.Flags();
	Piece  capturedPiece = GetCapturedPiece(m);
	Piece  capturingPiece = GetMovingPiece(m);
	Color attacker = ColorOf(capturingPiece);
	Value gain[32]{};
	Value d = 0;
	Bitboard fromSet = SQUARE_BB[sqFrom];
	Bitboard occ = position.AllPieces();
	Bitboard sqBB = SQUARE_BB[sqTo];
	Bitboard bishopsQueens;
	Bitboard rooksQueens;
	rooksQueens = bishopsQueens = position.bitboard_of(WHITE_QUEEN) | position.bitboard_of(BLACK_QUEEN);
	rooksQueens |= position.bitboard_of(WHITE_ROOK) | position.bitboard_of(BLACK_ROOK);
	bishopsQueens |= position.bitboard_of(WHITE_BISHOP) | position.bitboard_of(BLACK_BISHOP);
	Bitboard fixed = ((Shift(NORTH_WEST, sqBB) | Shift(NORTH_EAST, sqBB)) & position.bitboard_of(BLACK_PAWN))
		| ((Shift(SOUTH_WEST, sqBB) | Shift(SOUTH_EAST, sqBB)) & position.bitboard_of(WHITE_PAWN))
		| (PSEUDO_LEGAL_ATTACKS[KNIGHT][sqTo] & (position.bitboard_of(WHITE_KNIGHT) | position.bitboard_of(BLACK_KNIGHT)))
		| (PSEUDO_LEGAL_ATTACKS[KING][sqTo] & (position.bitboard_of(WHITE_KING) | position.bitboard_of(BLACK_KING)));
	Bitboard attadef = (fixed | ((GetBishopAttacks(sqTo, occ) & bishopsQueens) | (GetRookAttacks(sqTo, occ) & rooksQueens)));
	if (m.IsCapture())
		gain[d] = materialVal[TypeOf(capturedPiece)];
	else
		gain[d] = 0;
	do {
		d++;
		attacker = ~attacker;
		gain[d] = materialVal[TypeOf(capturingPiece)] - gain[d - 1];
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

static string ShowScore(Score s) {
	string result = std::format("({} {})", Mg(s), Eg(s));
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

static Value Eval(Color color, Square sq, PieceType pt) {
	Rank rank = RelativeRank(color, RankOf(sq));
	File file = FileOf(sq);
	return Mg(Bonus[pt][rank][file]);
}

Value Eval(Move m, Value& see) {
	Square fr = m.From();
	Square to = m.To();
	MoveFlags flags = m.Flags();
	Piece piece = position.Board(fr);
	PieceType pt = TypeOf(piece);
	Color color = ColorOf(piece);
	Value score = -Eval(color, fr, pt);
	if (flags & MoveFlags::CAPTURE) {
		see = See(m);
		score += see;
	}
	else
		see = 0;
	if (flags & MoveFlags::PROMOTION)
		pt = (PieceType)(1 + flags & 3);
	return score + Eval(color, to, pt);
}

static void Eval(Position& pos, SEvalSide& esUs, SEvalSide& esEn) {
	int cw = 0;
	Color color = esUs.color;
	int maskColUs = color ? 0b1000 : 0;
	int maskColEn = color ? 0 : 0b1000;
	Bitboard bbAll = pos.AllPieces();
	Bitboard bbPawnsUs = pos.piece_bb[maskColUs];
	Bitboard bbPawnsEn = pos.piece_bb[maskColEn];
	Direction north = RelativeDir(color, NORTH);
	Direction south = RelativeDir(color, SOUTH);
	const Bitboard bbProtected = PawnAttacks(color, bbPawnsUs);
	const Bitboard bbAtacked = PawnAttacks(~color, bbPawnsEn);
	Bitboard bbConnected = bbProtected | Shift(south, bbProtected);
	bbConnected |= Shift(south, bbConnected);
	const Bitboard bbSpan = Span(~color, bbAtacked);
	const Bitboard bbOutpostRanks = color ? Rank5BB | Rank4BB | Rank3BB : Rank4BB | Rank5BB | Rank6BB;
	for (PieceType pt = PAWN; pt < PT_NB; ++pt) {
		Bitboard copy = pos.piece_bb[pt | maskColUs];
		while (copy) {
			esUs.piece[pt]++;
			phase += phases[pt];
			cw += chance[pt];
			const Square sq = pop_lsb(&copy);
			const Rank r = RankOf(sq);
			const Rank rank = RelativeRank(color, r);
			const File file = FileOf(sq);
			esUs.scorePiece[pt] += Bonus[pt][rank][file];
			const Bitboard bbPiece = 1ULL << sq;

			if (pt == PAWN) {
				//passed pawn
				if (!(bbPassedPawnMask[color][sq] & bbPawnsEn)) {
					Value v = OutsideFile(file) * passedFile;
					v+= passedRank * pow((rank - 1.0) / 5.0, 2.0);
					if (Shift(RelativeDir(color, NORTH), bbPiece) & pos.AllPieces(~color))
						v += passedBlocked;
					v += passedKU * (rank - 1) * Distance(esUs.king, sq);
					v += passedKE * (rank - 1) * Distance(esEn.king, sq);
					esUs.scorePawnPassed += S(0, v);
				}
				if (bbPiece & bbConnected)
					esUs.scorePawnConnected += Score(pawnConnected * (rank - 1));
				else {
					if (bbPawnsUs & bbForwardFiles[color][sq])
						esUs.scorePawnDoubled += pawnDoubled;
					if (bbPawnsUs & bbAdjacentFiles[file])
						esUs.scorePawnIsolated += pawnIsolated;
				}
			}
			else if (pt == KING) {
				esUs.king = sq;
				Bitboard bbShield = Shift(north, bbPiece);
				bbShield |= Shift(EAST, bbShield) | Shift(WEST, bbShield);
				Bitboard bbShield2 = bbShield | Shift(north, bbShield);
				esUs.scorePiece[pt] += Score((SparsePopCount(bbShield & bbPawnsUs) + SparsePopCount(bbShield2 & bbPawnsUs)) * kingShield);
			}
			else {
				esUs.scoreMobility += Score(PopCount(attacks(pt, sq, bbAll) & ~bbAtacked) * mobility[pt]);
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
						esUs.scorePiece[pt] += Score(Outpost[pt == BISHOP][bbProtected && bbPiece] * 2);
					else if (bbOutpost & attacks((PieceType)pt, sq, pos.AllPieces()))
						esUs.scorePiece[pt] += Outpost[pt == BISHOP][bbProtected && bbPiece];
				}
			}
		}
	}
	for (PieceType pt = KNIGHT; pt < QUEEN; ++pt)
		if (esUs.piece[pt] > 1)
			esUs.scorePair += scorePair[pt];
	esUs.chance = cw > 2;
	if (esUs.chance)
		esUs.score += S(100, 100);
}


static Score ScoreSe(SEvalSide es) {
	Score result = es.score + es.scorePawnPassed + es.scorePawnConnected + es.scorePawnDoubled + es.scorePawnIsolated + es.scoreMobility + es.scorePair;
	for (Score s : es.scorePiece)
		result += s;
	return result;
}

Value ShowEval() {
	position.SetFen("1b1rr1k1/3q1pp1/8/NP1pPb1p/1B1Pp1n1/PQR1P1P1/2n1B1nP/5RK1 b - - 1 1");
	//position.SetFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
	//position.SetFen("3rr1k1/1b1q1pb1/8/NP1p3p/1B1Pp1n1/PQR1P1P1/1B3P1P/5RK1 b - - 1 1");
	//position.SetFen("8/8/6K1/8/4k3/8/2p5/8 b - - 0 46");
	phase = 0;
	SEvalSide esW = {};
	SEvalSide esB = {};
	esW.color = WHITE;
	esB.color = BLACK;
	esW.king = bsf(position.piece_bb[WHITE_KING]);
	esB.king = bsf(position.piece_bb[BLACK_KING]);
	Eval(position, esW, esB);
	Eval(position, esB, esW);
	Picker picker;
	position.MoveList(position.ColorUs(), picker.mList, picker.count);
	picker.Fill();
	picker.Sort();
	std::cout << "moves:" << endl;
	for (int n = 0; n < picker.count; n++) {
		PickerE pe = picker.pList[n];
		std::cout << pe.move.ToUci() << " " << pe.see << " " << pe.score << endl;
	}
	std::cout << position << std::endl;
	ShowScore("pawn connected", esW.scorePawnConnected, esB.scorePawnConnected);
	ShowScore("pawn isolated", esW.scorePawnIsolated, esB.scorePawnIsolated);
	ShowScore("pawn doubled", esW.scorePawnDoubled, esB.scorePawnDoubled);
	ShowScore("pawn passed", esW.scorePawnPassed, esB.scorePawnPassed);
	ShowScore("pawn", esW.scorePiece[0], esB.scorePiece[0]);
	ShowScore("knight", esW.scorePiece[1], esB.scorePiece[1]);
	ShowScore("bishop", esW.scorePiece[2], esB.scorePiece[2]);
	ShowScore("rook", esW.scorePiece[3], esB.scorePiece[3]);
	ShowScore("queen", esW.scorePiece[4], esB.scorePiece[4]);
	ShowScore("king", esW.scorePiece[5], esB.scorePiece[5]);
	ShowScore("mobility", esW.scoreMobility, esB.scoreMobility);
	ShowScore("pair", esW.scorePair, esB.scorePair);
	ShowScore("total", ScoreSe(esW), ScoreSe(esB));
	std::cout << "phase " << phase << endl;
	if (!esW.chance && !esB.chance)
		return 0;
	Score score = ScoreSe(esW) - ScoreSe(esB);
	score += position.ColorUs() ? -tempo : tempo;
	Value v = (Pg(score) * (100 - position.move50)) / 200;
	return position.ColorUs() ? -v : v;
}

Value Eval() {
	phase = 0;
	SEvalSide esW = {};
	SEvalSide esB = {};
	esW.color = WHITE;
	esB.color = BLACK;
	esW.king = bsf(position.piece_bb[WHITE_KING]);
	esB.king = bsf(position.piece_bb[BLACK_KING]);
	Eval(position, esW, esB);
	Eval(position, esB, esW);
	if (!esW.chance && !esB.chance)
		return 0;
	Score score = ScoreSe(esW) - ScoreSe(esB);
	score += position.ColorUs() ? -tempo : tempo;
	Value v = (Pg(score) * (100 - position.move50)) / 200;
	return position.ColorUs() ? -v : v;
}