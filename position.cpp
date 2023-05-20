#include "position.h"

#include "tables.h"

#include <sstream>

Position position;

U64 zobrist::hashColor;
//Zobrist keys for each piece and each square
//Used to incrementally update the hash key of a position
U64 zobrist::zobrist_table[NPIECES][NSQUARES];


//Initializes the zobrist table with random 64-bit numbers
void zobrist::initialise_zobrist_keys() {
	PRNG rng(70026072);
	hashColor = rng.rand<U64>();
	for (int i = 0; i < NPIECES; i++)
		for (int j = 0; j < NSQUARES; j++)
			zobrist::zobrist_table[i][j] = rng.rand<U64>();
}

//Pretty-prints the position (including FEN and hash key)
std::ostream& operator<< (std::ostream& os, const Position& p) {
	const char* s = "   +---+---+---+---+---+---+---+---+\n";
	const char* t = "     A   B   C   D   E   F   G   H\n";
	os << t;
	for (int i = 56; i >= 0; i -= 8) {
		os << s << " " << i / 8 + 1 << " ";
		for (int j = 0; j < 8; j++)
			os << "| " << PIECE_STR[p.board[i + j]] << " ";
		os << "| " << i / 8 + 1 << endl;
	}
	os << s;
	os << t << endl;
	os << "FEN: " << p.GetFen() << endl;
	os << "Hash: 0x" << std::hex << p.hash << std::dec << endl;
	os << "Side: " << (p.side_to_play == WHITE ? "w" : "b") << endl;
	return os;
}

void Position::MoveList(Color color, Move* list, int& count) {
	Move* last = GenerateMoves(color, list);
	count = last - list;
}

void Position::MoveListQ(Color color, Move* list, int& count) {
	Move* last = GenerateMoves(color, list, false);
	count = last - list;
}

void Position::MoveList(Move* list, int& count) {
	Move* last = GenerateMoves(ColorUs(), list);
	count = last - list;
}

void Position::MoveListQ(Move* list, int& count) {
	Move* last = GenerateMoves(ColorUs(), list, false);
	count = last - list;
}

bool Position::IsLegal(Move move) {
	Move list[218];
	GenerateMoves(side_to_play, list);
	for (Move m : list)
		if (m == move)
			return true;
	return false;
}

bool Position::InCheck(Color c) {
	return AttackersFrom(~c, bsf(bitboard_of(c, KING)), AllPieces());
}

bool Position::InCheck() {
	return AttackersFrom(~side_to_play, bsf(bitboard_of(side_to_play, KING)), AllPieces());
}

void Position::MakeNull() {
	hash ^= zobrist::hashColor;
	side_to_play = ~side_to_play;
	++historyIndex;
	history[historyIndex] = UndoInfo(history[historyIndex - 1]);
	history[historyIndex].epsq = NO_SQUARE;
}

void Position::UnmakeNull() {
	hash ^= zobrist::hashColor;
	side_to_play = ~side_to_play;
	--historyIndex;
}

//Plays a move in the position
void Position::MakeMove(const Move m) {
	hash ^= zobrist::hashColor;
	Color c = side_to_play;
	side_to_play = ~side_to_play;
	++historyIndex;
	history[historyIndex] = UndoInfo(history[historyIndex - 1]);
	Square fr = m.from();
	Square to = m.to();
	MoveFlags type = m.flags();
	history[historyIndex].entry |= SQUARE_BB[to] | SQUARE_BB[fr];
	move50 = type == QUIET && !type_of(board[fr]) ? ++move50 : 0;
	switch (type) {
	case QUIET:
		//The to square is guaranteed to be empty here
		move_piece_quiet(m.from(), m.to());
		break;
	case DOUBLE_PUSH:
		//The to square is guaranteed to be empty here
		move_piece_quiet(m.from(), m.to());

		//This is the square behind the pawn that was double-pushed
		history[historyIndex].epsq = m.from() + RelativeDir(c, NORTH);
		break;
	case OO:
		if (c == WHITE) {
			move_piece_quiet(e1, g1);
			move_piece_quiet(h1, f1);
		}
		else {
			move_piece_quiet(e8, g8);
			move_piece_quiet(h8, f8);
		}
		break;
	case OOO:
		if (c == WHITE) {
			move_piece_quiet(e1, c1);
			move_piece_quiet(a1, d1);
		}
		else {
			move_piece_quiet(e8, c8);
			move_piece_quiet(a8, d8);
		}
		break;
	case EN_PASSANT:
		move_piece_quiet(m.from(), m.to());
		remove_piece(m.to() + RelativeDir(c, SOUTH));
		break;
	case PR_KNIGHT:
		remove_piece(m.from());
		put_piece(make_piece(c, KNIGHT), m.to());
		break;
	case PR_BISHOP:
		remove_piece(m.from());
		put_piece(make_piece(c, BISHOP), m.to());
		break;
	case PR_ROOK:
		move50 = 0;
		remove_piece(m.from());
		put_piece(make_piece(c, ROOK), m.to());
		break;
	case PR_QUEEN:
		remove_piece(m.from());
		put_piece(make_piece(c, QUEEN), m.to());
		break;
	case PC_KNIGHT:
		remove_piece(m.from());
		history[historyIndex].captured = board[m.to()];
		remove_piece(m.to());
		put_piece(make_piece(c, KNIGHT), m.to());
		break;
	case PC_BISHOP:
		remove_piece(m.from());
		history[historyIndex].captured = board[m.to()];
		remove_piece(m.to());
		put_piece(make_piece(c, BISHOP), m.to());
		break;
	case PC_ROOK:
		remove_piece(m.from());
		history[historyIndex].captured = board[m.to()];
		remove_piece(m.to());
		put_piece(make_piece(c, ROOK), m.to());
		break;
	case PC_QUEEN:
		remove_piece(m.from());
		history[historyIndex].captured = board[m.to()];
		remove_piece(m.to());
		put_piece(make_piece(c, QUEEN), m.to());
		break;
	case CAPTURE:
		history[historyIndex].captured = board[m.to()];
		move_piece(m.from(), m.to());
		break;
	}
	inCheck = AttackersFrom(c, bsf(bitboard_of(~c, KING)), AllPieces());
	history[historyIndex].inCheck = inCheck;
	history[historyIndex].hash = hash;
	history[historyIndex].move50 = move50;
}

//Undos a move in the current position, rolling it back to the previous position
void Position::UnmakeMove(const Move m) {
	hash ^= zobrist::hashColor;
	side_to_play = ~side_to_play;
	Color c = side_to_play;
	MoveFlags type = m.flags();
	switch (type) {
	case QUIET:
		move_piece_quiet(m.to(), m.from());
		break;
	case DOUBLE_PUSH:
		move_piece_quiet(m.to(), m.from());
		break;
	case OO:
		if (c == WHITE) {
			move_piece_quiet(g1, e1);
			move_piece_quiet(f1, h1);
		}
		else {
			move_piece_quiet(g8, e8);
			move_piece_quiet(f8, h8);
		}
		break;
	case OOO:
		if (c == WHITE) {
			move_piece_quiet(c1, e1);
			move_piece_quiet(d1, a1);
		}
		else {
			move_piece_quiet(c8, e8);
			move_piece_quiet(d8, a8);
		}
		break;
	case EN_PASSANT:
		move_piece_quiet(m.to(), m.from());
		put_piece(make_piece(~c, PAWN), m.to() + RelativeDir(c, SOUTH));
		break;
	case PR_KNIGHT:
	case PR_BISHOP:
	case PR_ROOK:
	case PR_QUEEN:
		remove_piece(m.to());
		put_piece(make_piece(c, PAWN), m.from());
		break;
	case PC_KNIGHT:
	case PC_BISHOP:
	case PC_ROOK:
	case PC_QUEEN:
		remove_piece(m.to());
		put_piece(make_piece(c, PAWN), m.from());
		put_piece(history[historyIndex].captured, m.to());
		break;
	case CAPTURE:
		move_piece_quiet(m.to(), m.from());
		put_piece(history[historyIndex].captured, m.to());
		break;
	}
	--historyIndex;
	inCheck = history[historyIndex].inCheck;
	move50 = history[historyIndex].move50;
}

void Position::SetFen(const std::string& fen) {
	Clear();
	int square = a8;
	for (char ch : fen.substr(0, fen.find(' '))) {
		if (isdigit(ch))
			square += (ch - '0') * EAST;
		else if (ch == '/')
			square += 2 * SOUTH;
		else
			put_piece(Piece(PIECE_STR.find(ch)), Square(square++));
	}

	std::istringstream ss(fen.substr(fen.find(' ')));
	unsigned char token;

	ss >> token;
	side_to_play = token == 'w' ? WHITE : BLACK;

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
	inCheck = InCheck();
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

	fen << (side_to_play == WHITE ? " w " : " b ")
		<< (history[historyIndex].entry & WHITE_OO_MASK ? "" : "K")
		<< (history[historyIndex].entry & WHITE_OOO_MASK ? "" : "Q")
		<< (history[historyIndex].entry & BLACK_OO_MASK ? "" : "k")
		<< (history[historyIndex].entry & BLACK_OOO_MASK ? "" : "q")
		<< (history[historyIndex].entry & ALL_CASTLING_MASK ? "- " : "")
		<< (history[historyIndex].epsq == NO_SQUARE ? " -" : SQSTR[history[historyIndex].epsq]);

	return fen.str();
}

//Moves a piece to a (possibly empty) square on the board and updates the hash
void Position::move_piece(Square from, Square to) {
	hash ^= zobrist::zobrist_table[board[from]][from] ^ zobrist::zobrist_table[board[from]][to]
		^ zobrist::zobrist_table[board[to]][to];
	Bitboard mask = SQUARE_BB[from] | SQUARE_BB[to];
	piece_bb[board[from]] ^= mask;
	piece_bb[board[to]] &= ~mask;
	board[to] = board[from];
	board[from] = NO_PIECE;
}

//Moves a piece to an empty square. Note that it is an error if the <to> square contains a piece
void Position::move_piece_quiet(Square from, Square to) {
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

	switch (sparse_pop_count(checkers)) {
	case 2:
		//If there is a double check, the only legal moves are king moves out of check
		return list;
	case 1: {
		//It's a single check!

		Square checker_square = bsf(checkers);
		Piece piece = board[checker_square];
		PieceType pt = type_of(piece);
		Color pc = color_of(piece);
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
			while (b1) *list++ = Move(pop_lsb(&b1), checker_square, CAPTURE);

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

		if (history[historyIndex].epsq != NO_SQUARE) {
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
					MASK_RANK[rank_of(our_king)]) &
					their_orth_sliders) == 0)
					*list++ = Move(s, history[historyIndex].epsq, EN_PASSANT);
			}

			//Pinned pawns can only capture e.p. if they are pinned diagonally and the e.p. square is in line with the king 
			b1 = b2 & pinned & LINE[history[historyIndex].epsq][our_king];
			if (b1) {
				*list++ = Move(bsf(b1), history[historyIndex].epsq, EN_PASSANT);
			}
		}

		if (quiet) {
			//Only add castling if:
			//1. The king and the rook have both not moved
			//2. No piece is attacking between the the rook and the king
			//3. The king is not in check
			if (!((history[historyIndex].entry & oo_mask(Us)) | ((all | danger) & oo_blockers_mask(Us)))) {
				*list++ = Us == WHITE ? Move(e1, g1, OO) : Move(e8, g8, OO);
				//Move m = WHITE ? Move(e1, h1, OO) : Move(e8, h8, OO);
				//std::cout << m.ToUci() << endl;
			}
			if (!((history[historyIndex].entry & ooo_mask(Us)) |
				((all | (danger & ~ignore_ooo_danger(Us))) & ooo_blockers_mask(Us))))
				*list++ = Us == WHITE ? Move(e1, c1, OOO) : Move(e8, c8, OOO);
		}
		//For each pinned rook, bishop or queen...
		b1 = ~(not_pinned | bitboard_of(Us, KNIGHT));
		while (b1) {
			s = pop_lsb(&b1);

			//...only include attacks that are aligned with our king, since pinned pieces
			//are constrained to move in this direction only
			b2 = attacks(type_of(board[s]), s, all) & LINE[our_king][s];
			if (quiet)
				list = make<QUIET>(s, b2 & quiet_mask, list);
			list = make<CAPTURE>(s, b2 & capture_mask, list);
		}

		//For each pinned pawn...
		b1 = ~not_pinned & bitboard_of(Us, PAWN);
		while (b1) {
			s = pop_lsb(&b1);

			if (rank_of(s) == RelativeRank(Us, RANK7)) {
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
						MASK_RANK[RelativeRank(Us, RANK3)]) & ~all & LINE[our_king][s];
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
	b1 = bitboard_of(Us, PAWN) & not_pinned & ~MASK_RANK[RelativeRank(Us, RANK7)];

	if (quiet) {
		//Single pawn pushes
		b2 = Shift(RelativeDir(Us, NORTH), b1) & ~all;

		//Double pawn pushes (only pawns on rank 3/6 are eligible)
		b3 = Shift(RelativeDir(Us, NORTH), b2 & MASK_RANK[RelativeRank(Us, RANK3)]) & quiet_mask;

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
	b1 = bitboard_of(Us, PAWN) & not_pinned & MASK_RANK[RelativeRank(Us, RANK7)];
	if (b1) {
		//Quiet promotions
		b2 = Shift(RelativeDir(Us, NORTH), b1) & quiet_mask;
		while (b2) {
			s = pop_lsb(&b2);
			//One move is added for each promotion piece
			*list++ = Move(s - RelativeDir(Us, NORTH), s, PR_KNIGHT);
			*list++ = Move(s - RelativeDir(Us, NORTH), s, PR_BISHOP);
			*list++ = Move(s - RelativeDir(Us, NORTH), s, PR_ROOK);
			*list++ = Move(s - RelativeDir(Us, NORTH), s, PR_QUEEN);
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

CMoveList GetMoveList() {
	return CMoveList(position);
}

CMoveList GetMoveList(Color color) {
	return CMoveList(position, color);
}

