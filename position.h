#pragma once

#include <ostream>
#include <string>
#include <utility>
#include "tables.h"
#include "types.h"
#include "move.h"

//A psuedorandom number generator
//Source: Stockfish
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

/*U64 rand64() {
	static U64 next = 1;

	next = next * 1103515245 + 12345;
	return next;
}*/


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

	constexpr UndoInfo() : hash(0), move50(0),phase(0), entry(0), captured(NO_PIECE), epsq(SQ_NONE) {}

	//This preserves the entry bitboard across moves
	constexpr UndoInfo(const UndoInfo& prev) :
		hash(0), move50(0),phase(0), entry(prev.entry), captured(NO_PIECE), epsq(SQ_NONE) {}
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
