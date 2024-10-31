#pragma once

#include "types.h"

extern const Bitboard KING_ATTACKS[SQUARE_NB];
extern const Bitboard KNIGHT_ATTACKS[SQUARE_NB];
extern const Bitboard WHITE_PAWN_ATTACKS[SQUARE_NB];
extern const Bitboard BLACK_PAWN_ATTACKS[SQUARE_NB];

extern Bitboard reverse(Bitboard b);
extern Bitboard sliding_attacks(Square square, Bitboard occ, Bitboard mask);

extern Bitboard get_rook_attacks_for_init(Square square, Bitboard occ);
extern const Bitboard ROOK_MAGICS[SQUARE_NB];
extern Bitboard ROOK_ATTACK_MASKS[SQUARE_NB];
extern int ROOK_ATTACK_SHIFTS[SQUARE_NB];
extern Bitboard ROOK_ATTACKS[SQUARE_NB][4096];
extern void initialise_rook_attacks();


extern constexpr Bitboard GetRookAttacks(Square square, Bitboard occ);
extern Bitboard get_xray_rook_attacks(Square square, Bitboard occ, Bitboard blockers);

extern Bitboard get_bishop_attacks_for_init(Square square, Bitboard occ);
extern const Bitboard BISHOP_MAGICS[SQUARE_NB];
extern Bitboard BISHOP_ATTACK_MASKS[SQUARE_NB];
extern int BISHOP_ATTACK_SHIFTS[SQUARE_NB];
extern Bitboard BISHOP_ATTACKS[SQUARE_NB][512];
extern void initialise_bishop_attacks();


extern constexpr Bitboard GetBishopAttacks(Square square, Bitboard occ);
extern Bitboard get_xray_bishop_attacks(Square square, Bitboard occ, Bitboard blockers);

extern Bitboard SQUARES_BETWEEN_BB[SQUARE_NB][SQUARE_NB];
extern Bitboard LINE[SQUARE_NB][SQUARE_NB];
extern Bitboard PAWN_ATTACKS[COLOR_NB][SQUARE_NB];
extern Bitboard PSEUDO_LEGAL_ATTACKS[PT_NB][SQUARE_NB];
extern Bitboard bbForwardFiles[COLOR_NB][SQUARE_NB];
extern Bitboard bbForwardRank[COLOR_NB][RANK_NB];
extern Bitboard bbPawnAttackSpan[COLOR_NB][SQUARE_NB];
extern Bitboard bbAdjacentFiles[FILE_NB];
extern Bitboard bbPassedPawnMask[COLOR_NB][SQUARE_NB];

extern void initialise_squares_between();
extern void initialise_line();
extern void initialise_pseudo_legal();
extern void initialise_all_databases();


//Returns a bitboard containing all squares that a piece on a square can move to, in the given position
template<PieceType P>
constexpr Bitboard attacks(Square s, Bitboard occ) {
	static_assert(P != PAWN, "The piece type may not be a pawn; use pawn_attacks instead");
	return P == ROOK ? GetRookAttacks(s, occ) :
		P == BISHOP ? GetBishopAttacks(s, occ) :
		P == QUEEN ? attacks<ROOK>(s, occ) | attacks<BISHOP>(s, occ) :
		PSEUDO_LEGAL_ATTACKS[P][s];
}

//Returns a bitboard containing all squares that a piece on a square can move to, in the given position
//Used when the piece type is not known at compile-time
constexpr Bitboard attacks(PieceType pt, Square s, Bitboard occ) {
	switch (pt) {
	case ROOK:
		return attacks<ROOK>(s, occ);
	case BISHOP:
		return attacks<BISHOP>(s, occ);
	case QUEEN:
		return attacks<QUEEN>(s, occ);
	default:
		return PSEUDO_LEGAL_ATTACKS[pt][s];
	}
}

//Returns a bitboard containing pawn attacks from all pawns in the given bitboard
constexpr Bitboard PawnAttacks(Color c,Bitboard p) {
	return c == WHITE ? Shift(NORTH_WEST,p) | Shift(NORTH_EAST,p) :
		Shift(SOUTH_WEST,p) | Shift(SOUTH_EAST,p);
}

//Returns a bitboard containing pawn attacks from the pawn on the given square
constexpr Bitboard PawnAttacks(Color c,Square s) {
	return PAWN_ATTACKS[c][s];
}

constexpr Bitboard BBFile(Square sq) {
	return MASK_FILE[FileOf(sq)];
}

constexpr Bitboard BBRank(Square sq) {
	return MASK_RANK[RankOf(sq)];
}

constexpr Bitboard BBColor(Bitboard bb) { 
	return bb & bbLight ? bbLight : bbDark;
};
