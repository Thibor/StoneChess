#pragma once
#pragma warning(disable : 26812)

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <ostream>
#include <iostream>
#include <vector>

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
#define Value signed __int16
#define Depth signed __int16
//#define Score signed __int32

//constexpr int MAX_PLY = 128;
#define MOVE_NONE 0
#define VALUE_ZERO 0

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

enum NodeTypes {NTPV, NTNONPV};

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

/*enum Value : S16 {
	VALUE_ZERO = 0,
	VALUE_DRAW = 0,
	VALUE_KNOWN_WIN = 10000,
	VALUE_MATE = 32000,
	VALUE_INFINITE = 32001,
	VALUE_NONE = 32002,

	//VALUE_MATE_IN_MAX_PLY = VALUE_MATE - 2 * MAX_PLY,
	//VALUE_MATED_IN_MAX_PLY = -VALUE_MATE + 2 * MAX_PLY,

	PawnValueMg = 136, PawnValueEg = 208,
	KnightValueMg = 782, KnightValueEg = 865,
	BishopValueMg = 830, BishopValueEg = 918,
	RookValueMg = 1289, RookValueEg = 1378,
	QueenValueMg = 2529, QueenValueEg = 2687,

	MidgameLimit = 15258, EndgameLimit = 3915
};*/

/*inline Square& operator++(Square& s) { return s = Square(int(s) + 1); }
constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
inline Square& operator+=(Square& s, Direction d) { return s = s + d; }
inline Square& operator-=(Square& s, Direction d) { return s = s - d; }
constexpr Color operator~(Color c) {return Color(c ^ BLACK);}*/

enum Score : int { SCORE_ZERO };


inline Rank& operator++(Rank& r) { return r = Rank(int(r) + 1); }
inline File& operator++(File& f) { return f = File(int(f) + 1); }
inline PieceType& operator++(PieceType& pt) { return pt = PieceType(int(pt) + 1); }
inline Square& operator++(Square& s) { return s = Square(int(s) + 1); }
constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
inline Square& operator+=(Square& s, Direction d) { return s = s + d; }
inline Square& operator-=(Square& s, Direction d) { return s = s - d; }
constexpr Color operator~(Color c) { return Color(c ^ BLACK); }
inline Color& operator++(Color& c) { return c = Color(int(c) + 1); }
constexpr Score operator-(Score s) { return Score(-int(s)); }
constexpr Score operator+(Score a, Score b) { return Score(int(a) + int(b)); }
constexpr Score operator-(Score a, Score b) { return Score(int(a) - int(b)); }
constexpr Score operator*(Score a, Score b) { return Score(int(a) * int(b)); }
constexpr Score operator/(Score a, Score b) { return Score(int(a) / int(b)); }
inline Score& operator+=(Score& a, Score b) { return a = a + b; }
inline Score& operator-=(Score& a, Score b) { return a = a - b; }

constexpr Score S(int mg, int eg) {
	return Score((int)((unsigned int)eg << 16) + mg);
}

/// Extracting the signed lower and upper 16 bits is not so trivial because
/// according to the standard a simple cast to short is implementation defined
/// and so is a right shift of a signed integer.
inline Value Eg(Score s) {
	union { uint16_t u; int16_t s; } eg = { uint16_t(unsigned(s + 0x8000) >> 16) };
	return Value(eg.s);
}

inline Value Mg(Score s) {
	union { uint16_t u; int16_t s; } mg = { uint16_t(unsigned(s)) };
	return Value(mg.s);
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
const std::string PIECE_STR = "PNBRQK~>pnbrqk.";

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

constexpr Rank RankOf(Square s) { return Rank(s >> 3); }
constexpr File FileOf(Square s) { return File(s & 0b111); }
constexpr int diagonal_of(Square s) { return 7 + RankOf(s) - FileOf(s); }
constexpr int anti_diagonal_of(Square s) { return RankOf(s) + FileOf(s); }
constexpr Square CreateSquare(File f, Rank r) { return Square(r << 3 | f); }
constexpr int Distance(Square s1, Square s2) { return max(abs(FileOf(s1) - FileOf(s2)), abs(RankOf(s1) - RankOf(s2))); };

//Shifts a bitboard in a particular direction. There is no wrapping, so bits that are shifted of the edge are lost 
 constexpr Bitboard Shift(Direction D,Bitboard b) {
	return D == NORTH ? b << 8 : D == SOUTH ? b >> 8
		: D == NORTH + NORTH ? b << 16 : D == SOUTH + SOUTH ? b >> 16
		: D == EAST ? (b & ~MASK_FILE[FILE_H]) << 1 : D == WEST ? (b & ~MASK_FILE[FILE_A]) >> 1
		: D == NORTH_EAST ? (b & ~MASK_FILE[FILE_H]) << 9
		: D == NORTH_WEST ? (b & ~MASK_FILE[FILE_A]) << 7
		: D == SOUTH_EAST ? (b & ~MASK_FILE[FILE_H]) >> 7
		: D == SOUTH_WEST ? (b & ~MASK_FILE[FILE_A]) >> 9
		: 0;
}

 constexpr Bitboard Span(Color color,Bitboard b) {
	 return color ? b | b >> 8 | b >> 16 | b >> 24 | b >> 32 : b | b << 8 | b << 16 | b << 24 | b << 32;
 }

//Returns the actual rank from a given side's perspective (e.g. rank 1 is rank 8 from Black's perspective)
constexpr Rank RelativeRank(Color c,Rank r) {
	return c == WHITE ? r : Rank(RANK_8 - r);
}

//Returns the actual direction from a given side's perspective (e.g. North is South from Black's perspective)
constexpr Direction RelativeDir(Color c,Direction d) {
	return Direction(c == WHITE ? d : -d);
}

class Move {
public:
	U16 move;

	//Defaults to a null move (a1a1)
	inline Move() : move(0) {}
	inline Move(uint16_t m) { move = m; }
	inline Move(Square from, Square to) : move(0) {move = (from << 6) | to;}
	inline Move(Square from, Square to, MoveFlags flags) : move(0) {move = (flags << 12) | (from << 6) | to;}
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
	bool operator==(Move a) const { return ToFrom() == a.ToFrom(); }
	bool operator!=(Move a) const { return ToFrom() != a.ToFrom(); }
};

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

//extern std::ostream& operator<<(std::ostream& os, const Move& m);
extern std::ostream& operator<<(std::ostream& os, Move m);

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

struct [[nodiscard]] Stack {
	Move move;
	Move killer1;
	Move killer2;
	Value score = 0;
};

extern Stack stack[128];
