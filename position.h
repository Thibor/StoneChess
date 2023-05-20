#pragma once

#include <ostream>
#include <string>
#include <utility>
#include "tables.h"
#include "types.h"

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
	template<typename T>
	T sparse_rand() {
		return T(rand64() & rand64() & rand64());
	}
};


namespace zobrist {
	extern U64 hashColor;
	extern U64 zobrist_table[NPIECES][NSQUARES];
	extern void initialise_zobrist_keys();
}

//Stores position information which cannot be recovered on undo-ing a move
struct UndoInfo {
	bool inCheck;
	//The bitboard of squares on which pieces have either moved from, or have been moved to. Used for castling
	//legality checks
	Bitboard entry;

	//The piece that was captured on the last move
	Piece captured;

	//The en passant square. This is the square which pawns can move to in order to en passant capture an enemy pawn that has 
	//double pushed on the previous move
	Square epsq;

	uint64_t hash;

	int move50;

	constexpr UndoInfo() : inCheck(false),hash(0),move50(0), entry(0), captured(NO_PIECE), epsq(NO_SQUARE) {}

	//This preserves the entry bitboard across moves
	UndoInfo(const UndoInfo& prev) :
		inCheck(false),hash(0), move50(0),entry(prev.entry), captured(NO_PIECE), epsq(NO_SQUARE) {}
};

class Position {
private:
	//The side whose turn it is to play next
	Color side_to_play;

	//The zobrist hash of the position, which can be incrementally updated and rolled back after each
	//make/unmake
	uint64_t hash;
public:
	bool inCheck = false;
	int move50;
	int historyIndex;
	int phase = 28;
	//A mailbox representation of the board. Stores the piece occupying each square on the board
	Piece board[NSQUARES];

	//A bitboard of the locations of each piece
	Bitboard piece_bb[NPIECES];

	//The history of non-recoverable information
	UndoInfo history[512];

	//The bitboard of enemy pieces that are currently attacking the king, updated whenever generate_moves()
	//is called
	Bitboard checkers;

	//The bitboard of pieces that are currently pinned to the king by enemy sliders, updated whenever 
	//generate_moves() is called
	Bitboard pinned;


	Position() : piece_bb{ 0 }, side_to_play(WHITE), historyIndex(0), board{},
		hash(0), pinned(0), checkers(0) {

		//Sets all squares on the board as empty
		for (int i = 0; i < 64; i++) board[i] = NO_PIECE;
		history[0] = UndoInfo();
	}

	//Places a piece on a particular square and updates the hash. Placing a piece on a square that is 
	//already occupied is an error
	inline void put_piece(Piece pc, Square s) {
		board[s] = pc;
		piece_bb[pc] |= SQUARE_BB[s];
		hash ^= zobrist::zobrist_table[pc][s];
	}

	//Removes a piece from a particular square and updates the hash. 
	inline void remove_piece(Square s) {
		hash ^= zobrist::zobrist_table[board[s]][s];
		piece_bb[board[s]] &= ~SQUARE_BB[s];
		board[s] = NO_PIECE;
	}

	void move_piece(Square from, Square to);
	void move_piece_quiet(Square from, Square to);


	friend std::ostream& operator<<(std::ostream& os, const Position& p);
	void SetFen(const std::string& fen= DEFAULT_FEN);
	std::string GetFen() const;

	void Clear() {
		side_to_play = WHITE;
		historyIndex = 0;
		hash = 0;
		pinned = 0;
		checkers = 0;
		for (int n = 0; n < NSQUARES; n++)
			board[n] = NO_PIECE;
		for (int n = 0; n < NPIECES; n++)
			piece_bb[n] = 0;
		history[0] = UndoInfo();
	};

	Position& operator=(const Position&) = delete;
	inline bool operator==(const Position& other) const { return hash == other.hash; }

	inline Bitboard bitboard_of(Piece pc) const { return piece_bb[pc]; }
	inline Bitboard bitboard_of(Color c, PieceType pt) const { return piece_bb[make_piece(c, pt)]; }
	inline Bitboard AllPieces()const {
		return 
			piece_bb[WHITE_PAWN] | piece_bb[WHITE_KNIGHT] | piece_bb[WHITE_BISHOP] | piece_bb[WHITE_ROOK] | piece_bb[WHITE_QUEEN] | piece_bb[WHITE_KING] | 
			piece_bb[BLACK_PAWN] | piece_bb[BLACK_KNIGHT] | piece_bb[BLACK_BISHOP] | piece_bb[BLACK_ROOK] | piece_bb[BLACK_QUEEN] | piece_bb[BLACK_KING];
	}
	inline Piece at(Square sq) const { return board[sq]; }
	inline Color ColorUs() const { return side_to_play; }
	inline Color ColorEn() const { return ~side_to_play; }
	void MoveList(Color color, Move *list, int& count);
	void MoveListQ(Color color, Move *list, int& count);
	void MoveList(Move* list, int& count);
	void MoveListQ(Move* list, int& count);
	bool InCheck(Color color);
	bool InCheck();
	bool IsLegal(Move m);
	int Phase() {
		int result = pop_count(AllPieces())-4;
		return result < 0 ? 0 : result;
	};
	bool IsRepetition() {
		for (int n = historyIndex - 2; n >= historyIndex - move50; n -= 2)
			if (n < 0)
				return false;
			else if (history[n].hash == hash)
				return true;
		return false;
	}

	inline int ply() const { return historyIndex; }
	inline uint64_t get_hash() const { return hash; }

	inline Bitboard DiagonalSliders(Color c) const;
	inline Bitboard OrthogonalSliders(Color c) const;
	inline Bitboard AllPieces(Color c) const;
	inline Bitboard AttackersFrom(Color c,Square s, Bitboard occ) const;
	inline Bitboard Attackers(Square s) const;
	void MakeNull();
	void UnmakeNull();
	void MakeMove(const Move m);
	void UnmakeMove(const Move m);
	inline bool NotOnlyPawns()const;
	Move* GenerateMoves(Color Us,Move* list,bool quiet = true);
};

extern Position position;

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

inline bool Position::NotOnlyPawns()const{
	return side_to_play == WHITE ?
		piece_bb[WHITE_KNIGHT] || piece_bb[WHITE_BISHOP] || piece_bb[WHITE_ROOK] || piece_bb[WHITE_QUEEN] :
		piece_bb[BLACK_KNIGHT] || piece_bb[BLACK_BISHOP] || piece_bb[BLACK_ROOK] || piece_bb[BLACK_QUEEN];
}

//Returns a bitboard containing all the pieces of a given color
inline Bitboard Position::AllPieces(Color c) const {
	return c == WHITE ?
		piece_bb[WHITE_PAWN] | piece_bb[WHITE_KNIGHT] | piece_bb[WHITE_BISHOP] |
		piece_bb[WHITE_ROOK] | piece_bb[WHITE_QUEEN] | piece_bb[WHITE_KING] :

		piece_bb[BLACK_PAWN] | piece_bb[BLACK_KNIGHT] | piece_bb[BLACK_BISHOP] |
		piece_bb[BLACK_ROOK] | piece_bb[BLACK_QUEEN] | piece_bb[BLACK_KING];
}

//inline Bitboard Position::AllPieces() const {return AllPieces(WHITE) | AllPieces(BLACK);}

//Returns a bitboard containing all pieces of a given color attacking a particluar square
inline Bitboard Position::AttackersFrom(Color c,Square s, Bitboard occ) const {
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

inline Bitboard Position::Attackers(Square s) const {
	Bitboard occ = AllPieces();
	return side_to_play == BLACK ?
		(PawnAttacks(BLACK, s) & piece_bb[WHITE_PAWN]) |
		(attacks<KNIGHT>(s, occ) & piece_bb[WHITE_KNIGHT]) |
		(attacks<BISHOP>(s, occ) & (piece_bb[WHITE_BISHOP] | piece_bb[WHITE_QUEEN])) |
		(attacks<ROOK>(s, occ) & (piece_bb[WHITE_ROOK] | piece_bb[WHITE_QUEEN])) :
		(PawnAttacks(WHITE, s) & piece_bb[BLACK_PAWN]) |
		(attacks<KNIGHT>(s, occ) & piece_bb[BLACK_KNIGHT]) |
		(attacks<BISHOP>(s, occ) & (piece_bb[BLACK_BISHOP] | piece_bb[BLACK_QUEEN])) |
		(attacks<ROOK>(s, occ) & (piece_bb[BLACK_ROOK] | piece_bb[BLACK_QUEEN]));
}

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

//A convenience class for interfacing with legal moves, rather than using the low-level
//generate_legals() function directly. It can be iterated over.
class CMoveList {
public:
	explicit CMoveList(Position& p,Color color) : last(p.GenerateMoves(color,list)) {}
	explicit CMoveList(Position& p) : last(p.GenerateMoves(p.ColorUs(),list)) {}
	const Move* begin() const { return list; }
	const Move* end() const { return last; }
	size_t size() const { return last - list; }
	Move list[218];
private:
	Move* last;
};
