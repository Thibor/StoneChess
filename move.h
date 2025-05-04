#pragma once

#include <string>

#include "types.h"

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