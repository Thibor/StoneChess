#include "program.h"

//A lookup table for king move bitboards
const Bitboard KING_ATTACKS[64] = {
	0x302, 0x705, 0xe0a, 0x1c14,
	0x3828, 0x7050, 0xe0a0, 0xc040,
	0x30203, 0x70507, 0xe0a0e, 0x1c141c,
	0x382838, 0x705070, 0xe0a0e0, 0xc040c0,
	0x3020300, 0x7050700, 0xe0a0e00, 0x1c141c00,
	0x38283800, 0x70507000, 0xe0a0e000, 0xc040c000,
	0x302030000, 0x705070000, 0xe0a0e0000, 0x1c141c0000,
	0x3828380000, 0x7050700000, 0xe0a0e00000, 0xc040c00000,
	0x30203000000, 0x70507000000, 0xe0a0e000000, 0x1c141c000000,
	0x382838000000, 0x705070000000, 0xe0a0e0000000, 0xc040c0000000,
	0x3020300000000, 0x7050700000000, 0xe0a0e00000000, 0x1c141c00000000,
	0x38283800000000, 0x70507000000000, 0xe0a0e000000000, 0xc040c000000000,
	0x302030000000000, 0x705070000000000, 0xe0a0e0000000000, 0x1c141c0000000000,
	0x3828380000000000, 0x7050700000000000, 0xe0a0e00000000000, 0xc040c00000000000,
	0x203000000000000, 0x507000000000000, 0xa0e000000000000, 0x141c000000000000,
	0x2838000000000000, 0x5070000000000000, 0xa0e0000000000000, 0x40c0000000000000,
};

//A lookup table for knight move bitboards
const Bitboard KNIGHT_ATTACKS[64] = {
	0x20400, 0x50800, 0xa1100, 0x142200,
	0x284400, 0x508800, 0xa01000, 0x402000,
	0x2040004, 0x5080008, 0xa110011, 0x14220022,
	0x28440044, 0x50880088, 0xa0100010, 0x40200020,
	0x204000402, 0x508000805, 0xa1100110a, 0x1422002214,
	0x2844004428, 0x5088008850, 0xa0100010a0, 0x4020002040,
	0x20400040200, 0x50800080500, 0xa1100110a00, 0x142200221400,
	0x284400442800, 0x508800885000, 0xa0100010a000, 0x402000204000,
	0x2040004020000, 0x5080008050000, 0xa1100110a0000, 0x14220022140000,
	0x28440044280000, 0x50880088500000, 0xa0100010a00000, 0x40200020400000,
	0x204000402000000, 0x508000805000000, 0xa1100110a000000, 0x1422002214000000,
	0x2844004428000000, 0x5088008850000000, 0xa0100010a0000000, 0x4020002040000000,
	0x400040200000000, 0x800080500000000, 0x1100110a00000000, 0x2200221400000000,
	0x4400442800000000, 0x8800885000000000, 0x100010a000000000, 0x2000204000000000,
	0x4020000000000, 0x8050000000000, 0x110a0000000000, 0x22140000000000,
	0x44280000000000, 0x0088500000000000, 0x0010a00000000000, 0x20400000000000
};

//A lookup table for white pawn move bitboards
const Bitboard WHITE_PAWN_ATTACKS[64] = {
	0x200, 0x500, 0xa00, 0x1400,
	0x2800, 0x5000, 0xa000, 0x4000,
	0x20000, 0x50000, 0xa0000, 0x140000,
	0x280000, 0x500000, 0xa00000, 0x400000,
	0x2000000, 0x5000000, 0xa000000, 0x14000000,
	0x28000000, 0x50000000, 0xa0000000, 0x40000000,
	0x200000000, 0x500000000, 0xa00000000, 0x1400000000,
	0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000,
	0x20000000000, 0x50000000000, 0xa0000000000, 0x140000000000,
	0x280000000000, 0x500000000000, 0xa00000000000, 0x400000000000,
	0x2000000000000, 0x5000000000000, 0xa000000000000, 0x14000000000000,
	0x28000000000000, 0x50000000000000, 0xa0000000000000, 0x40000000000000,
	0x200000000000000, 0x500000000000000, 0xa00000000000000, 0x1400000000000000,
	0x2800000000000000, 0x5000000000000000, 0xa000000000000000, 0x4000000000000000,
	0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0,
};

//A lookup table for black pawn move bitboards
const Bitboard BLACK_PAWN_ATTACKS[64] = {
	0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0,
	0x2, 0x5, 0xa, 0x14,
	0x28, 0x50, 0xa0, 0x40,
	0x200, 0x500, 0xa00, 0x1400,
	0x2800, 0x5000, 0xa000, 0x4000,
	0x20000, 0x50000, 0xa0000, 0x140000,
	0x280000, 0x500000, 0xa00000, 0x400000,
	0x2000000, 0x5000000, 0xa000000, 0x14000000,
	0x28000000, 0x50000000, 0xa0000000, 0x40000000,
	0x200000000, 0x500000000, 0xa00000000, 0x1400000000,
	0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000,
	0x20000000000, 0x50000000000, 0xa0000000000, 0x140000000000,
	0x280000000000, 0x500000000000, 0xa00000000000, 0x400000000000,
	0x2000000000000, 0x5000000000000, 0xa000000000000, 0x14000000000000,
	0x28000000000000, 0x50000000000000, 0xa0000000000000, 0x40000000000000,
};



//Reverses a bitboard                        
Bitboard reverse(Bitboard b) {
	b = (b & 0x5555555555555555) << 1 | (b >> 1) & 0x5555555555555555;
	b = (b & 0x3333333333333333) << 2 | (b >> 2) & 0x3333333333333333;
	b = (b & 0x0f0f0f0f0f0f0f0f) << 4 | (b >> 4) & 0x0f0f0f0f0f0f0f0f;
	b = (b & 0x00ff00ff00ff00ff) << 8 | (b >> 8) & 0x00ff00ff00ff00ff;

	return (b << 48) | ((b & 0xffff0000) << 16) |
		((b >> 16) & 0xffff0000) | (b >> 48);
}

//Calculates sliding attacks from a given square, on a given axis, taking into
//account the blocking pieces. This uses the Hyperbola Quintessence Algorithm.
Bitboard sliding_attacks(Square square, Bitboard occ, Bitboard mask) {
	return (((mask & occ) - SQUARE_BB[square] * 2) ^
		reverse(reverse(mask & occ) - reverse(SQUARE_BB[square]) * 2)) & mask;
}

//Returns rook attacks from a given square, using the Hyperbola Quintessence Algorithm. Only used to initialize
//the magic lookup table
Bitboard get_rook_attacks_for_init(Square square, Bitboard occ) {
	return sliding_attacks(square, occ, MASK_FILE[FileOf(square)]) |
		sliding_attacks(square, occ, MASK_RANK[RankOf(square)]);
}

Bitboard ROOK_ATTACK_MASKS[64];
int ROOK_ATTACK_SHIFTS[64];
Bitboard ROOK_ATTACKS[64][4096];

const Bitboard ROOK_MAGICS[64] = {
	0x0080001020400080, 0x0040001000200040, 0x0080081000200080, 0x0080040800100080,
	0x0080020400080080, 0x0080010200040080, 0x0080008001000200, 0x0080002040800100,
	0x0000800020400080, 0x0000400020005000, 0x0000801000200080, 0x0000800800100080,
	0x0000800400080080, 0x0000800200040080, 0x0000800100020080, 0x0000800040800100,
	0x0000208000400080, 0x0000404000201000, 0x0000808010002000, 0x0000808008001000,
	0x0000808004000800, 0x0000808002000400, 0x0000010100020004, 0x0000020000408104,
	0x0000208080004000, 0x0000200040005000, 0x0000100080200080, 0x0000080080100080,
	0x0000040080080080, 0x0000020080040080, 0x0000010080800200, 0x0000800080004100,
	0x0000204000800080, 0x0000200040401000, 0x0000100080802000, 0x0000080080801000,
	0x0000040080800800, 0x0000020080800400, 0x0000020001010004, 0x0000800040800100,
	0x0000204000808000, 0x0000200040008080, 0x0000100020008080, 0x0000080010008080,
	0x0000040008008080, 0x0000020004008080, 0x0000010002008080, 0x0000004081020004,
	0x0000204000800080, 0x0000200040008080, 0x0000100020008080, 0x0000080010008080,
	0x0000040008008080, 0x0000020004008080, 0x0000800100020080, 0x0000800041000080,
	0x00FFFCDDFCED714A, 0x007FFCDDFCED714A, 0x003FFFCDFFD88096, 0x0000040810002101,
	0x0001000204080011, 0x0001000204000801, 0x0001000082000401, 0x0001FFFAABFAD1A2
};

//Initializes the magic lookup table for rooks
void initialise_rook_attacks() {
	Bitboard edges, subset, index;

	for (Square sq = SQ_A1; sq <= SQ_H8; ++sq) {
		edges = ((MASK_RANK[FILE_A] | MASK_RANK[FILE_H]) & ~MASK_RANK[RankOf(sq)]) |
			((MASK_FILE[FILE_A] | MASK_FILE[FILE_H]) & ~MASK_FILE[FileOf(sq)]);
		ROOK_ATTACK_MASKS[sq] = (MASK_RANK[RankOf(sq)]
			^ MASK_FILE[FileOf(sq)]) & ~edges;
		ROOK_ATTACK_SHIFTS[sq] = 64 - PopCount(ROOK_ATTACK_MASKS[sq]);

		subset = 0;
		do {
			index = subset;
			index = index * ROOK_MAGICS[sq];
			index = index >> ROOK_ATTACK_SHIFTS[sq];
			ROOK_ATTACKS[sq][index] = get_rook_attacks_for_init(sq, subset);
			subset = (subset - ROOK_ATTACK_MASKS[sq]) & ROOK_ATTACK_MASKS[sq];
		} while (subset);
	}
}

//Returns the attacks bitboard for a rook at a given square, using the magic lookup table
constexpr Bitboard GetRookAttacks(Square square, Bitboard occ) {
	return ROOK_ATTACKS[square][((occ & ROOK_ATTACK_MASKS[square]) * ROOK_MAGICS[square])
		>> ROOK_ATTACK_SHIFTS[square]];
}

//Returns the 'x-ray attacks' for a rook at a given square. X-ray attacks cover squares that are not immediately
//accessible by the rook, but become available when the immediate blockers are removed from the board 
Bitboard get_xray_rook_attacks(Square square, Bitboard occ, Bitboard blockers) {
	Bitboard attacks = GetRookAttacks(square, occ);
	blockers &= attacks;
	return attacks ^ GetRookAttacks(square, occ ^ blockers);
}

//Returns bishop attacks from a given square, using the Hyperbola Quintessence Algorithm. Only used to initialize
//the magic lookup table
Bitboard get_bishop_attacks_for_init(Square square, Bitboard occ) {
	return sliding_attacks(square, occ, MASK_DIAGONAL[diagonal_of(square)]) |
		sliding_attacks(square, occ, MASK_ANTI_DIAGONAL[anti_diagonal_of(square)]);
}

Bitboard BISHOP_ATTACK_MASKS[64];
int BISHOP_ATTACK_SHIFTS[64];
Bitboard BISHOP_ATTACKS[64][512];

const Bitboard BISHOP_MAGICS[64] = {
	0x0002020202020200, 0x0002020202020000, 0x0004010202000000, 0x0004040080000000,
	0x0001104000000000, 0x0000821040000000, 0x0000410410400000, 0x0000104104104000,
	0x0000040404040400, 0x0000020202020200, 0x0000040102020000, 0x0000040400800000,
	0x0000011040000000, 0x0000008210400000, 0x0000004104104000, 0x0000002082082000,
	0x0004000808080800, 0x0002000404040400, 0x0001000202020200, 0x0000800802004000,
	0x0000800400A00000, 0x0000200100884000, 0x0000400082082000, 0x0000200041041000,
	0x0002080010101000, 0x0001040008080800, 0x0000208004010400, 0x0000404004010200,
	0x0000840000802000, 0x0000404002011000, 0x0000808001041000, 0x0000404000820800,
	0x0001041000202000, 0x0000820800101000, 0x0000104400080800, 0x0000020080080080,
	0x0000404040040100, 0x0000808100020100, 0x0001010100020800, 0x0000808080010400,
	0x0000820820004000, 0x0000410410002000, 0x0000082088001000, 0x0000002011000800,
	0x0000080100400400, 0x0001010101000200, 0x0002020202000400, 0x0001010101000200,
	0x0000410410400000, 0x0000208208200000, 0x0000002084100000, 0x0000000020880000,
	0x0000001002020000, 0x0000040408020000, 0x0004040404040000, 0x0002020202020000,
	0x0000104104104000, 0x0000002082082000, 0x0000000020841000, 0x0000000000208800,
	0x0000000010020200, 0x0000000404080200, 0x0000040404040400, 0x0002020202020200
};

//Initializes the magic lookup table for bishops
void initialise_bishop_attacks() {
	Bitboard edges, subset, index;

	for (Square sq = SQ_A1; sq <= SQ_H8; ++sq) {
		edges = ((MASK_RANK[FILE_A] | MASK_RANK[FILE_H]) & ~MASK_RANK[RankOf(sq)]) |
			((MASK_FILE[FILE_A] | MASK_FILE[FILE_H]) & ~MASK_FILE[FileOf(sq)]);
		BISHOP_ATTACK_MASKS[sq] = (MASK_DIAGONAL[diagonal_of(sq)]
			^ MASK_ANTI_DIAGONAL[anti_diagonal_of(sq)]) & ~edges;
		BISHOP_ATTACK_SHIFTS[sq] = 64 - PopCount(BISHOP_ATTACK_MASKS[sq]);

		subset = 0;
		do {
			index = subset;
			index = index * BISHOP_MAGICS[sq];
			index = index >> BISHOP_ATTACK_SHIFTS[sq];
			BISHOP_ATTACKS[sq][index] = get_bishop_attacks_for_init(sq, subset);
			subset = (subset - BISHOP_ATTACK_MASKS[sq]) & BISHOP_ATTACK_MASKS[sq];
		} while (subset);
	}
}

//Returns the attacks bitboard for a bishop at a given square, using the magic lookup table
constexpr Bitboard GetBishopAttacks(Square square, Bitboard occ) {
	return BISHOP_ATTACKS[square][((occ & BISHOP_ATTACK_MASKS[square]) * BISHOP_MAGICS[square])
		>> BISHOP_ATTACK_SHIFTS[square]];
}

//Returns the 'x-ray attacks' for a bishop at a given square. X-ray attacks cover squares that are not immediately
//accessible by the rook, but become available when the immediate blockers are removed from the board 
Bitboard get_xray_bishop_attacks(Square square, Bitboard occ, Bitboard blockers) {
	Bitboard attacks = GetBishopAttacks(square, occ);
	blockers &= attacks;
	return attacks ^ GetBishopAttacks(square, occ ^ blockers);
}


Bitboard SQUARES_BETWEEN_BB[64][64];

//Initializes the lookup table for the bitboard of squares in between two given squares (0 if the 
//two squares are not aligned)
void initialise_squares_between() {
	Bitboard sqs;
	for (Square sq1 = SQ_A1; sq1 <= SQ_H8; ++sq1)
		for (Square sq2 = SQ_A1; sq2 <= SQ_H8; ++sq2) {
			sqs = SQUARE_BB[sq1] | SQUARE_BB[sq2];
			if (FileOf(sq1) == FileOf(sq2) || RankOf(sq1) == RankOf(sq2))
				SQUARES_BETWEEN_BB[sq1][sq2] =
				get_rook_attacks_for_init(sq1, sqs) & get_rook_attacks_for_init(sq2, sqs);
			else if (diagonal_of(sq1) == diagonal_of(sq2) || anti_diagonal_of(sq1) == anti_diagonal_of(sq2))
				SQUARES_BETWEEN_BB[sq1][sq2] =
				get_bishop_attacks_for_init(sq1, sqs) & get_bishop_attacks_for_init(sq2, sqs);
		}
}


Bitboard LINE[64][64];

//Initializes the lookup table for the bitboard of all squares along the line of two given squares (0 if the 
//two squares are not aligned)
void initialise_line() {
	for (Square sq1 = SQ_A1; sq1 <= SQ_H8; ++sq1)
		for (Square sq2 = SQ_A1; sq2 <= SQ_H8; ++sq2) {
			if (FileOf(sq1) == FileOf(sq2) || RankOf(sq1) == RankOf(sq2))
				LINE[sq1][sq2] =
				get_rook_attacks_for_init(sq1, 0) & get_rook_attacks_for_init(sq2, 0)
				| SQUARE_BB[sq1] | SQUARE_BB[sq2];
			else if (diagonal_of(sq1) == diagonal_of(sq2) || anti_diagonal_of(sq1) == anti_diagonal_of(sq2))
				LINE[sq1][sq2] =
				get_bishop_attacks_for_init(sq1, 0) & get_bishop_attacks_for_init(sq2, 0)
				| SQUARE_BB[sq1] | SQUARE_BB[sq2];
		}
}


Bitboard PAWN_ATTACKS[COLOR_NB][SQUARE_NB];
Bitboard PSEUDO_LEGAL_ATTACKS[PT_NB][SQUARE_NB];
Bitboard bbForwardFiles[COLOR_NB][SQUARE_NB];
Bitboard bbForwardRank[COLOR_NB][RANK_NB];
Bitboard bbPawnAttackSpan[COLOR_NB][SQUARE_NB];
Bitboard bbAdjacentFiles[FILE_NB];
Bitboard bbPassedPawnMask[COLOR_NB][SQUARE_NB];

//Initializes the table containg pseudolegal attacks of each piece for each square. This does not include blockers
//for sliding pieces
void initialise_pseudo_legal() {
	memcpy(PAWN_ATTACKS[WHITE], WHITE_PAWN_ATTACKS, sizeof(WHITE_PAWN_ATTACKS));
	memcpy(PAWN_ATTACKS[BLACK], BLACK_PAWN_ATTACKS, sizeof(BLACK_PAWN_ATTACKS));
	memcpy(PSEUDO_LEGAL_ATTACKS[KNIGHT], KNIGHT_ATTACKS, sizeof(KNIGHT_ATTACKS));
	memcpy(PSEUDO_LEGAL_ATTACKS[KING], KING_ATTACKS, sizeof(KING_ATTACKS));
	for (Square s = SQ_A1; s <= SQ_H8; ++s) {
		PSEUDO_LEGAL_ATTACKS[ROOK][s] = get_rook_attacks_for_init(s, 0);
		PSEUDO_LEGAL_ATTACKS[BISHOP][s] = get_bishop_attacks_for_init(s, 0);
		PSEUDO_LEGAL_ATTACKS[QUEEN][s] = PSEUDO_LEGAL_ATTACKS[ROOK][s] |
			PSEUDO_LEGAL_ATTACKS[BISHOP][s];
	}
}

//Initializes lookup tables for rook moves, bishop moves, in-between squares, aligned squares and pseudolegal moves
void initialise_all_databases() {
	initialise_rook_attacks();
	initialise_bishop_attacks();
	initialise_squares_between();
	initialise_line();
	initialise_pseudo_legal();
	for (int f = FILE_A; f <= FILE_H; ++f)
		bbAdjacentFiles[f] = (f > FILE_A ? MASK_FILE[f - 1] : 0) | (f < FILE_H ? MASK_FILE[f + 1] : 0);
	for (int r = RANK_1; r < RANK_8; ++r)
		bbForwardRank[WHITE][r] = ~(bbForwardRank[BLACK][r + 1] = bbForwardRank[BLACK][r] | MASK_RANK[r]);
	for (int c = WHITE; c <= BLACK; ++c)
		for (Square s = SQ_A1; s <= SQ_H8; ++s)
		{
			bbForwardFiles[c][s] = bbForwardRank[c][RankOf(s)] & MASK_FILE[FileOf(s)];
			bbPawnAttackSpan[c][s] = bbForwardRank[c][RankOf(s)] & bbAdjacentFiles[FileOf(s)];
			bbPassedPawnMask[c][s] = bbForwardFiles[c][s] | bbPawnAttackSpan[c][s];
		}
}

Position g_pos;

U64 zobrist::hashColor;
//Zobrist keys for each piece and each square
//Used to incrementally update the hash key of a position
U64 zobrist::zobrist_table[PIECE_NB][SQUARE_NB];

//Initializes the zobrist table with random 64-bit numbers
void zobrist::InitialiseZobristKeys() {
	PRNG rng(70026072);
	hashColor = rng.rand<U64>();
	for (int i = 0; i < PIECE_NB; i++)
		for (int j = 0; j < SQUARE_NB; j++)
			zobrist::zobrist_table[i][j] = rng.rand<U64>();
}

//Pretty-prints the position (including FEN and hash key)
void Position::PrintBoard() const {
	string s = "   +---+---+---+---+---+---+---+---+";
	string t = "     A   B   C   D   E   F   G   H";
	cout << t << endl;
	for (int i = 56; i >= 0; i -= 8) {
		cout << s << endl;
		cout << " " << i / 8 + 1 << " ";
		for (int j = 0; j < 8; j++) {
			Piece piece = board[i + j];
			cout << "| " << PIECE_PRT[piece] << " ";
		}
		cout << "| " << i / 8 + 1 << endl;
	}
	cout << s << endl;
	cout << t << endl;
	cout << "FEN : " << GetFen() << endl;
	cout << "Hash: 0x" << std::hex << hash << std::dec << endl;
	cout << "Side: " << (color == WHITE ? "white" : "black") << endl;
}

//Adds, to the move pointer all moves of the form (from, s), where s is a square in the bitboard to
template<MoveFlags F = QUIET>
inline Move* make(Square from, Bitboard to, Move* list) {
	while (to) *list++ = Move(from, pop_lsb(&to), F);
	return list;
}

//Adds, to the move pointer all quiet promotion moves of the form (from, s), where s is a square in the bitboard to
template<>
inline Move* make<PROMOTIONS>(Square from, Bitboard b, Move* list) {
	Square p;
	while (b) {
		p = pop_lsb(&b);
		*list++ = Move(from, p, PR_KNIGHT);
		*list++ = Move(from, p, PR_BISHOP);
		*list++ = Move(from, p, PR_ROOK);
		*list++ = Move(from, p, PR_QUEEN);
	}
	return list;
}

//Adds, to the move pointer all capture promotion moves of the form (from, s), where s is a square in the bitboard to
template<>
inline Move* make<PROMOTION_CAPTURES>(Square from, Bitboard b, Move* list) {
	Square p;
	while (b) {
		p = pop_lsb(&b);
		*list++ = Move(from, p, PC_KNIGHT);
		*list++ = Move(from, p, PC_BISHOP);
		*list++ = Move(from, p, PC_ROOK);
		*list++ = Move(from, p, PC_QUEEN);
	}
	return list;
}

constexpr Position::Position() : piece_bb{ 0 }, color(WHITE), historyIndex(0), board{}, hash(0), pinned(0), checkers(0), move50(0) {
	//Sets all squares on the board as empty
	for (int i = 0; i < 64; i++)
		board[i] = NO_PIECE;
	history[0] = UndoInfo();
}

void Position::Clear() {
	color = WHITE;
	historyIndex = 0;
	hash = 0;
	pinned = 0;
	checkers = 0;
	for (int n = 0; n < SQUARE_NB; n++)
		board[n] = NO_PIECE;
	for (int n = 0; n < PIECE_NB; n++)
		piece_bb[n] = 0;
	history[0] = UndoInfo();
};

void Position::MoveList(Color color, Move* list, int& count, bool quiet) {
	Move* last = GenerateMoves(color, list, quiet);
	count = last - list;
}

void Position::MoveList(Move* list, int& count, bool quiet) {
	Move* last = GenerateMoves(ColorUs(), list);
	count = last - list;
}

bool Position::IsLegal(Move move) {
	Move list[218];
	GenerateMoves(color, list);
	for (Move m : list)
		if (m == move)
			return true;
	return false;
}

/*bool Position::InCheck(Color c) const {
	return AttackersFrom(~c, bsf(bitboard_of(c, KING)), AllPieces());
}*/

bool Position::InCheck() const {
	return AttackersFrom(~color, bsf(bitboard_of(color, KING)), AllPieces());
}

inline void Position::PutPiece(Piece pc, Square s) {
	board[s] = pc;
	piece_bb[pc] |= SQUARE_BB[s];
	hash ^= zobrist::zobrist_table[pc][s];
}

/*void Position::Phase() {
	phase= PopCount(piece_bb[WHITE_KNIGHT] | piece_bb[BLACK_KNIGHT] | piece_bb[WHITE_BISHOP] | piece_bb[BLACK_BISHOP]) +
		PopCount(piece_bb[WHITE_ROOK] | piece_bb[BLACK_ROOK]) * 2 +
		SparsePopCount(piece_bb[WHITE_QUEEN] | piece_bb[BLACK_QUEEN]) * 4;
	if (phase > 24)
		phase = 24;
}*/

inline void Position::RemovePiece(Square s) {
	Piece pc = board[s];
	hash ^= zobrist::zobrist_table[pc][s];
	piece_bb[board[s]] &= ~SQUARE_BB[s];
	board[s] = NO_PIECE;
}

bool Position::IsRepetition() const {
	for (int n = historyIndex - 2; n >= historyIndex - move50; n -= 2)
		if (n >= 0)
			if (history[n].hash == hash)
				return true;
	return false;
}

void Position::MakeNull() {
	hash ^= zobrist::hashColor;
	color = ~color;
	++historyIndex;
	history[historyIndex] = UndoInfo(history[historyIndex - 1]);
	history[historyIndex].epsq = SQ_NONE;
}

void Position::UnmakeNull() {
	hash ^= zobrist::hashColor;
	color = ~color;
	--historyIndex;
}

//Plays a move in the position
void Position::MakeMove(const Move m) {
	history[historyIndex].move50 = move50;
	history[historyIndex].hash = hash;
	hash ^= zobrist::hashColor;
	Color c = color;
	color = ~color;
	++historyIndex;
	history[historyIndex] = UndoInfo(history[historyIndex - 1]);
	Square fr = m.From();
	Square to = m.To();
	MoveFlags type = m.Flags();
	history[historyIndex].entry |= SQUARE_BB[to] | SQUARE_BB[fr];
	move50 = (type == QUIET) && (TypeOf(board[fr]) != PAWN) ? ++move50 : 0;
	switch (type) {
	case QUIET:
		//The to square is guaranteed to be empty here
		MovePieceQuiet(m.From(), m.To());
		break;
	case DOUBLE_PUSH:
		//The to square is guaranteed to be empty here
		MovePieceQuiet(m.From(), m.To());

		//This is the square behind the pawn that was double-pushed
		history[historyIndex].epsq = m.From() + RelativeDir(c, NORTH);
		break;
	case OO:
		if (c == WHITE) {
			MovePieceQuiet(SQ_E1, SQ_G1);
			MovePieceQuiet(SQ_H1, SQ_F1);
		}
		else {
			MovePieceQuiet(SQ_E8, SQ_G8);
			MovePieceQuiet(SQ_H8, SQ_F8);
		}
		break;
	case OOO:
		if (c == WHITE) {
			MovePieceQuiet(SQ_E1, SQ_C1);
			MovePieceQuiet(SQ_A1, SQ_D1);
		}
		else {
			MovePieceQuiet(SQ_E8, SQ_C8);
			MovePieceQuiet(SQ_A8, SQ_D8);
		}
		break;
	case EN_PASSANT:
		MovePieceQuiet(m.From(), m.To());
		RemovePiece(m.To() + RelativeDir(c, SOUTH));
		break;
	case PR_KNIGHT:
		RemovePiece(m.From());
		PutPiece(MakePiece(c, KNIGHT), m.To());
		break;
	case PR_BISHOP:
		RemovePiece(m.From());
		PutPiece(MakePiece(c, BISHOP), m.To());
		break;
	case PR_ROOK:
		move50 = 0;
		RemovePiece(m.From());
		PutPiece(MakePiece(c, ROOK), m.To());
		break;
	case PR_QUEEN:
		RemovePiece(m.From());
		PutPiece(MakePiece(c, QUEEN), m.To());
		break;
	case PC_KNIGHT:
		RemovePiece(m.From());
		history[historyIndex].captured = board[m.To()];
		RemovePiece(m.To());
		PutPiece(MakePiece(c, KNIGHT), m.To());
		break;
	case PC_BISHOP:
		RemovePiece(m.From());
		history[historyIndex].captured = board[m.To()];
		RemovePiece(m.To());
		PutPiece(MakePiece(c, BISHOP), m.To());
		break;
	case PC_ROOK:
		RemovePiece(m.From());
		history[historyIndex].captured = board[m.To()];
		RemovePiece(m.To());
		PutPiece(MakePiece(c, ROOK), m.To());
		break;
	case PC_QUEEN:
		RemovePiece(m.From());
		history[historyIndex].captured = board[m.To()];
		RemovePiece(m.To());
		PutPiece(MakePiece(c, QUEEN), m.To());
		break;
	case CAPTURE:
		history[historyIndex].captured = board[m.To()];
		MovePiece(m.From(), m.To());
		break;
	}
}

//Undos a move in the current position, rolling it back to the previous position
void Position::UnmakeMove(const Move m) {
	//hash ^= zobrist::hashColor;
	color = ~color;
	Color c = color;
	MoveFlags type = m.Flags();
	switch (type) {
	case QUIET:
		MovePieceQuiet(m.To(), m.From());
		break;
	case DOUBLE_PUSH:
		MovePieceQuiet(m.To(), m.From());
		break;
	case OO:
		if (c == WHITE) {
			MovePieceQuiet(SQ_G1, SQ_E1);
			MovePieceQuiet(SQ_F1, SQ_H1);
		}
		else {
			MovePieceQuiet(SQ_G8, SQ_E8);
			MovePieceQuiet(SQ_F8, SQ_H8);
		}
		break;
	case OOO:
		if (c == WHITE) {
			MovePieceQuiet(SQ_C1, SQ_E1);
			MovePieceQuiet(SQ_D1, SQ_A1);
		}
		else {
			MovePieceQuiet(SQ_C8, SQ_E8);
			MovePieceQuiet(SQ_D8, SQ_A8);
		}
		break;
	case EN_PASSANT:
		MovePieceQuiet(m.To(), m.From());
		PutPiece(MakePiece(~c, PAWN), m.To() + RelativeDir(c, SOUTH));
		break;
	case PR_KNIGHT:
	case PR_BISHOP:
	case PR_ROOK:
	case PR_QUEEN:
		RemovePiece(m.To());
		PutPiece(MakePiece(c, PAWN), m.From());
		break;
	case PC_KNIGHT:
	case PC_BISHOP:
	case PC_ROOK:
	case PC_QUEEN:
		RemovePiece(m.To());
		PutPiece(MakePiece(c, PAWN), m.From());
		PutPiece(history[historyIndex].captured, m.To());
		break;
	case CAPTURE:
		MovePieceQuiet(m.To(), m.From());
		PutPiece(history[historyIndex].captured, m.To());
		break;
	}
	--historyIndex;
	move50 = history[historyIndex].move50;
	hash = history[historyIndex].hash;
}

void Position::SetFen(const std::string& fen) {
	Clear();
	int square = SQ_A8;
	for (char ch : fen.substr(0, fen.find(' '))) {
		if (isdigit(ch))
			square += (ch - '0') * EAST;
		else if (ch == '/')
			square += 2 * SOUTH;
		else
			PutPiece(Piece(PIECE_STR.find(ch)), Square(square++));
	}

	std::istringstream ss(fen.substr(fen.find(' ')));
	unsigned char token;

	ss >> token;
	color = token == 'w' ? WHITE : BLACK;

	history[historyIndex].entry = ALL_CASTLING_MASK;
	while (ss >> token && !isspace(token)) {
		switch (token) {
		case 'K':
			history[historyIndex].entry &= ~WHITE_OO_MASK;
			break;
		case 'Q':
			history[historyIndex].entry &= ~WHITE_OOO_MASK;
			break;
		case 'k':
			history[historyIndex].entry &= ~BLACK_OO_MASK;
			break;
		case 'q':
			history[historyIndex].entry &= ~BLACK_OOO_MASK;
			break;
		}
	}
	//cout << "phase= " << phase << endl;
}

//Returns the FEN (Forsyth-Edwards Notation) representation of the position
std::string Position::GetFen() const {
	std::ostringstream fen;
	int empty;

	for (int i = 56; i >= 0; i -= 8) {
		empty = 0;
		for (int j = 0; j < 8; j++) {
			Piece p = board[i + j];
			if (p == NO_PIECE) empty++;
			else {
				fen << (empty == 0 ? "" : std::to_string(empty))
					<< PIECE_STR[p];
				empty = 0;
			}
		}

		if (empty != 0) fen << empty;
		if (i > 0) fen << '/';
	}

	fen << (color == WHITE ? " w " : " b ")
		<< (history[historyIndex].entry & WHITE_OO_MASK ? "" : "K")
		<< (history[historyIndex].entry & WHITE_OOO_MASK ? "" : "Q")
		<< (history[historyIndex].entry & BLACK_OO_MASK ? "" : "k")
		<< (history[historyIndex].entry & BLACK_OOO_MASK ? "" : "q")
		<< (history[historyIndex].entry & ALL_CASTLING_MASK ? "- " : "")
		<< (history[historyIndex].epsq == SQ_NONE ? " -" : SQSTR[history[historyIndex].epsq]);

	return fen.str();
}

//Moves a piece to a (possibly empty) square on the board and updates the hash
void Position::MovePiece(Square from, Square to) {
	hash ^= zobrist::zobrist_table[board[from]][from] ^ zobrist::zobrist_table[board[from]][to] ^ zobrist::zobrist_table[board[to]][to];
	Bitboard mask = SQUARE_BB[from] | SQUARE_BB[to];
	piece_bb[board[from]] ^= mask;
	piece_bb[board[to]] &= ~mask;
	board[to] = board[from];
	board[from] = NO_PIECE;
}

//Moves a piece to an empty square. Note that it is an error if the <to> square contains a piece
void Position::MovePieceQuiet(Square from, Square to) {
	hash ^= zobrist::zobrist_table[board[from]][from] ^ zobrist::zobrist_table[board[from]][to];
	piece_bb[board[from]] ^= (SQUARE_BB[from] | SQUARE_BB[to]);
	board[to] = board[from];
	board[from] = NO_PIECE;
}

Move* Position::GenerateMoves(Color Us, Move* list, bool quiet) {
	Color Them = ~Us;

	const Bitboard us_bb = AllPieces(Us);
	const Bitboard them_bb = AllPieces(Them);
	const Bitboard all = us_bb | them_bb;

	const Square our_king = bsf(bitboard_of(Us, KING));
	const Square their_king = bsf(bitboard_of(Them, KING));

	const Bitboard our_diag_sliders = DiagonalSliders(Us);
	const Bitboard their_diag_sliders = DiagonalSliders(Them);
	const Bitboard our_orth_sliders = OrthogonalSliders(Us);
	const Bitboard their_orth_sliders = OrthogonalSliders(Them);

	//General purpose bitboards for attacks, masks, etc.
	Bitboard b1, b2, b3;

	//Squares that our king cannot move to
	Bitboard danger = 0;

	//For each enemy piece, add all of its attacks to the danger bitboard
	danger |= PawnAttacks(Them, bitboard_of(Them, PAWN)) | attacks<KING>(their_king, all);

	b1 = bitboard_of(Them, KNIGHT);
	while (b1) danger |= attacks<KNIGHT>(pop_lsb(&b1), all);

	b1 = their_diag_sliders;
	//all ^ SQUARE_BB[our_king] is written to prevent the king from moving to squares which are 'x-rayed'
	//by enemy bishops and queens
	while (b1) danger |= attacks<BISHOP>(pop_lsb(&b1), all ^ SQUARE_BB[our_king]);

	b1 = their_orth_sliders;
	//all ^ SQUARE_BB[our_king] is written to prevent the king from moving to squares which are 'x-rayed'
	//by enemy rooks and queens
	while (b1) danger |= attacks<ROOK>(pop_lsb(&b1), all ^ SQUARE_BB[our_king]);

	//The king can move to all of its surrounding squares, except ones that are attacked, and
	//ones that have our own pieces on them
	b1 = attacks<KING>(our_king, all) & ~(us_bb | danger);
	if (quiet)
		list = make<QUIET>(our_king, b1 & ~them_bb, list);
	list = make<CAPTURE>(our_king, b1 & them_bb, list);

	//The capture mask filters destination squares to those that contain an enemy piece that is checking the 
	//king and must be captured
	Bitboard capture_mask;

	//The quiet mask filter destination squares to those where pieces must be moved to block an incoming attack 
	//to the king
	Bitboard quiet_mask;

	//A general purpose square for storing destinations, etc.
	Square s;

	//Checkers of each piece type are identified by:
	//1. Projecting attacks FROM the king square
	//2. Intersecting this bitboard with the enemy bitboard of that piece type
	checkers = attacks<KNIGHT>(our_king, all) & bitboard_of(Them, KNIGHT)
		| PawnAttacks(Us, our_king) & bitboard_of(Them, PAWN);

	//Here, we identify slider checkers and pinners simultaneously, and candidates for such pinners 
	//and checkers are represented by the bitboard <candidates>
	Bitboard candidates = attacks<ROOK>(our_king, them_bb) & their_orth_sliders
		| attacks<BISHOP>(our_king, them_bb) & their_diag_sliders;

	pinned = 0;
	while (candidates) {
		s = pop_lsb(&candidates);
		b1 = SQUARES_BETWEEN_BB[our_king][s] & us_bb;

		//Do the squares in between the enemy slider and our king contain any of our pieces?
		//If not, add the slider to the checker bitboard
		if (b1 == 0) checkers ^= SQUARE_BB[s];
		//If there is only one of our pieces between them, add our piece to the pinned bitboard 
		else if ((b1 & b1 - 1) == 0) pinned ^= b1;
	}

	//This makes it easier to mask pieces
	const Bitboard not_pinned = ~pinned;

	switch (SparsePopCount(checkers)) {
	case 2:
		//If there is a double check, the only legal moves are king moves out of check
		return list;
	case 1: {
		//It's a single check!

		Square checker_square = bsf(checkers);
		Piece piece = board[checker_square];
		PieceType pt = TypeOf(piece);
		Color pc = ColorOf(piece);
		if ((pc == Them) && (pt == PAWN)) {
			//If the checker is a pawn, we must check for e.p. moves that can capture it
			//This evaluates to true if the checking piece is the one which just double pushed
			if (checkers == Shift(RelativeDir(Us, SOUTH), SQUARE_BB[history[historyIndex].epsq])) {
				//b1 contains our pawns that can capture the checker e.p.
				b1 = PawnAttacks(Them, history[historyIndex].epsq) & bitboard_of(Us, PAWN) & not_pinned;
				while (b1) *list++ = Move(pop_lsb(&b1), history[historyIndex].epsq, EN_PASSANT);
			}
		}
		if ((pc == Them) && (pt == KNIGHT)) {
			//If the checker is either a pawn or a knight, the only legal moves are to capture
		//the checker. Only non-pinned pieces can capture it
			b1 = AttackersFrom(Us, checker_square, all) & not_pinned;
			while (b1) {
				Square sf = pop_lsb(&b1);
				if (TypeOf(board[sf]) == PAWN && RelativeRank(Us, RankOf(sf)) == RANK_7) {
					*list++ = Move(sf, checker_square, PC_KNIGHT);
					*list++ = Move(sf, checker_square, PC_BISHOP);
					*list++ = Move(sf, checker_square, PC_ROOK);
					*list++ = Move(sf, checker_square, PC_QUEEN);
				}
				else
					*list++ = Move(sf, checker_square, CAPTURE);
			}

			return list;
		}
		//We must capture the checking piece
		capture_mask = checkers;

		//...or we can block it since it is guaranteed to be a slider
		quiet_mask = SQUARES_BETWEEN_BB[our_king][checker_square];

		/**Square checker_square = bsf(checkers);

		switch (board[checker_square]) {
		case make_piece(Them, PAWN):
			//If the checker is a pawn, we must check for e.p. moves that can capture it
			//This evaluates to true if the checking piece is the one which just double pushed
			if (checkers == Shift(RelativeDir(Us, SOUTH),SQUARE_BB[history[game_ply].epsq])) {
				//b1 contains our pawns that can capture the checker e.p.
				b1 = PawnAttacks(Them, history[game_ply].epsq) & bitboard_of(Us, PAWN) & not_pinned;
				while (b1) *list++ = Move(pop_lsb(&b1), history[game_ply].epsq, EN_PASSANT);
			}
			//FALL THROUGH INTENTIONAL
		case make_piece(Them, KNIGHT):
			//If the checker is either a pawn or a knight, the only legal moves are to capture
			//the checker. Only non-pinned pieces can capture it
			b1 = AttackersFrom(Us, checker_square, all) & not_pinned;
			while (b1) *list++ = Move(pop_lsb(&b1), checker_square, CAPTURE);

			return list;
		default:
			//We must capture the checking piece
			capture_mask = checkers;

			//...or we can block it since it is guaranteed to be a slider
			quiet_mask = SQUARES_BETWEEN_BB[our_king][checker_square];
			break;
		}*/

		break;
	}

	default:
		//We can capture any enemy piece
		capture_mask = them_bb;

		//...and we can play a quiet move to any square which is not occupied
		quiet_mask = ~all;

		if (history[historyIndex].epsq != SQ_NONE) {
			//b1 contains our pawns that can perform an e.p. capture
			b2 = PawnAttacks(Them, history[historyIndex].epsq) & bitboard_of(Us, PAWN);
			b1 = b2 & not_pinned;
			while (b1) {
				s = pop_lsb(&b1);

				//This piece of evil bit-fiddling magic prevents the infamous 'pseudo-pinned' e.p. case,
				//where the pawn is not directly pinned, but on moving the pawn and capturing the enemy pawn
				//e.p., a rook or queen attack to the king is revealed

				/*
				.nbqkbnr
				ppp.pppp
				........
				r..pP..K
				........
				........
				PPPP.PPP
				RNBQ.BNR

				Here, if white plays exd5 e.p., the black rook on a5 attacks the white king on h5
				*/

				if ((sliding_attacks(our_king, all ^ SQUARE_BB[s]
					^ Shift(RelativeDir(Us, SOUTH), SQUARE_BB[history[historyIndex].epsq]),
						MASK_RANK[RankOf(our_king)]) &
					their_orth_sliders) == 0)
					*list++ = Move(s, history[historyIndex].epsq, EN_PASSANT);
			}

			//Pinned pawns can only capture e.p. if they are pinned diagonally and the e.p. square is in line with the king 
			b1 = b2 & pinned & LINE[history[historyIndex].epsq][our_king];
			if (b1) {
				*list++ = Move(bsf(b1), history[historyIndex].epsq, EN_PASSANT);
			}
		}

		if (quiet)
		{
			//Only add castling if:
			//1. The king and the rook have both not moved
			//2. No piece is attacking between the the rook and the king
			//3. The king is not in check
			if (!((history[historyIndex].entry & oo_mask(Us)) | ((all | danger) & oo_blockers_mask(Us)))) {
				*list++ = Us == WHITE ? Move(SQ_E1, SQ_G1, OO) : Move(SQ_E8, SQ_G8, OO);
				//Move m = WHITE ? Move(e1, h1, OO) : Move(e8, h8, OO);
				//std::cout << m.ToUci() << endl;
			}
			if (!((history[historyIndex].entry & ooo_mask(Us)) |
				((all | (danger & ~ignore_ooo_danger(Us))) & ooo_blockers_mask(Us))))
				*list++ = Us == WHITE ? Move(SQ_E1, SQ_C1, OOO) : Move(SQ_E8, SQ_C8, OOO);
		}
		//For each pinned rook, bishop or queen...
		b1 = ~(not_pinned | bitboard_of(Us, KNIGHT));
		while (b1) {
			s = pop_lsb(&b1);

			//...only include attacks that are aligned with our king, since pinned pieces
			//are constrained to move in this direction only
			b2 = attacks(TypeOf(board[s]), s, all) & LINE[our_king][s];
			if (quiet)
				list = make<QUIET>(s, b2 & quiet_mask, list);
			list = make<CAPTURE>(s, b2 & capture_mask, list);
		}

		//For each pinned pawn...
		b1 = ~not_pinned & bitboard_of(Us, PAWN);
		while (b1) {
			s = pop_lsb(&b1);

			if (RankOf(s) == RelativeRank(Us, RANK_7)) {
				//Quiet promotions are impossible since the square in front of the pawn will
				//either be occupied by the king or the pinner, or doing so would leave our king
				//in check
				b2 = PawnAttacks(Us, s) & capture_mask & LINE[our_king][s];
				list = make<PROMOTION_CAPTURES>(s, b2, list);
			}
			else {
				b2 = PawnAttacks(Us, s) & them_bb & LINE[s][our_king];
				list = make<CAPTURE>(s, b2, list);
				if (quiet) {
					//Single pawn pushes
					b2 = Shift(RelativeDir(Us, NORTH), SQUARE_BB[s]) & ~all & LINE[our_king][s];
					//Double pawn pushes (only pawns on rank 3/6 are eligible)
					b3 = Shift(RelativeDir(Us, NORTH), b2 &
						MASK_RANK[RelativeRank(Us, RANK_3)]) & ~all & LINE[our_king][s];
					list = make<QUIET>(s, b2, list);
					list = make<DOUBLE_PUSH>(s, b3, list);
				}
			}
		}

		//Pinned knights cannot move anywhere, so we're done with pinned pieces!

		break;
	}

	//Non-pinned knight moves
	b1 = bitboard_of(Us, KNIGHT) & not_pinned;
	while (b1) {
		s = pop_lsb(&b1);
		b2 = attacks<KNIGHT>(s, all);
		if (quiet)
			list = make<QUIET>(s, b2 & quiet_mask, list);
		list = make<CAPTURE>(s, b2 & capture_mask, list);
	}

	//Non-pinned bishops and queens
	b1 = our_diag_sliders & not_pinned;
	while (b1) {
		s = pop_lsb(&b1);
		b2 = attacks<BISHOP>(s, all);
		if (quiet)
			list = make<QUIET>(s, b2 & quiet_mask, list);
		list = make<CAPTURE>(s, b2 & capture_mask, list);
	}

	//Non-pinned rooks and queens
	b1 = our_orth_sliders & not_pinned;
	while (b1) {
		s = pop_lsb(&b1);
		b2 = attacks<ROOK>(s, all);
		if (quiet)
			list = make<QUIET>(s, b2 & quiet_mask, list);
		list = make<CAPTURE>(s, b2 & capture_mask, list);
	}

	//b1 contains non-pinned pawns which are not on the last rank
	b1 = bitboard_of(Us, PAWN) & not_pinned & ~MASK_RANK[RelativeRank(Us, RANK_7)];

	if (quiet)
	{
		//Single pawn pushes
		b2 = Shift(RelativeDir(Us, NORTH), b1) & ~all;

		//Double pawn pushes (only pawns on rank 3/6 are eligible)
		b3 = Shift(RelativeDir(Us, NORTH), b2 & MASK_RANK[RelativeRank(Us, RANK_3)]) & quiet_mask;

		//We & this with the quiet mask only later, as a non-check-blocking single push does NOT mean that the 
		//corresponding double push is not blocking check either.
		b2 &= quiet_mask;

		while (b2) {
			s = pop_lsb(&b2);
			*list++ = Move(s - RelativeDir(Us, NORTH), s, QUIET);
		}

		while (b3) {
			s = pop_lsb(&b3);
			*list++ = Move(s - RelativeDir(Us, NORTH_NORTH), s, DOUBLE_PUSH);
		}
	}
	//Pawn captures
	b2 = Shift(RelativeDir(Us, NORTH_WEST), b1) & capture_mask;
	b3 = Shift(RelativeDir(Us, NORTH_EAST), b1) & capture_mask;

	while (b2) {
		s = pop_lsb(&b2);
		*list++ = Move(s - RelativeDir(Us, NORTH_WEST), s, CAPTURE);
	}

	while (b3) {
		s = pop_lsb(&b3);
		*list++ = Move(s - RelativeDir(Us, NORTH_EAST), s, CAPTURE);
	}

	//b1 now contains non-pinned pawns which ARE on the last rank (about to promote)
	b1 = bitboard_of(Us, PAWN) & not_pinned & MASK_RANK[RelativeRank(Us, RANK_7)];
	if (b1) {
		//Quiet promotions
		//if (quiet) {
		b2 = Shift(RelativeDir(Us, NORTH), b1) & quiet_mask;
		while (b2) {
			s = pop_lsb(&b2);
			//One move is added for each promotion piece
			*list++ = Move(s - RelativeDir(Us, NORTH), s, PR_KNIGHT);
			*list++ = Move(s - RelativeDir(Us, NORTH), s, PR_BISHOP);
			*list++ = Move(s - RelativeDir(Us, NORTH), s, PR_ROOK);
			*list++ = Move(s - RelativeDir(Us, NORTH), s, PR_QUEEN);
			//}
		}
		//Promotion captures
		b2 = Shift(RelativeDir(Us, NORTH_WEST), b1) & capture_mask;
		b3 = Shift(RelativeDir(Us, NORTH_EAST), b1) & capture_mask;

		while (b2) {
			s = pop_lsb(&b2);
			//One move is added for each promotion piece
			*list++ = Move(s - RelativeDir(Us, NORTH_WEST), s, PC_KNIGHT);
			*list++ = Move(s - RelativeDir(Us, NORTH_WEST), s, PC_BISHOP);
			*list++ = Move(s - RelativeDir(Us, NORTH_WEST), s, PC_ROOK);
			*list++ = Move(s - RelativeDir(Us, NORTH_WEST), s, PC_QUEEN);
		}

		while (b3) {
			s = pop_lsb(&b3);
			//One move is added for each promotion piece
			*list++ = Move(s - RelativeDir(Us, NORTH_EAST), s, PC_KNIGHT);
			*list++ = Move(s - RelativeDir(Us, NORTH_EAST), s, PC_BISHOP);
			*list++ = Move(s - RelativeDir(Us, NORTH_EAST), s, PC_ROOK);
			*list++ = Move(s - RelativeDir(Us, NORTH_EAST), s, PC_QUEEN);
		}
	}

	return list;
}

//Returns a bitboard containing all pieces attacking a particluar square
inline Bitboard Position::Attackers(Square s) const {
	Bitboard occ = AllPieces();
	return color == BLACK ?
		(PawnAttacks(BLACK, s) & piece_bb[WHITE_PAWN]) |
		(attacks<KNIGHT>(s, occ) & piece_bb[WHITE_KNIGHT]) |
		(attacks<BISHOP>(s, occ) & (piece_bb[WHITE_BISHOP] | piece_bb[WHITE_QUEEN])) |
		(attacks<ROOK>(s, occ) & (piece_bb[WHITE_ROOK] | piece_bb[WHITE_QUEEN])) :
		(PawnAttacks(WHITE, s) & piece_bb[BLACK_PAWN]) |
		(attacks<KNIGHT>(s, occ) & piece_bb[BLACK_KNIGHT]) |
		(attacks<BISHOP>(s, occ) & (piece_bb[BLACK_BISHOP] | piece_bb[BLACK_QUEEN])) |
		(attacks<ROOK>(s, occ) & (piece_bb[BLACK_ROOK] | piece_bb[BLACK_QUEEN]));
}

//Returns a bitboard containing all pieces of a given color attacking a particluar square
inline Bitboard Position::AttackersFrom(Color c, Square s, Bitboard occ) const {
	return c == WHITE ?
		(PawnAttacks(BLACK, s) & piece_bb[WHITE_PAWN]) |
		(attacks<KNIGHT>(s, occ) & piece_bb[WHITE_KNIGHT]) |
		(attacks<BISHOP>(s, occ) & (piece_bb[WHITE_BISHOP] | piece_bb[WHITE_QUEEN])) |
		(attacks<ROOK>(s, occ) & (piece_bb[WHITE_ROOK] | piece_bb[WHITE_QUEEN])) :
		(PawnAttacks(WHITE, s) & piece_bb[BLACK_PAWN]) |
		(attacks<KNIGHT>(s, occ) & piece_bb[BLACK_KNIGHT]) |
		(attacks<BISHOP>(s, occ) & (piece_bb[BLACK_BISHOP] | piece_bb[BLACK_QUEEN])) |
		(attacks<ROOK>(s, occ) & (piece_bb[BLACK_ROOK] | piece_bb[BLACK_QUEEN]));
}

//Returns the bitboard of all bishops and queens of a given color
inline Bitboard Position::DiagonalSliders(Color c) const {
	return c == WHITE ? piece_bb[WHITE_BISHOP] | piece_bb[WHITE_QUEEN] :
		piece_bb[BLACK_BISHOP] | piece_bb[BLACK_QUEEN];
}

//Returns the bitboard of all rooks and queens of a given color
inline Bitboard Position::OrthogonalSliders(Color c) const {
	return c == WHITE ? piece_bb[WHITE_ROOK] | piece_bb[WHITE_QUEEN] :
		piece_bb[BLACK_ROOK] | piece_bb[BLACK_QUEEN];
}

/*void Position::Phase() {
	phase= PopCount(piece_bb[WHITE_KNIGHT] | piece_bb[BLACK_KNIGHT] | piece_bb[WHITE_BISHOP] | piece_bb[BLACK_BISHOP]) +
		PopCount(piece_bb[WHITE_ROOK] | piece_bb[BLACK_ROOK]) * 2 +
		SparsePopCount(piece_bb[WHITE_QUEEN] | piece_bb[BLACK_QUEEN]) * 4;
	if (phase > 24)
		phase = 24;
}*/


/*template<Color C>
Bitboard Position::pinned(Square s, Bitboard us, Bitboard occ) const {
	Bitboard pinned = 0;

	Bitboard pinners = get_xray_rook_attacks(s, occ, us) & orthogonal_sliders<~C>();
	while (pinners) pinned |= SQUARES_BETWEEN_BB[s][pop_lsb(&pinners)] & us;

	pinners = get_xray_bishop_attacks(s, occ, us) & diagonal_sliders<~C>();
	while (pinners) pinned |= SQUARES_BETWEEN_BB[s][pop_lsb(&pinners)] & us;

	return pinned;
}

template<Color C>
Bitboard Position::blockers_to(Square s, Bitboard occ) const {
	Bitboard blockers = 0;
	Bitboard candidates = get_rook_attacks(s, occ) & occ;
	Bitboard attackers = get_rook_attacks(s, occ ^ candidates) & orthogonal_sliders<~C>();

	candidates = get_bishop_attacks(s, occ) & occ;
	attackers |= get_bishop_attacks(s, occ ^ candidates) & diagonal_sliders<~C>();

	while (attackers) blockers |= SQUARES_BETWEEN_BB[s][pop_lsb(&attackers)];
	return blockers;
}*/

Move::Move(const std::string& move) {
	Square fr = CreateSquare(File(move[0] - 'a'), Rank(move[1] - '1'));
	Square to = CreateSquare(File(move[2] - 'a'), Rank(move[3] - '1'));
	MoveFlags mf = MoveFlags::QUIET;
	if (move.length() > 4)
		if (move[4] == 'q')
			mf = MoveFlags::PC_QUEEN;
		else if (move[4] == 'r')
			mf = MoveFlags::PC_ROOK;
		else if (move[4] == 'b')
			mf = MoveFlags::PC_BISHOP;
		else if (move[4] == 'n')
			mf = MoveFlags::PC_KNIGHT;
	this->move = (mf << 12) | (fr << 6) | to;
}

string Move::ToUci() const {
	string uci = SQSTR[From()] + SQSTR[To()];
	if (Flags() & PROMOTION)
		return uci + MOVE_TYPESTR_UCI[Flags() & 7];
	return uci;
}

//Prints the move
std::ostream& operator<<(std::ostream& os, Move& m) {
	os << m.ToUci();
	return os;
}

