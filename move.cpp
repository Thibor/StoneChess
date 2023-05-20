#include "move.h"

class Move {
private:
	//The internal representation of the move
	uint16_t move;
public:
	//Defaults to a null move (a1a1)
	inline Move() : move(0) {}

	inline Move(uint16_t m) { move = m; }

	inline Move(Square from, Square to) : move(0) {
		move = (from << 6) | to;
	}

	inline Move(Square from, Square to, MoveFlags flags) : move(0) {
		move = (flags << 12) | (from << 6) | to;
	}

	Move(const std::string& move) {
		Square fr = create_square(File(move[0] - 'a'), Rank(move[1] - '1'));
		Square to = create_square(File(move[2] - 'a'), Rank(move[3] - '1'));
		MoveFlags mf = MoveFlags::QUIET;
		if (move.length() > 4)
			if (move[5] == 'q')
				mf = MoveFlags::PC_QUEEN;
			else if (move[5] == 'r')
				mf = MoveFlags::PC_ROOK;
			else if (move[5] == 'b')
				mf = MoveFlags::PC_BISHOP;
			else if (move[5] == 'n')
				mf = MoveFlags::PC_KNIGHT;
		this->move = (mf << 12) | (fr << 6) | to;
	}

	inline Square to() const { return Square(move & 0x3f); }
	inline Square from() const { return Square((move >> 6) & 0x3f); }
	inline int to_from() const { return move & 0xffff; }
	inline MoveFlags flags() const { return MoveFlags((move >> 12) & 0xf); }

	inline bool is_capture() const {
		return (move >> 12) & CAPTURE;
	}

	string ToUci() {
		return SQSTR[from()] + SQSTR[to()] + MOVE_TYPESTR_UCI[flags() & 7];
	}

	void operator=(Move m) { move = m.move; }
	bool operator==(Move a) const { return to_from() == a.to_from(); }
	bool operator!=(Move a) const { return to_from() != a.to_from(); }
};
