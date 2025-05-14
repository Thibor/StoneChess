#include "position.h"

#include <sstream>

#include "tables.h"

Position g_pos;

U64 zobrist::hashColor;
//Zobrist keys for each piece and each square
//Used to incrementally update the hash key of a position
U64 zobrist::zobrist_table[PIECE_NB][SQUARE_NB];

//Initializes the zobrist table with random 64-bit numbers
void zobrist::InitialiseZobristKeys() {
	PRNG rng(70026072);
	hashColor = rng.rand<U64>();
	for (int i = 0; i < PIECE_NB; i++)
		for (int j = 0; j < SQUARE_NB; j++)
			zobrist::zobrist_table[i][j] = rng.rand<U64>();
}

//Pretty-prints the position (including FEN and hash key)
void Position::PrintBoard() const {
	const char* s = "   +---+---+---+---+---+---+---+---+\n";
	const char* t = "     A   B   C   D   E   F   G   H\n";
	cout << t;
	for (int i = 56; i >= 0; i -= 8) {
		cout << s << " " << i / 8 + 1 << " ";
		for (int j = 0; j < 8; j++) {
			Piece piece = board[i + j];
			cout << "| " << PIECE_PRT[piece] << " ";
		}
		cout << "| " << i / 8 + 1 << endl;
	}
	cout << s;
	cout << t << endl;
	cout << "FEN: " << GetFen() << endl;
	cout << "Hash: 0x" << std::hex << hash << std::dec << endl;
	cout << "Side: " << (color == WHITE ? "w" : "b") << endl;
}

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

constexpr Position::Position() : piece_bb{ 0 }, color(WHITE), historyIndex(0), board{}, hash(0), pinned(0), checkers(0),move50(0) {
	//Sets all squares on the board as empty
	for (int i = 0; i < 64; i++)
		board[i] = NO_PIECE;
	history[0] = UndoInfo();
}

void Position::Clear() {
	color = WHITE;
	historyIndex = 0;
	hash = 0;
	pinned = 0;
	checkers = 0;
	for (int n = 0; n < SQUARE_NB; n++)
		board[n] = NO_PIECE;
	for (int n = 0; n < PIECE_NB; n++)
		piece_bb[n] = 0;
	history[0] = UndoInfo();
};

void Position::MoveList(Color color, Move* list, int& count, bool quiet) {
	Move* last = GenerateMoves(color, list, quiet);
	count = last - list;
}

void Position::MoveList(Move* list, int& count, bool quiet) {
	Move* last = GenerateMoves(ColorUs(), list);
	count = last - list;
}

bool Position::IsLegal(Move move) {
	Move list[218];
	GenerateMoves(color, list);
	for (Move m : list)
		if (m == move)
			return true;
	return false;
}

/*bool Position::InCheck(Color c) const {
	return AttackersFrom(~c, bsf(bitboard_of(c, KING)), AllPieces());
}*/

bool Position::InCheck() const {
	return AttackersFrom(~color, bsf(bitboard_of(color, KING)), AllPieces());
}

inline void Position::PutPiece(Piece pc, Square s) {
	board[s] = pc;
	piece_bb[pc] |= SQUARE_BB[s];
	hash ^= zobrist::zobrist_table[pc][s];
}

/*void Position::Phase() {
	phase= PopCount(piece_bb[WHITE_KNIGHT] | piece_bb[BLACK_KNIGHT] | piece_bb[WHITE_BISHOP] | piece_bb[BLACK_BISHOP]) +
		PopCount(piece_bb[WHITE_ROOK] | piece_bb[BLACK_ROOK]) * 2 +
		SparsePopCount(piece_bb[WHITE_QUEEN] | piece_bb[BLACK_QUEEN]) * 4;
	if (phase > 24)
		phase = 24;
}*/

inline void Position::RemovePiece(Square s) {
	Piece pc = board[s];
	hash ^= zobrist::zobrist_table[pc][s];
	piece_bb[board[s]] &= ~SQUARE_BB[s];
	board[s] = NO_PIECE;
}

bool Position::IsRepetition() const{
	for (int n = historyIndex - 2; n >= historyIndex - move50; n -= 2)
		if (n >= 0)
			if (history[n].hash == hash)
				return true;
	return false;
}

void Position::MakeNull() {
	hash ^= zobrist::hashColor;
	color = ~color;
	++historyIndex;
	history[historyIndex] = UndoInfo(history[historyIndex - 1]);
	history[historyIndex].epsq = SQ_NONE;
}

void Position::UnmakeNull() {
	hash ^= zobrist::hashColor;
	color = ~color;
	--historyIndex;
}

//Plays a move in the position
void Position::MakeMove(const Move m) {
	history[historyIndex].move50 = move50;
	history[historyIndex].hash = hash;
	hash ^= zobrist::hashColor;
	Color c = color;
	color = ~color;
	++historyIndex;
	history[historyIndex] = UndoInfo(history[historyIndex - 1]);
	Square fr = m.From();
	Square to = m.To();
	MoveFlags type = m.Flags();
	history[historyIndex].entry |= SQUARE_BB[to] | SQUARE_BB[fr];
	move50 = (type == QUIET) && (TypeOf(board[fr]) != PAWN) ? ++move50 : 0;
	switch (type) {
	case QUIET:
		//The to square is guaranteed to be empty here
		MovePieceQuiet(m.From(), m.To());
		break;
	case DOUBLE_PUSH:
		//The to square is guaranteed to be empty here
		MovePieceQuiet(m.From(), m.To());

		//This is the square behind the pawn that was double-pushed
		history[historyIndex].epsq = m.From() + RelativeDir(c, NORTH);
		break;
	case OO:
		if (c == WHITE) {
			MovePieceQuiet (SQ_E1,SQ_G1);
			MovePieceQuiet(SQ_H1,SQ_F1);
		}
		else {
			MovePieceQuiet(SQ_E8,SQ_G8);
			MovePieceQuiet(SQ_H8,SQ_F8);
		}
		break;
	case OOO:
		if (c == WHITE) {
			MovePieceQuiet(SQ_E1,SQ_C1);
			MovePieceQuiet(SQ_A1,SQ_D1);
		}
		else {
			MovePieceQuiet(SQ_E8,SQ_C8);
			MovePieceQuiet(SQ_A8,SQ_D8);
		}
		break;
	case EN_PASSANT:
		MovePieceQuiet(m.From(), m.To());
		RemovePiece(m.To() + RelativeDir(c, SOUTH));
		break;
	case PR_KNIGHT:
		RemovePiece(m.From());
		PutPiece(MakePiece(c, KNIGHT), m.To());
		break;
	case PR_BISHOP:
		RemovePiece(m.From());
		PutPiece(MakePiece(c, BISHOP), m.To());
		break;
	case PR_ROOK:
		move50 = 0;
		RemovePiece(m.From());
		PutPiece(MakePiece(c, ROOK), m.To());
		break;
	case PR_QUEEN:
		RemovePiece(m.From());
		PutPiece(MakePiece(c, QUEEN), m.To());
		break;
	case PC_KNIGHT:
		RemovePiece(m.From());
		history[historyIndex].captured = board[m.To()];
		RemovePiece(m.To());
		PutPiece(MakePiece(c, KNIGHT), m.To());
		break;
	case PC_BISHOP:
		RemovePiece(m.From());
		history[historyIndex].captured = board[m.To()];
		RemovePiece(m.To());
		PutPiece(MakePiece(c, BISHOP), m.To());
		break;
	case PC_ROOK:
		RemovePiece(m.From());
		history[historyIndex].captured = board[m.To()];
		RemovePiece(m.To());
		PutPiece(MakePiece(c, ROOK), m.To());
		break;
	case PC_QUEEN:
		RemovePiece(m.From());
		history[historyIndex].captured = board[m.To()];
		RemovePiece(m.To());
		PutPiece(MakePiece(c, QUEEN), m.To());
		break;
	case CAPTURE:
		history[historyIndex].captured = board[m.To()];
		MovePiece(m.From(), m.To());
		break;
	}
}

//Undos a move in the current position, rolling it back to the previous position
void Position::UnmakeMove(const Move m) {
	//hash ^= zobrist::hashColor;
	color = ~color;
	Color c = color;
	MoveFlags type = m.Flags();
	switch (type) {
	case QUIET:
		MovePieceQuiet(m.To(), m.From());
		break;
	case DOUBLE_PUSH:
		MovePieceQuiet(m.To(), m.From());
		break;
	case OO:
		if (c == WHITE) {
			MovePieceQuiet(SQ_G1,SQ_E1);
			MovePieceQuiet(SQ_F1,SQ_H1);
		}
		else {
			MovePieceQuiet(SQ_G8,SQ_E8);
			MovePieceQuiet(SQ_F8,SQ_H8);
		}
		break;
	case OOO:
		if (c == WHITE) {
			MovePieceQuiet(SQ_C1,SQ_E1);
			MovePieceQuiet(SQ_D1,SQ_A1);
		}
		else {
			MovePieceQuiet(SQ_C8,SQ_E8);
			MovePieceQuiet(SQ_D8,SQ_A8);
		}
		break;
	case EN_PASSANT:
		MovePieceQuiet(m.To(), m.From());
		PutPiece(MakePiece(~c, PAWN), m.To() + RelativeDir(c, SOUTH));
		break;
	case PR_KNIGHT:
	case PR_BISHOP:
	case PR_ROOK:
	case PR_QUEEN:
		RemovePiece(m.To());
		PutPiece(MakePiece(c, PAWN), m.From());
		break;
	case PC_KNIGHT:
	case PC_BISHOP:
	case PC_ROOK:
	case PC_QUEEN:
		RemovePiece(m.To());
		PutPiece(MakePiece(c, PAWN), m.From());
		PutPiece(history[historyIndex].captured, m.To());
		break;
	case CAPTURE:
		MovePieceQuiet(m.To(), m.From());
		PutPiece(history[historyIndex].captured, m.To());
		break;
	}
	--historyIndex;
	move50 = history[historyIndex].move50;
	hash = history[historyIndex].hash;
}

void Position::SetFen(const std::string& fen) {
	Clear();
	int square = SQ_A8;
	for (char ch : fen.substr(0, fen.find(' '))) {
		if (isdigit(ch))
			square += (ch - '0') * EAST;
		else if (ch == '/')
			square += 2 * SOUTH;
		else
			PutPiece(Piece(PIECE_STR.find(ch)), Square(square++));
	}

	std::istringstream ss(fen.substr(fen.find(' ')));
	unsigned char token;

	ss >> token;
	color = token == 'w' ? WHITE : BLACK;

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
	//cout << "phase= " << phase << endl;
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

	fen << (color == WHITE ? " w " : " b ")
		<< (history[historyIndex].entry & WHITE_OO_MASK ? "" : "K")
		<< (history[historyIndex].entry & WHITE_OOO_MASK ? "" : "Q")
		<< (history[historyIndex].entry & BLACK_OO_MASK ? "" : "k")
		<< (history[historyIndex].entry & BLACK_OOO_MASK ? "" : "q")
		<< (history[historyIndex].entry & ALL_CASTLING_MASK ? "- " : "")
		<< (history[historyIndex].epsq == SQ_NONE ? " -" : SQSTR[history[historyIndex].epsq]);

	return fen.str();
}

//Moves a piece to a (possibly empty) square on the board and updates the hash
void Position::MovePiece(Square from, Square to) {
	hash ^= zobrist::zobrist_table[board[from]][from] ^ zobrist::zobrist_table[board[from]][to] ^ zobrist::zobrist_table[board[to]][to];
	Bitboard mask = SQUARE_BB[from] | SQUARE_BB[to];
	piece_bb[board[from]] ^= mask;
	piece_bb[board[to]] &= ~mask;
	board[to] = board[from];
	board[from] = NO_PIECE;
}

//Moves a piece to an empty square. Note that it is an error if the <to> square contains a piece
void Position::MovePieceQuiet(Square from, Square to) {
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

	switch (SparsePopCount(checkers)) {
	case 2:
		//If there is a double check, the only legal moves are king moves out of check
		return list;
	case 1: {
		//It's a single check!

		Square checker_square = bsf(checkers);
		Piece piece = board[checker_square];
		PieceType pt = TypeOf(piece);
		Color pc = ColorOf(piece);
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
			while (b1) {
				Square sf = pop_lsb(&b1);
				if (TypeOf(board[sf]) == PAWN && RelativeRank(Us, RankOf(sf)) == RANK_7) {
					*list++ = Move(sf, checker_square, PC_KNIGHT);
					*list++ = Move(sf, checker_square, PC_BISHOP);
					*list++ = Move(sf, checker_square, PC_ROOK);
					*list++ = Move(sf, checker_square, PC_QUEEN);
				}
				else
					*list++ = Move(sf, checker_square, CAPTURE);
			}

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

		if (history[historyIndex].epsq != SQ_NONE) {
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
					MASK_RANK[RankOf(our_king)]) &
					their_orth_sliders) == 0)
					*list++ = Move(s, history[historyIndex].epsq, EN_PASSANT);
			}

			//Pinned pawns can only capture e.p. if they are pinned diagonally and the e.p. square is in line with the king 
			b1 = b2 & pinned & LINE[history[historyIndex].epsq][our_king];
			if (b1) {
				*list++ = Move(bsf(b1), history[historyIndex].epsq, EN_PASSANT);
			}
		}

		if (quiet) 
		{
			//Only add castling if:
			//1. The king and the rook have both not moved
			//2. No piece is attacking between the the rook and the king
			//3. The king is not in check
			if (!((history[historyIndex].entry & oo_mask(Us)) | ((all | danger) & oo_blockers_mask(Us)))) {
				*list++ = Us == WHITE ? Move(SQ_E1,SQ_G1, OO) : Move(SQ_E8, SQ_G8, OO);
				//Move m = WHITE ? Move(e1, h1, OO) : Move(e8, h8, OO);
				//std::cout << m.ToUci() << endl;
			}
			if (!((history[historyIndex].entry & ooo_mask(Us)) |
				((all | (danger & ~ignore_ooo_danger(Us))) & ooo_blockers_mask(Us))))
				*list++ = Us == WHITE ? Move(SQ_E1,SQ_C1, OOO) : Move(SQ_E8,SQ_C8, OOO);
		}
		//For each pinned rook, bishop or queen...
		b1 = ~(not_pinned | bitboard_of(Us, KNIGHT));
		while (b1) {
			s = pop_lsb(&b1);

			//...only include attacks that are aligned with our king, since pinned pieces
			//are constrained to move in this direction only
			b2 = attacks(TypeOf(board[s]), s, all) & LINE[our_king][s];
			if (quiet)
				list = make<QUIET>(s, b2 & quiet_mask, list);
			list = make<CAPTURE>(s, b2 & capture_mask, list);
		}

		//For each pinned pawn...
		b1 = ~not_pinned & bitboard_of(Us, PAWN);
		while (b1) {
			s = pop_lsb(&b1);

			if (RankOf(s) == RelativeRank(Us, RANK_7)) {
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
						MASK_RANK[RelativeRank(Us, RANK_3)]) & ~all & LINE[our_king][s];
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
	b1 = bitboard_of(Us, PAWN) & not_pinned & ~MASK_RANK[RelativeRank(Us, RANK_7)];

	if (quiet) 
	{
		//Single pawn pushes
		b2 = Shift(RelativeDir(Us, NORTH), b1) & ~all;

		//Double pawn pushes (only pawns on rank 3/6 are eligible)
		b3 = Shift(RelativeDir(Us, NORTH), b2 & MASK_RANK[RelativeRank(Us, RANK_3)]) & quiet_mask;

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
	b1 = bitboard_of(Us, PAWN) & not_pinned & MASK_RANK[RelativeRank(Us, RANK_7)];
	if (b1) {
		//Quiet promotions
		//if (quiet) {
		b2 = Shift(RelativeDir(Us, NORTH), b1) & quiet_mask;
		while (b2) {
			s = pop_lsb(&b2);
			//One move is added for each promotion piece
			*list++ = Move(s - RelativeDir(Us, NORTH), s, PR_KNIGHT);
			*list++ = Move(s - RelativeDir(Us, NORTH), s, PR_BISHOP);
			*list++ = Move(s - RelativeDir(Us, NORTH), s, PR_ROOK);
			*list++ = Move(s - RelativeDir(Us, NORTH), s, PR_QUEEN);
			//}
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

//Returns a bitboard containing all pieces attacking a particluar square
inline Bitboard Position::Attackers(Square s) const {
	Bitboard occ = AllPieces();
	return color == BLACK ?
		(PawnAttacks(BLACK, s) & piece_bb[WHITE_PAWN]) |
		(attacks<KNIGHT>(s, occ) & piece_bb[WHITE_KNIGHT]) |
		(attacks<BISHOP>(s, occ) & (piece_bb[WHITE_BISHOP] | piece_bb[WHITE_QUEEN])) |
		(attacks<ROOK>(s, occ) & (piece_bb[WHITE_ROOK] | piece_bb[WHITE_QUEEN])) :
		(PawnAttacks(WHITE, s) & piece_bb[BLACK_PAWN]) |
		(attacks<KNIGHT>(s, occ) & piece_bb[BLACK_KNIGHT]) |
		(attacks<BISHOP>(s, occ) & (piece_bb[BLACK_BISHOP] | piece_bb[BLACK_QUEEN])) |
		(attacks<ROOK>(s, occ) & (piece_bb[BLACK_ROOK] | piece_bb[BLACK_QUEEN]));
}

//Returns a bitboard containing all pieces of a given color attacking a particluar square
inline Bitboard Position::AttackersFrom(Color c, Square s, Bitboard occ) const {
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

/*void Position::Phase() {
	phase= PopCount(piece_bb[WHITE_KNIGHT] | piece_bb[BLACK_KNIGHT] | piece_bb[WHITE_BISHOP] | piece_bb[BLACK_BISHOP]) +
		PopCount(piece_bb[WHITE_ROOK] | piece_bb[BLACK_ROOK]) * 2 +
		SparsePopCount(piece_bb[WHITE_QUEEN] | piece_bb[BLACK_QUEEN]) * 4;
	if (phase > 24)
		phase = 24;
}*/


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

