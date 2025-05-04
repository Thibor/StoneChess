#include "move.h"

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