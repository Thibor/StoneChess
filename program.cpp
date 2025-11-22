#include "program.h"

SearchInfo info;
Options options;

//Lookup tables of square names in algebraic chess notation
const string SQSTR[65] = {
	"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
	"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
	"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
	"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
	"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
	"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
	"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
	"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
	"None"
};

const string MOVE_TYPESTR_UCI[8] = { "","","","","n", "b", "r", "q" };
const char* MOVE_TYPESTR[16] = { "", "", " O-O", " O-O-O", "N", "B", "R", "Q", " (capture)", "", " e.p.", "","N", "B", "R", "Q" };

//Precomputed file masks
const Bitboard MASK_FILE[8] = {
	0x101010101010101, 0x202020202020202, 0x404040404040404, 0x808080808080808,
	0x1010101010101010, 0x2020202020202020, 0x4040404040404040, 0x8080808080808080,
};

//Precomputed rank masks
const Bitboard MASK_RANK[8] = {
	0xff, 0xff00, 0xff0000, 0xff000000,
	0xff00000000, 0xff0000000000, 0xff000000000000, 0xff00000000000000
};

//Precomputed diagonal masks
const Bitboard MASK_DIAGONAL[15] = {
	0x80, 0x8040, 0x804020,
	0x80402010, 0x8040201008, 0x804020100804,
	0x80402010080402, 0x8040201008040201, 0x4020100804020100,
	0x2010080402010000, 0x1008040201000000, 0x804020100000000,
	0x402010000000000, 0x201000000000000, 0x100000000000000,
};

//Precomputed anti-diagonal masks
const Bitboard MASK_ANTI_DIAGONAL[15] = {
	0x1, 0x102, 0x10204,
	0x1020408, 0x102040810, 0x10204081020,
	0x1020408102040, 0x102040810204080, 0x204081020408000,
	0x408102040800000, 0x810204080000000, 0x1020408000000000,
	0x2040800000000000, 0x4080000000000000, 0x8000000000000000,
};

//Precomputed square masks
const Bitboard SQUARE_BB[65] = {
	0x1, 0x2, 0x4, 0x8,
	0x10, 0x20, 0x40, 0x80,
	0x100, 0x200, 0x400, 0x800,
	0x1000, 0x2000, 0x4000, 0x8000,
	0x10000, 0x20000, 0x40000, 0x80000,
	0x100000, 0x200000, 0x400000, 0x800000,
	0x1000000, 0x2000000, 0x4000000, 0x8000000,
	0x10000000, 0x20000000, 0x40000000, 0x80000000,
	0x100000000, 0x200000000, 0x400000000, 0x800000000,
	0x1000000000, 0x2000000000, 0x4000000000, 0x8000000000,
	0x10000000000, 0x20000000000, 0x40000000000, 0x80000000000,
	0x100000000000, 0x200000000000, 0x400000000000, 0x800000000000,
	0x1000000000000, 0x2000000000000, 0x4000000000000, 0x8000000000000,
	0x10000000000000, 0x20000000000000, 0x40000000000000, 0x80000000000000,
	0x100000000000000, 0x200000000000000, 0x400000000000000, 0x800000000000000,
	0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000,
	0x0
};

//Prints the bitboard
void PrintBitboard(Bitboard bb) {
	const char* s = "   +---+---+---+---+---+---+---+---+\n";
	const char* t = "     A   B   C   D   E   F   G   H\n";
	cout << t;
	for (int i = 56; i >= 0; i -= 8) {
		cout << s << " " << i / 8 + 1 << " ";
		for (int x = 0; x < 8; x++) {
			const char* c = SQUARE_BB[i + x] & bb ? "x" : " ";
			cout << "| " << c << " ";
		}
		cout << "| " << i / 8 + 1 << endl;
	}
	cout << s;
	cout << t << endl;
}

const Bitboard k1 = 0x5555555555555555;
const Bitboard k2 = 0x3333333333333333;
const Bitboard k4 = 0x0f0f0f0f0f0f0f0f;
const Bitboard kf = 0x0101010101010101;

//Returns number of set bits in the bitboard
inline int PopCount(Bitboard x) {
	x = x - ((x >> 1) & k1);
	x = (x & k2) + ((x >> 2) & k2);
	x = (x + (x >> 4)) & k4;
	x = (x * kf) >> 56;
	return int(x);
}

//Returns number of set bits in the bitboard. Faster than pop_count(x) when the bitboard has few set bits
inline int SparsePopCount(Bitboard x) {
	int count = 0;
	while (x) {
		count++;
		x &= x - 1;
	}
	return count;
}

const int DEBRUIJN64[64] = {
	0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

const Bitboard MAGIC = 0x03f79d71b4cb0a89;

//Returns the index of the least significant bit in the bitboard, and removes the bit from the bitboard
inline Square pop_lsb(Bitboard* b) {
	int lsb = bsf(*b);
	*b &= *b - 1;
	return Square(lsb);
}

//Returns the index of the least significant bit in the bitboard
constexpr Square bsf(Bitboard b) {
	return Square(DEBRUIJN64[MAGIC * (b ^ (b - 1)) >> 58]);
}

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
void InitPosition() {
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

std::string Trim(const std::string& s)
{
	if (s.empty())
		return s;
	auto start = s.begin();
	while (start != s.end() && std::isspace(*start)) {
		start++;
	}

	auto end = s.end();
	do {
		end--;
	} while (std::distance(start, end) > 0 && std::isspace(*end));

	return std::string(start, end + 1);
}

void SplitString(const std::string& txt, std::vector<std::string>& vStr, char ch) {
	vStr.clear();
	if (txt == "")
		return;
	size_t pos = txt.find(ch);
	size_t initialPos = 0;

	// Decompose statement
	while (pos != std::string::npos) {
		vStr.push_back(txt.substr(initialPos, pos - initialPos));
		initialPos = pos + 1;

		pos = txt.find(ch, initialPos);
	}

	// Add the last one
	vStr.push_back(txt.substr(initialPos, min(pos, txt.size()) - initialPos + 1));
}

void SplitInt(const string& txt, vector<int>& vInt, char ch) {
	vInt.clear();
	if (txt == "")
		return;
	size_t pos = txt.find(ch);
	size_t initialPos = 0;

	// Decompose statement
	while (pos != std::string::npos) {
		vInt.push_back(stoi(txt.substr(initialPos, pos - initialPos)));
		initialPos = pos + 1;

		pos = txt.find(ch, initialPos);
	}

	// Add the last one
	vInt.push_back(stoi(txt.substr(initialPos, min(pos, txt.size()) - initialPos + 1)));
}

// Function to put thousands
// separators in the given integer
string thousandSeparator(uint64_t n)
{
	string ans = "";

	// Convert the given integer
	// to equivalent string
	string num = to_string(n);

	// Initialise count
	int count = 0;

	// Traverse the string in reverse
	for (int i = (int)num.size() - 1; i >= 0; i--) {
		ans.push_back(num[i]);

		// If three characters
		// are traversed
		if (++count == 3) {
			ans.push_back(' ');
			count = 0;
		}
	}

	// Reverse the string to get
	// the desired output
	reverse(ans.begin(), ans.end());

	// If the given string is
	// less than 1000
	if (ans.size() % 4 == 0) {

		// Remove ','
		ans.erase(ans.begin());
	}

	return ans;
}

string StrToLower(string s) {
	transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

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

U64 GetTimeMs() {
#ifdef WIN32
	return GetTickCount64();
#else
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec * 1000 + t.tv_usec / 1000;
#endif
}

//check if we need to stop the search
bool CheckUp() {
	string input;
	if (GetInput(input))
		UciCommand(input);
	if ((info.timeLimit && GetTimeMs() - info.timeStart > info.timeLimit) || (info.nodesLimit && info.nodes > info.nodesLimit))
		info.stop = true;
	return info.stop;
}

int main() {
	cout << NAME << " " << VERSION << endl;
	InitImput();
	InitEval();
	InitSearch();
	InitPosition();
	zobrist::InitialiseZobristKeys();
	g_pos.SetFen();
	UciLoop();
}
