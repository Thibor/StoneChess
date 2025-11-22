#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <algorithm>
#include <vector>
#include <sstream> 
#include "windows.h" 

#define NAME "StoneChess"
#define VERSION "2025-09-30"


using namespace std;

#define U64 unsigned __int64
#define U32 unsigned __int32
#define U16 unsigned __int16
#define U8  unsigned __int8
#define S64 signed __int64
#define S32 signed __int32
#define S16 signed __int16
#define S8  signed __int8
#define Bitboard unsigned __int64
#define Hash unsigned __int64

constexpr int MAX_PLY = 100;
#define MOVE_NONE 0

enum Square : int {
	SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
	SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
	SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
	SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
	SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
	SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
	SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
	SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
	SQ_NONE,
	SQUARE_NB = 64
};

enum Piece : int {
	WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
	BLACK_PAWN = 8, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
	NO_PIECE,
	PIECE_NB = 15
};

enum PieceType : int { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PT_NB };

enum File : int { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NB };

enum Rank : int { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NB };

enum Direction : int {
	NORTH = 8, NORTH_EAST = 9, EAST = 1, SOUTH_EAST = -7,
	SOUTH = -8, SOUTH_WEST = -9, WEST = -1, NORTH_WEST = 7,
	NORTH_NORTH = 16, SOUTH_SOUTH = -16,
	NDIRS = 8
};

enum Color : int { WHITE, BLACK, COLOR_NB };

enum NodeType { PV, NONPV };

//The type of the move
enum MoveFlags : int {
	QUIET = 0b0000, DOUBLE_PUSH = 0b0001,
	OO = 0b0010, OOO = 0b0011,
	CAPTURE = 0b1000,
	EN_PASSANT = 0b1010,
	PROMOTION = 0b0100,
	PROMOTIONS = 0b0111,
	PROMOTION_CAPTURES = 0b1100,
	PR_KNIGHT = 0b0100, PR_BISHOP = 0b0101, PR_ROOK = 0b0110, PR_QUEEN = 0b0111,
	PC_KNIGHT = 0b1100, PC_BISHOP = 0b1101, PC_ROOK = 0b1110, PC_QUEEN = 0b1111
};

enum Value : int {
	VALUE_NONE = 0,
	VALUE_ZERO = 0,
	VALUE_KNOWN_WIN = 10000,
	VALUE_MATE = 32000,
	VALUE_INFINITE = 32001,
	VALUE_MATE_IN = VALUE_MATE - 2 * MAX_PLY,
	VALUE_MATED_IN = -VALUE_MATE + 2 * MAX_PLY
};

enum Score : int {
	SCORE_ZERO
};

enum Depth : int {
	DEPTH_ZERO = 0,
	ONE_PLY = 1,
	DEPTH_MAX = MAX_PLY
};

constexpr Score S(int mg, int eg) {
	return Score((int)((unsigned int)eg << 16) + mg);
}

constexpr Score D(int v, int d) {
	return S(v + d, v - d);
}

/// Extracting the signed lower and upper 16 bits is not so trivial because
/// according to the standard a simple cast to short is implementation defined
/// and so is a right shift of a signed integer.
inline Value Eg(Score s) {
	//return Value(s >> 16);
	union { uint16_t u; int16_t s; } eg = { uint16_t(unsigned(s + 0x8000) >> 16) }; return Value(eg.s);
}

inline Value Mg(Score s) {
	//return Value(s & 0xffff);
	union { uint16_t u; int16_t s; } mg = { uint16_t(unsigned(s)) }; return Value(mg.s);
}

inline Value VDV(Score s) {
	int mg = Mg(s);
	int eg = Eg(s);
	return Value((mg + eg) >> 2);
}

inline Value VDD(Score s) {
	int mg = Mg(s);
	int eg = Eg(s);
	return Value(abs(mg - eg) >> 2);
}

#define ENABLE_BASE_OPERATORS_ON(T)                                \
constexpr T operator+(T d1, T d2) { return T(int(d1) + int(d2)); } \
constexpr T operator-(T d1, T d2) { return T(int(d1) - int(d2)); } \
constexpr T operator-(T d) { return T(-int(d)); }                  \
inline T& operator+=(T& d1, T d2) { return d1 = d1 + d2; }         \
inline T& operator-=(T& d1, T d2) { return d1 = d1 - d2; }

#define ENABLE_INCR_OPERATORS_ON(T)                                \
inline T& operator++(T& d) { return d = T(int(d) + 1); }           \
inline T& operator--(T& d) { return d = T(int(d) - 1); }

#define ENABLE_FULL_OPERATORS_ON(T)                                \
ENABLE_BASE_OPERATORS_ON(T)                                        \
ENABLE_INCR_OPERATORS_ON(T)                                        \
constexpr T operator*(int i, T d) { return T(i * int(d)); }        \
constexpr T operator*(T d, int i) { return T(int(d) * i); }        \
constexpr T operator/(T d, int i) { return T(int(d) / i); }        \
constexpr int operator/(T d1, T d2) { return int(d1) / int(d2); }  \
inline T& operator*=(T& d, int i) { return d = T(int(d) * i); }    \
inline T& operator/=(T& d, int i) { return d = T(int(d) / i); }

ENABLE_FULL_OPERATORS_ON(Value)
ENABLE_FULL_OPERATORS_ON(Depth)
ENABLE_FULL_OPERATORS_ON(Direction)

ENABLE_INCR_OPERATORS_ON(PieceType)
ENABLE_INCR_OPERATORS_ON(Piece)
ENABLE_INCR_OPERATORS_ON(Color)
ENABLE_INCR_OPERATORS_ON(Square)
ENABLE_INCR_OPERATORS_ON(File)
ENABLE_INCR_OPERATORS_ON(Rank)

ENABLE_BASE_OPERATORS_ON(Score)

#undef ENABLE_FULL_OPERATORS_ON
#undef ENABLE_INCR_OPERATORS_ON
#undef ENABLE_BASE_OPERATORS_ON


/// Additional operators to add integers to a Value
constexpr Value operator+(Value v, int i) { return Value(int(v) + i); }
constexpr Value operator-(Value v, int i) { return Value(int(v) - i); }
inline Value& operator+=(Value& v, int i) { return v = v + i; }
inline Value& operator-=(Value& v, int i) { return v = v - i; }

/// Additional operators to add a Direction to a Square
constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
inline Square& operator+=(Square& s, Direction d) { return s = s + d; }
inline Square& operator-=(Square& s, Direction d) { return s = s - d; }

/// Only declared but not defined. We don't want to multiply two scores due to
/// a very high risk of overflow. So user should explicitly convert to integer.
Score operator*(Score, Score) = delete;

/// Division of a Score must be handled separately for each term
inline Score operator/(Score s, int i) {
	return S(Mg(s) / i, Eg(s) / i);
}

/// Multiplication of a Score by an integer. We check for overflow in debug mode.
inline Score operator*(Score s, int i) {
	return Score(int(s) * i);
}

constexpr Color operator~(Color c) {
	return Color(c ^ BLACK); // Toggle color
}

constexpr Square operator~(Square s) {
	return Square(s ^ SQ_A8); // Vertical flip SQ_A1 -> SQ_A8
}

constexpr File operator~(File f) {
	return File(f ^ FILE_H); // Horizontal flip FILE_A -> FILE_H
}

constexpr Piece operator~(Piece pc) {
	return Piece(pc ^ 8); // Swap color of piece B_KNIGHT -> W_KNIGHT
}


const Bitboard bbLight = 0xaa55aa55aa55aa55ull;
const Bitboard bbDark = 0x55aa55aa55aa55aaull;

constexpr Bitboard FileABB = 0x0101010101010101ULL;
constexpr Bitboard FileBBB = FileABB << 1;
constexpr Bitboard FileCBB = FileABB << 2;
constexpr Bitboard FileDBB = FileABB << 3;
constexpr Bitboard FileEBB = FileABB << 4;
constexpr Bitboard FileFBB = FileABB << 5;
constexpr Bitboard FileGBB = FileABB << 6;
constexpr Bitboard FileHBB = FileABB << 7;

constexpr Bitboard Rank1BB = 0xFF;
constexpr Bitboard Rank2BB = Rank1BB << (8 * 1);
constexpr Bitboard Rank3BB = Rank1BB << (8 * 2);
constexpr Bitboard Rank4BB = Rank1BB << (8 * 3);
constexpr Bitboard Rank5BB = Rank1BB << (8 * 4);
constexpr Bitboard Rank6BB = Rank1BB << (8 * 5);
constexpr Bitboard Rank7BB = Rank1BB << (8 * 6);
constexpr Bitboard Rank8BB = Rank1BB << (8 * 7);

constexpr Bitboard QueenSide = FileABB | FileBBB | FileCBB | FileDBB;
constexpr Bitboard CenterFiles = FileCBB | FileDBB | FileEBB | FileFBB;
constexpr Bitboard KingSide = FileEBB | FileFBB | FileGBB | FileHBB;
constexpr Bitboard Center = (FileDBB | FileEBB) & (Rank4BB | Rank5BB);
constexpr Bitboard CenterKing = Center & KingSide;
constexpr Bitboard CenterQueen = Center & QueenSide;


constexpr Bitboard KingFlank[FILE_NB] = {
  QueenSide ^ FileDBB, QueenSide, QueenSide,
  CenterFiles, CenterFiles,
  KingSide, KingSide, KingSide ^ FileEBB
};

//PIECE_STR[piece] is the algebraic chess representation of that piece
const std::string PIECE_STR = "PNBRQK~>pnbrqk ";
const std::string PIECE_PRT = "ANBRQK~>anbrqk ";

//The FEN of the starting position
const std::string DEFAULT_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";

constexpr Piece MakePiece(Color c, PieceType pt) {
	return Piece((c << 3) + pt);
}

constexpr PieceType TypeOf(Piece pc) {
	return PieceType(pc & 0b111);
}

constexpr Color ColorOf(Piece pc) {
	return Color((pc & 0b1000) >> 3);
}

extern const string SQSTR[65];
extern const string MOVE_TYPESTR_UCI[8];

extern const Bitboard MASK_FILE[8];
extern const Bitboard MASK_RANK[8];
extern const Bitboard MASK_DIAGONAL[15];
extern const Bitboard MASK_ANTI_DIAGONAL[15];
extern const Bitboard SQUARE_BB[65];

extern void PrintBitboard(Bitboard b);

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
extern void InitPosition();

constexpr Rank RankOf(Square s) { return Rank(s >> 3); }
constexpr File FileOf(Square s) { return File(s & 0b111); }
constexpr int diagonal_of(Square s) { return 7 + RankOf(s) - FileOf(s); }
constexpr int anti_diagonal_of(Square s) { return RankOf(s) + FileOf(s); }
constexpr Square CreateSquare(File f, Rank r) { return Square(r << 3 | f); }

//Shifts a bitboard in a particular direction. There is no wrapping, so bits that are shifted of the edge are lost 
constexpr Bitboard Shift(Direction D, Bitboard b) {
	return D == NORTH ? b << 8 : D == SOUTH ? b >> 8
		: D == NORTH + NORTH ? b << 16 : D == SOUTH + SOUTH ? b >> 16
		: D == EAST ? (b & ~MASK_FILE[FILE_H]) << 1 : D == WEST ? (b & ~MASK_FILE[FILE_A]) >> 1
		: D == NORTH_EAST ? (b & ~MASK_FILE[FILE_H]) << 9
		: D == NORTH_WEST ? (b & ~MASK_FILE[FILE_A]) << 7
		: D == SOUTH_EAST ? (b & ~MASK_FILE[FILE_H]) >> 7
		: D == SOUTH_WEST ? (b & ~MASK_FILE[FILE_A]) >> 9
		: 0;
}


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
constexpr Bitboard PawnAttacks(Color c, Bitboard p) {
	return c == WHITE ? Shift(NORTH_WEST, p) | Shift(NORTH_EAST, p) :
		Shift(SOUTH_WEST, p) | Shift(SOUTH_EAST, p);
}

//Returns a bitboard containing pawn attacks from the pawn on the given square
constexpr Bitboard PawnAttacks(Color c, Square s) {
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

extern const Bitboard k1;
extern const Bitboard k2;
extern const Bitboard k4;
extern const Bitboard kf;

extern inline int PopCount(Bitboard x);
extern inline int SparsePopCount(Bitboard x);
extern inline Square pop_lsb(Bitboard* b);

extern const int DEBRUIJN64[64];
extern const Bitboard MAGIC;
extern constexpr Square bsf(Bitboard b);

constexpr Bitboard Span(Color color, Bitboard b) {
	return color ? b | b >> 8 | b >> 16 | b >> 24 | b >> 32 : b | b << 8 | b << 16 | b << 24 | b << 32;
}

//Returns the actual rank from a given side's perspective (e.g. rank 1 is rank 8 from Black's perspective)
constexpr Rank RelativeRank(Color c, Rank r) {
	return c == WHITE ? r : Rank(RANK_8 - r);
}

//Returns the actual direction from a given side's perspective (e.g. North is South from Black's perspective)
constexpr Direction RelativeDir(Color c, Direction d) {
	return Direction(c == WHITE ? d : -d);
}


//The white king and kingside rook
const Bitboard WHITE_OO_MASK = 0x90;
//The white king and queenside rook
const Bitboard WHITE_OOO_MASK = 0x11;

//Squares between the white king and kingside rook
const Bitboard WHITE_OO_BLOCKERS_AND_ATTACKERS_MASK = 0x60;
//Squares between the white king and queenside rook
const Bitboard WHITE_OOO_BLOCKERS_AND_ATTACKERS_MASK = 0xe;

//The black king and kingside rook
const Bitboard BLACK_OO_MASK = 0x9000000000000000;
//The black king and queenside rook
const Bitboard BLACK_OOO_MASK = 0x1100000000000000;
//Squares between the black king and kingside rook
const Bitboard BLACK_OO_BLOCKERS_AND_ATTACKERS_MASK = 0x6000000000000000;
//Squares between the black king and queenside rook
const Bitboard BLACK_OOO_BLOCKERS_AND_ATTACKERS_MASK = 0xE00000000000000;

//The white king, white rooks, black king and black rooks
const Bitboard ALL_CASTLING_MASK = 0x9100000000000091;

constexpr Bitboard oo_mask(Color c) { return c == WHITE ? WHITE_OO_MASK : BLACK_OO_MASK; }
constexpr Bitboard ooo_mask(Color c) { return c == WHITE ? WHITE_OOO_MASK : BLACK_OOO_MASK; }

constexpr Bitboard oo_blockers_mask(Color c) {
	return c == WHITE ? WHITE_OO_BLOCKERS_AND_ATTACKERS_MASK :
		BLACK_OO_BLOCKERS_AND_ATTACKERS_MASK;
}

constexpr Bitboard ooo_blockers_mask(Color c) {
	return c == WHITE ? WHITE_OOO_BLOCKERS_AND_ATTACKERS_MASK :
		BLACK_OOO_BLOCKERS_AND_ATTACKERS_MASK;
}

constexpr Bitboard ignore_ooo_danger(Color c) { return c == WHITE ? 0x2 : 0x200000000000000; }

class Move {
public:
	U16 move;

	//Defaults to a null move (a1a1)
	inline Move() : move(0) {}
	inline Move(U16 m) { move = m; }
	inline Move(Square from, Square to) : move(0) { move = (from << 6) | to; }
	inline Move(Square from, Square to, MoveFlags flags) : move(0) { move = (flags << 12) | (from << 6) | to; }
	Move(const std::string& move);

	inline Square To() const { return Square(move & 0x3f); }
	inline Square From() const { return Square((move >> 6) & 0x3f); }
	inline int ToFrom() const { return move & 0xffff; }
	inline MoveFlags Flags() const { return MoveFlags((move >> 12) & 0xf); }
	inline bool IsCapture() const { return (move >> 12) & CAPTURE; }
	inline bool IsProm() const { return (move >> 12) & PROMOTION; }
	inline bool IsQuiet() const { return !((move >> 12) & 0b1100); }
	string ToUci() const;

	void operator=(Move m) { move = m.move; }
	bool operator==(Move a) const { return move == a.move; }
	bool operator!=(Move a) const { return move != a.move; }
};


extern std::ostream& operator<<(std::ostream& os, Move& m);

struct PickerE {
	Move move = MOVE_NONE;
	Value value = VALUE_ZERO;
};

class Picker {
public:
	int best = 0;
	int count = 0;
	Move mList[228];
	PickerE pList[228];
	void Fill();
	PickerE Pick(int index);
	bool SetBest(Move);
};

struct SEvalSide {
	bool chance = false;
	int piece[PT_NB] = {};
	Color color = COLOR_NB;
	Square king = SQUARE_NB;
};

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

//transposition.cpp
enum Bound :U8 {
	BOUND_NONE,
	BOUND_UPPER,
	BOUND_LOWER,
	BOUND_EXACT = BOUND_UPPER | BOUND_LOWER
};

struct CRec {
	Hash hash;//8
	Move move;//2
	S16 score;//2
	S8 depth;//1
	Bound bound;//1 
	U16 age;//2

	Value GetValue() const { return (Value)score; }

	void SetRec(Hash h, Value s, Move m, Bound t, Depth d, U16 a) {
		hash = h;
		score = (S16)s;
		move = m;
		bound = t;
		depth = d;
		age = a;
	}
};

class CTranspositionTable
{
	U64 records = 0;
	U64 mask = 0;
	CRec* rt = NULL;
	int limit = 2;
public:
	U16 age = limit;
	~CTranspositionTable();
	void Clear();
	int Permill() const;
	void Resize(U64 mbSize);
	bool SetRec(Hash hash, Value score, Move move, Bound type, Depth depth);
	CRec* GetRec(Hash hash);

};

extern CTranspositionTable tt;

//uci.cpp
struct Options {
	bool ponder = true;
	int hash = 32;
	int elo = 2500;
	int eloMin = 0;
	int eloMax = 2500;
	int multiPV = 1;

	int aspiration = 35;
	int futility = 749;
	int lmr = 183;
	int nullMove = 939;
	int razoring = 533;
	int rfp = 120;
	string bishop = "32 54 -36 -3";
	string defense = "12 14 12 19 -6 17 -4 13 -59 12 -49 22";
	string king = "52 39";
	string material = "-27 14 23 -37 30";
	string mobility = "8 6 4 8 3 5 3 1";
	string outFile = "1 -5 -4 -7 -26 -4 -6 -8 -5 1 12 -15";
	string outpost = "78 3 11 4";
	string outRank = "2 57 -17 6 -18 1 2 4 -10 13 16 -24";
	string passed = "-5 8 -48 -5 5";
	string pawn = "3 7 -30 -24 -8 -22 -10 8";
	string rook = "77 9 29 11";
	string tempo = "20 9";

};
extern Options options;


//A PsuedoRandom Number Generator
class PRNG {
	uint64_t s;

	uint64_t rand64() {
		s ^= s >> 12, s ^= s << 25, s ^= s >> 27;
		return s * 2685821657736338717LL;
	}

public:
	PRNG(uint64_t seed) : s(seed) {}

	//Generate psuedorandom number
	template<typename T> T rand() { return T(rand64()); }

	//Generate psuedorandom number with only a few set bits
	template<typename T> T sparse_rand() { return T(rand64() & rand64() & rand64()); }
};

namespace zobrist {
	extern Hash hashColor;
	extern Hash zobrist_table[PIECE_NB][SQUARE_NB];
	extern void InitialiseZobristKeys();
}

//Stores position information which cannot be recovered on undo-ing a move
struct UndoInfo {

	//The bitboard of squares on which pieces have either moved from, or have been moved to. Used for castling legality checks
	Bitboard entry;

	//The piece that was captured on the last move
	Piece captured;

	//The en passant square
	Square epsq;

	Hash hash;

	int move50;
	int phase;

	constexpr UndoInfo() : hash(0), move50(0), phase(0), entry(0), captured(NO_PIECE), epsq(SQ_NONE) {}

	//This preserves the entry bitboard across moves
	constexpr UndoInfo(const UndoInfo& prev) :
		hash(0), move50(0), phase(0), entry(prev.entry), captured(NO_PIECE), epsq(SQ_NONE) {
	}
};

class Position {
public:
	//int phaseVal[PIECE_NB] = { 0,1,1,2,4,0,0,0,0,1,1,2,4 };
	//The side whose turn it is to play next
	Color color;
	//The zobrist hash of the position, which can be incrementally updated and rolled back after each make/unmake
	Hash hash;
	//A mailbox representation of the board. Stores the piece occupying each square on the board
	Piece board[SQUARE_NB];
	int move50;
	int historyIndex;

	//A bitboard of the locations of each piece
	Bitboard piece_bb[PIECE_NB];

	//The history of non-recoverable information
	UndoInfo history[512] = {};

	//The bitboard of enemy pieces that are currently attacking the king, updated whenever generate_moves() is called
	Bitboard checkers;

	//The bitboard of pieces that are currently pinned to the king by enemy sliders, updated whenever generate_moves() is called
	Bitboard pinned;

	//constexpr Position(piece_bb{ 0 }, side_to_play(WHITE), historyIndex(0), board{}, hash(0), pinned(0), checkers(0));
	constexpr Position();

	//Places a piece on a particular square and updates the hash. Placing a piece on a square that is already occupied is an error
	inline void PutPiece(Piece pc, Square s);

	//Removes a piece from a particular square and updates the hash. 
	inline void RemovePiece(Square s);
	void MovePiece(Square from, Square to);
	void MovePieceQuiet(Square from, Square to);
	void Clear();
	void SetFen(const string& fen = DEFAULT_FEN);
	string GetFen() const;
	inline Bitboard bitboard_of(Piece pc) const { return piece_bb[pc]; }
	inline Bitboard bitboard_of(Color c, PieceType pt) const { return piece_bb[MakePiece(c, pt)]; }
	inline Bitboard AllPieces()const;
	inline Bitboard AllPieces(Color c) const;
	inline Bitboard Pieces(PieceType pt)const;
	inline Bitboard Pieces(PieceType pt, Color c) const;
	inline Piece Board(Square sq) const { return board[sq]; }
	inline Color ColorUs() const { return color; }
	inline Color ColorEn() const { return ~color; }
	inline bool ColorBlack() const { return color; }
	inline bool ColorWhite() const { return ~color; }
	inline Bitboard East(const Bitboard bb) { return (bb << 1) & ~0x0101010101010101ULL; }
	inline Bitboard West(const Bitboard bb) { return (bb >> 1) & ~0x8080808080808080ULL; }
	inline Bitboard North(const Bitboard bb) { return bb << 8; }
	inline Bitboard South(const Bitboard bb) { return bb >> 8; }
	inline Bitboard NorthWest(const Bitboard bb) { return North(West(bb)); }
	inline Bitboard NorthEast(const Bitboard bb) { return North(East(bb)); }
	inline Bitboard SouthWest(const Bitboard bb) { return South(West(bb)); }
	inline Bitboard SouthEast(const Bitboard bb) { return South(East(bb)); }
	void MoveList(Color color, Move* list, int& count, bool quiet = true);
	void MoveList(Move* list, int& count, bool quiet = true);
	//bool InCheck(Color color) const;
	bool InCheck() const;
	//inline bool InCheck() { return checkers; };
	bool IsLegal(Move m);
	bool IsRepetition() const;
	inline int HistoryIndex() const { return historyIndex; }
	inline Hash GetHash() const { return hash; }
	inline Bitboard DiagonalSliders(Color c) const;
	inline Bitboard OrthogonalSliders(Color c) const;
	inline Bitboard AttackersFrom(Color c, Square s, Bitboard occ) const;
	inline Bitboard Attackers(Square s) const;
	void MakeNull();
	void UnmakeNull();
	void MakeMove(const Move m);
	void UnmakeMove(const Move m);
	inline bool NotOnlyPawns()const;
	Move* GenerateMoves(Color Us, Move* list, bool quiet = true);
	void PrintBoard() const;
	Position& operator=(const Position&) = delete;
	inline bool operator==(const Position& other) const { return hash == other.hash; }
};

extern Position g_pos;

//Return true if active player got major or minor pieces
inline bool Position::NotOnlyPawns()const {
	return color == WHITE ?
		piece_bb[WHITE_KNIGHT] || piece_bb[WHITE_BISHOP] || piece_bb[WHITE_ROOK] || piece_bb[WHITE_QUEEN] :
		piece_bb[BLACK_KNIGHT] || piece_bb[BLACK_BISHOP] || piece_bb[BLACK_ROOK] || piece_bb[BLACK_QUEEN];
}

inline Bitboard Position::Pieces(PieceType pt) const {
	return piece_bb[pt] | piece_bb[pt | 0b1000];
}

inline Bitboard Position::Pieces(PieceType pt, Color c) const {
	return c ? piece_bb[pt] : piece_bb[pt | 0b1000];
}

//Returns a bitboard containing all the pieces of a given color
inline Bitboard Position::AllPieces(Color c) const {
	return c == WHITE ?
		piece_bb[WHITE_PAWN] | piece_bb[WHITE_KNIGHT] | piece_bb[WHITE_BISHOP] | piece_bb[WHITE_ROOK] | piece_bb[WHITE_QUEEN] | piece_bb[WHITE_KING] :
		piece_bb[BLACK_PAWN] | piece_bb[BLACK_KNIGHT] | piece_bb[BLACK_BISHOP] | piece_bb[BLACK_ROOK] | piece_bb[BLACK_QUEEN] | piece_bb[BLACK_KING];
}

//Returns a bitboard containing all the pieces
inline Bitboard Position::AllPieces() const {
	return
		piece_bb[WHITE_PAWN] | piece_bb[WHITE_KNIGHT] | piece_bb[WHITE_BISHOP] | piece_bb[WHITE_ROOK] | piece_bb[WHITE_QUEEN] | piece_bb[WHITE_KING] |
		piece_bb[BLACK_PAWN] | piece_bb[BLACK_KNIGHT] | piece_bb[BLACK_BISHOP] | piece_bb[BLACK_ROOK] | piece_bb[BLACK_QUEEN] | piece_bb[BLACK_KING];
}

//A convenience class for interfacing with legal moves, rather than using the low-level generate_legals() function directly
class CMoveList {
public:
	explicit CMoveList(Position& p, Color color) : last(p.GenerateMoves(color, list)) {}
	explicit CMoveList(Position& p) : last(p.GenerateMoves(p.ColorUs(), list)) {}
	const Move* begin() const { return list; }
	const Move* end() const { return last; }
	size_t size() const { return last - list; }
	Move list[218];
private:
	Move* last;
};

struct SearchInfo {
	bool stop = false;
	bool post = true;
	bool infinite = false;
	bool ponder = false;
	int multiPV = 1;
	Depth depthLimit = DEPTH_MAX;
	S64 timeStart = 0;
	S64 timeLimit = 0;
	U64 nodes = 0;
	U64 nodesLimit = 0;
	vector<Move> rootMoves;
	Move bestMove = MOVE_NONE;
	Move ponderMove = MOVE_NONE;
};
extern SearchInfo info;

bool CheckUp();
Value Eval();
Value Eval(Move m);
void InitEval();
Value ShowEval();
U64 GetTimeMs();
void InitSearch();
void SplitInt(const std::string& txt, std::vector<int>& vInt, char ch);
void SplitString(const std::string& txt, std::vector<std::string>& vStr, char ch);
string thousandSeparator(uint64_t n);
string Trim(const string& s);
string StrToLower(std::string s);
bool GetInput(std::string& s);
int InitImput();
void PrintSummary(U64 time, U64 nodes);
void ResetLimit();
void SearchIteratively();
void UciBench();
void UciCommand(string str);
void UciLoop();
int ValueToCp(Value v);