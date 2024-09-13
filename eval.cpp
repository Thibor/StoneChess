#include "eval.h"

int phase;

constexpr DScore S(const Score mg, const Score eg) {
	return (eg << 16) + mg;
}

constexpr Score Mg(DScore score) {
	return (short)score;
}

constexpr Score Eg(DScore score) {
	return (score + 0x8000) >> 16;
}

constexpr Score Ag(DScore score) {
	return max(Mg(score), Eg(score));
}

constexpr Score Pg(DScore score) {
	return (Mg(score) * phase + Eg(score) * (24 - phase)) / 24;
}

U64 shelterKingW = 0xE7;
U64 shelterKingB = 0xE700000000000000ull;
U64 shelterKW = 0x0700;
U64 shelterKB = 0x0007000000000000ull;
U64 shelterQW = 0xE000;
U64 shelterQB = 0x00E0000000000000ull;
U64 shelterSW = 0xC3D7;
U64 shelterSB = 0xD7C3000000000000ull;

const DScore psts[][4] = {
	{S(-18, -1), S(-0, -6), S(9, 6), S(9, 1)},
	{S(-22, -0), S(-5, -2), S(8, 1), S(20, 0)},
	{S(1, -4), S(1, -2), S(-3, 1), S(1, 4)},
	{S(-17, -1), S(3, -11), S(-10, 12), S(24, 1)},
	{S(-2, -34), S(4, -19), S(-31, 22), S(29, 30)},
	{S(-47, 0), S(-18, -1), S(60, -12), S(6, 12)},
};
const DScore centralities[] = { S(16, -14), S(15, 15), S(22, 6), S(-7, 0), S(-1, 20), S(-27, 20) };
//const DScore centralities[] = { S(12, -10), S(12, 12), S(16, 4), S(-5, 0), S(-1, 15), S(-20, 14) };
//const DScore centralities[] = { S(8, -7), S(8, 8), S(11, 3), S(-4, 0), S(-1, 10), S(-14, 10) };
const DScore outside_files[] = { S(2, -6), S(-3, -5), S(6, -4), S(-6, -2), S(-4, -1), S(2, -2) };
const DScore pawn_protection[] = { S(10, 15), S(11, 21), S(-6, 18), S(-4, 13), S(-6, 17), S(-49, 22) };
const DScore passers[] = { S(20, 6), S(10, 11), S(-2, 26), S(-7, 42), S(20, 111), S(107, 205) };
const DScore pawn_doubled = S(-21, -27);
const DScore pawn_passed_blocked = S(4, -46);
const DScore bishop_pair = S(33, 56);
const DScore rook_open = S(72, 1);
const DScore rook_semi_open = S(30, 12);
const DScore rook_rank78 = S(40, 2);
const DScore king_shield[] = { S(24, -11), S(12, -16) };
const DScore materialOrg[] = { S(79, 128), S(421, 292), S(404, 327), S(556, 596), S(1271, 1060), 0 };
const Score materialMax[] = { 128, 421, 404, 596, 1271, 0 };
const Score materialVal[] = { 100, 325, 325, 500, 1000, 10000 };
const int phases[] = { 0, 1, 1, 2, 4, 0 };
DScore material[6] = {};

inline Piece GetCapturedPiece(Move m) {
	return position.Board(m.To());
}

inline Piece GetMovingPiece(Move m) {
	return position.Board(m.From());
}

void InitEval() {
	srand(time(NULL));
	int elo = options.elo;
	if (elo < options.eloMin)
		elo = options.eloMin;
	if (elo > options.eloMax)
		elo = options.eloMax;
	elo -= options.eloMin;
	int eloRange = options.eloMax - options.eloMin;
	int range = 800 - (elo * 800) / eloRange;
	for (int r = 0; r < 5; r++) {
		int mg = (-500 * (eloRange - elo) + Mg(materialOrg[r]) * elo) / eloRange;
		int eg = Eg(materialOrg[r]);
		material[r] = S(mg, eg);
	}
}

Bitboard GetLeastValuablePiece(Bitboard attadef, Color bySide, Piece& piece) {
	int maskColor = bySide << 3;
	for (int n = PAWN; n <= KING; n++) {
		piece = Piece(maskColor | n);
		const Bitboard subset = attadef & position.bitboard_of(piece);
		if (subset)
			return subset & -subset;    // single bit
	}
	return 0;    // empty set
}

Score See(Move m) {
	if (!m.IsCapture())
		return 0;
	Square sqFrom = m.From();
	Square sqTo = m.To();
	MoveFlags flags = m.Flags();
	Piece  capturedPiece = GetCapturedPiece(m);
	Piece  capturingPiece = GetMovingPiece(m);
	Color attacker = color_of(capturingPiece);
	Score gain[32]{};
	Score d = 0;
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
	Bitboard attadef = (fixed | ((get_bishop_attacks(sqTo, occ) & bishopsQueens) | (get_rook_attacks(sqTo, occ) & rooksQueens)));
	if (m.IsCapture())
		gain[d] = materialVal[type_of(capturedPiece)];
	else
		gain[d] = 0;
	do {
		d++;
		attacker = Color(1 - attacker);
		gain[d] = materialVal[type_of(capturingPiece)] - gain[d - 1];
		if (-gain[d - 1] < 0 && gain[d] < 0)
			break;    // pruning does not influence the result
		attadef ^= fromSet;    // reset bit in set to traverse
		occ ^= fromSet;
		attadef |=
			occ & ((get_bishop_attacks(sqTo, occ) & bishopsQueens) | (get_rook_attacks(sqTo, occ) & rooksQueens));
		fromSet = GetLeastValuablePiece(attadef, attacker, capturingPiece);
	} while (fromSet);
	while (--d) {
		gain[d - 1] = -(-gain[d - 1] > gain[d] ? -gain[d - 1] : gain[d]);
	}
	return gain[0];
}

string ShowScore(int s) {
	string result = std::format("({} {})", Mg(s), Eg(s));
	int len = 16 - result.length();
	if (len < 0)
		len = 0;
	result.append(len, ' ');
	return result;
}

void ShowScore(string name, int sw, int sb) {
	int len = 16 - name.length();
	if (len < 0)
		len = 0;
	name.append(len, ' ');
	cout << name << ShowScore(sw) << " " << ShowScore(sb) << " " << ShowScore(sw - sb) << endl;
}

Score Eval(Square sq, int type) {
	const Rank rank = RankOf(sq);
	const File file = FileOf(sq);
	const Rank rankR = RelativeRank(position.ColorUs(), rank);
	Score result = materialVal[type] + rankR;
	return result + 3 - floor(abs(file - 3.5));
}

Score Eval(Move m, Score& see) {
	Square fr = m.From();
	Square to = m.To();
	MoveFlags flags = m.Flags();
	Piece pfr = position.Board(fr);
	PieceType frType = type_of(pfr);
	Score score = -Eval(fr, frType);
	if (flags & MoveFlags::CAPTURE) {
		see = See(m);
		score += see;
	}
	else
		see = 0;
	if (flags & MoveFlags::PROMOTION)
		frType = (PieceType)(1 + flags & 3);
	return score + Eval(to, frType);
}

int Centrality(Rank rank, File file) {
	return 4 - max(abs(rank * 2 - 7) / 2, abs(file * 2 - 7) / 2);
}

int Centrality(Square sq) {
	return Centrality(RankOf(sq), FileOf(sq));
}

SEval Eval(Position& pos, Color color, Square kpUs, Square kpEn) {
	SEval result;
	int maskColUs = color ? 0b1000 : 0;
	int maskColEn = color ? 0 : 0b1000;
	Bitboard pawnsUs = pos.piece_bb[maskColUs];
	Bitboard pawnsEn = pos.piece_bb[maskColEn];
	const Bitboard protected_by_pawns = Shift(RelativeDir(color, NORTH_WEST), pawnsUs) | Shift(RelativeDir(color, NORTH_EAST), pawnsUs);

	// Bishop pair
	Bitboard bbBishop = pos.piece_bb[BISHOP | maskColUs];
	if ((bbBishop & bbLight) && (bbBishop & bbDark))
		result.scoreBishopPair += bishop_pair;
	// For each piece type
	for (int p = 0; p < 6; ++p) {
		Bitboard copy = pos.piece_bb[p | maskColUs];
		while (copy) {
			phase += phases[p];
			const Square sq = pop_lsb(&copy);
			//const Rank rank = ;
			const Rank rank = RelativeRank(color, RankOf(sq));
			const File file = FileOf(sq);
			//const int centrality = (7 - abs(7 - rankR - file) - abs(rankR - file)) / 2;
			//const int centrality = ;
			// Material
			result.scorePiece[p] += material[p];

			// Centrality
			result.scorePiece[p] += Centrality(rank, file) * centralities[p];

			// Closeness to outside files
			result.scorePiece[p] += abs(file - 3) * outside_files[p];

			// Quadrant PSTs
			result.scorePiece[p] += psts[p][(rank / 4) * 2 + file / 4];

			// Pawn protection
			const Bitboard piece_bb = 1ULL << sq;
			if (piece_bb & protected_by_pawns)
				result.scorePiece[p] += pawn_protection[p];

			if (p == PAWN) {
				// Passed pawns
				Bitboard blockers = color ? 0x8080808080808080ULL >> (63 - sq) : 0x0101010101010101ULL << sq;
				blockers = Shift(RelativeDir(color, NORTH_WEST), blockers) | Shift(RelativeDir(color, NORTH_EAST), blockers);
				//if(!c)print_bitboard(blockers);
				if (!(blockers & pawnsEn)) {
					result.scorePawnPassed += passers[rank - 1];

					// Blocked passed pawns
					if (Shift(RelativeDir(color, NORTH), piece_bb) & pos.AllPieces(~color))
						result.scorePawnBlocked += pawn_passed_blocked;
					// King defense/attack
					// king distance to square in front of passer
					int i1 = abs(((int)kpUs / 8) - (rank + 1));
					int i2 = abs(((int)kpUs % 8) - file);
					result.scorePawnPassed -= S(0, 1) * (rank - 1) * max(i1, i2);
					i1 = abs(((int)kpEn / 8) - (rank + 1));
					i2 = abs(((int)kpEn % 8) - file);
					result.scorePawnPassed += S(0, 3) * (rank - 1) * max(i1, i2);
					//cout << SQSTR[sq] << " mg " << Mg(score - oldScore) << endl;
				}

				// Doubled pawns
				if ((Shift(RelativeDir(color, NORTH), piece_bb) | Shift(RelativeDir(color, NORTH), Shift(RelativeDir(color, NORTH), piece_bb))) & pawnsUs)
					result.scorePawnDoubled += pawn_doubled;
			}
			else if (p == ROOK) {
				// Rook on open or semi-open files
				const Bitboard file_bb = 0x101010101010101ULL << file;
				if (!(file_bb & pawnsUs)) {
					if (!(file_bb & pawnsEn))
						result.scorePiece[p] += rook_open;
					else
						result.scorePiece[p] += rook_semi_open;
				}

				// Rook on 7th or 8th rank
				if (rank >= 6)result.scorePiece[p] += rook_rank78;
			}
			else if (p == KING)
				if (piece_bb & (color ? shelterKingB : shelterKingW)) {
					const Bitboard shield = file < 3 ? (color ? shelterKB : shelterKW) : (color ? shelterQB : shelterQW);
					result.scoreKingShelter += sparse_pop_count(shield & pawnsUs) * king_shield[0];
					result.scoreKingShelter += sparse_pop_count(Shift(RelativeDir(color, NORTH), shield) & pawnsUs) * king_shield[1];
					result.scoreKingShelter -= !(piece_bb & (color ? shelterSB : shelterSW)) * king_shield[0];
				}
				else result.scoreKingShelter -= !(piece_bb & (color ? shelterSB : shelterSW)) * king_shield[0];
		}
	}
	return result;
}

DScore ScoreSe(SEval se) {
	DScore result = se.scorePawnPassed + se.scorePawnBlocked + se.scorePawnDoubled + se.scoreBishopPair + se.scoreKingShelter;
	for (int s : se.scorePiece)
		result += s;
	return result;
}

Score ShowEval() {
	position.SetFen("1b1rr1k1/3q1pp1/8/NP1p1b1p/1B1Pp1n1/PQR1P1P1/4BP1P/5RK1 w - -0 1");
	phase = 0;
	Square kpW = bsf(position.piece_bb[WHITE_KING]);
	Square kpB = bsf(position.piece_bb[BLACK_KING]);
	SEval seW = Eval(position, WHITE, kpW, kpB);
	SEval seB = Eval(position, BLACK, kpB, kpW);
	std::cout << position << std::endl;
	ShowScore("pawn", seW.scorePiece[0], seB.scorePiece[0]);
	ShowScore("pawn passed", seW.scorePawnPassed, seB.scorePawnPassed);
	ShowScore("pawn blocked", seW.scorePawnBlocked, seB.scorePawnBlocked);
	ShowScore("pawn doubled", seW.scorePawnDoubled, seB.scorePawnDoubled);
	ShowScore("knight", seW.scorePiece[1], seB.scorePiece[1]);
	ShowScore("bishop", seW.scorePiece[2], seB.scorePiece[2]);
	ShowScore("bishop pair", seW.scoreBishopPair, seB.scoreBishopPair);
	ShowScore("rook", seW.scorePiece[3], seB.scorePiece[3]);
	ShowScore("queen", seW.scorePiece[4], seB.scorePiece[4]);
	ShowScore("king", seW.scorePiece[5], seB.scorePiece[5]);
	ShowScore("king shelter", seW.scoreKingShelter, seB.scoreKingShelter);
	ShowScore("total", ScoreSe(seW), ScoreSe(seB));
	cout << "phase " << phase << endl;
	DScore score = S(10, 10) + ScoreSe(seW) - ScoreSe(seB);
	score = Pg(score);
	return position.ColorUs() ? -score : score;
}

Score Eval() {
	phase = 0;
	Square kpW = bsf(position.piece_bb[WHITE_KING]);
	Square kpB = bsf(position.piece_bb[BLACK_KING]);
	SEval seW = Eval(position, WHITE, kpW, kpB);
	SEval seB = Eval(position, BLACK, kpB, kpW);
	DScore score = S(10, 10) + ScoreSe(seW) - ScoreSe(seB);
	score = Pg(score);
	return position.ColorUs() ? -score : score;
}