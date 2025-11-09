#include "search.h"

int RFP_MARGIN;
int FUTILITY_MARGIN;
int RAZOR_MARGIN;
int NULL_MARGIN;
int LMR_MARGIN;
int ASPIRATION;

int FutilityMoveCounts[2][16]; // [improving][depth]
int Reductions[2][2][64][64];  // [pv][improving][depth][moveNumber]

template <bool PvNode> Depth reduction(bool i, Depth d, int mn) {
	return Reductions[PvNode][i][std::min(d / ONE_PLY, 63)][std::min(mn, 63)] * ONE_PLY;
}

struct Stack {
	int ply = 0;
	Move move = MOVE_NONE;
	Move killers[2] = {};
	Value staticEval = VALUE_ZERO;
	Value staticValue = VALUE_ZERO;
};

sSearchDriver sd;

void InitSearch() {
	RFP_MARGIN = options.rfp;
	FUTILITY_MARGIN = options.futility;
	RAZOR_MARGIN = options.razoring;
	NULL_MARGIN = options.nullMove;
	LMR_MARGIN = options.lmr;
	ASPIRATION = options.aspiration;
	tt.Resize(options.hash);

	for (int imp = 0; imp <= 1; ++imp)
		for (int d = 1; d < 64; ++d)
			for (int mc = 1; mc < 64; ++mc)
			{
				double r = log(d) * log(mc) / (LMR_MARGIN / 100.0);

				Reductions[NONPV][imp][d][mc] = int(std::round(r));
				Reductions[PV][imp][d][mc] = std::max(Reductions[NONPV][imp][d][mc] - 1, 0);

				// Increase reduction for non-PV nodes when eval is not improving
				if (!imp && r > 1.0)
					Reductions[NONPV][imp][d][mc]++;
			}

	for (int d = 0; d < 16; ++d)
	{
		FutilityMoveCounts[0][d] = int(2.4 + 0.74 * pow(d, 1.78));
		FutilityMoveCounts[1][d] = int(5.0 + 1.00 * pow(d, 2.00));
	}

}


//Check for time
static bool CheckTime() {
	string input;
	if (GetInput(input))
		UciCommand(input);
	if (info.gameOver)
		return true;
	info.gameOver = (info.flags & FMOVETIME) && (sd.Ms() > info.movetime);
	return info.gameOver;
}

static void ExtractPv(vector<Move>& list) {
	Hash hash = g_pos.GetHash();
	CRec* rec = tt.GetRec(hash);
	if (!rec)
		return;
	Move move = rec->move;
	if (!g_pos.IsLegal(move))
		return;
	g_pos.MakeMove(move);
	if (g_pos.IsRepetition()) {
		g_pos.UnmakeMove(move);
		return;
	}
	list.push_back(move);
	ExtractPv(list);
}

static string ExtractPv() {
	g_pos.MakeMove(sd.bestMove);
	string pv = "";
	vector<Move> list;
	ExtractPv(list);
	sd.ponderMove = list.size() ? list[0] : MOVE_NONE;
	for (int n = (int)list.size() - 1; n >= 0; n--) {
		Move move = list[n];
		pv = " " + move.ToUci() + pv;
		g_pos.UnmakeMove(move);
	}
	g_pos.UnmakeMove(sd.bestMove);
	return sd.bestMove.ToUci() + pv;
}

//show depth score and next best move in principal variation
static void ShowInfoPv() {
	if (info.ponder || !info.post)
		return;
	U64 ms = sd.Ms();
	U64 nps = ms ? (sd.nodes * 1000) / ms : 0;
	string score = sd.bestScore > VALUE_MATE_IN ? "mate " + to_string((VALUE_MATE - sd.bestScore + 1) >> 1) :
		sd.bestScore < VALUE_MATED_IN ? "mate " + to_string((-VALUE_MATE - sd.bestScore) >> 1) :
		"cp " + to_string(ValueToCp(sd.bestScore));
	string pv = ExtractPv();
	cout << "info time " << ms << " depth " << sd.depth << " multipv " << sd.multiPV << " score " << score << " nps " << nps << " nodes " << sd.nodes << " hashfull " << tt.Permill() << " pv " << pv << endl;
}

static void ShowBestMove() {
	if (info.ponder || !info.post)
		return;
	//ShowInfoPv();
	U64 proNode = sd.nodes ? ((sd.nodes - sd.nodesq) * 100) / sd.nodes : 0;
	int proMove = sd.moveSetTry ? (sd.moveOk * 100) / sd.moveSetTry : 0;
	cout << "info string quiesce " << proNode << '%' << " hash " << proMove << '%' << endl;
	cout << "bestmove " << sd.bestMove.ToUci();
	if (options.ponder && sd.ponderMove.move)
		cout << " ponder " << sd.ponderMove.ToUci();
	cout << endl;
}

static Value futility_margin(Depth d, bool improving) {
	return Value((175 - 50 * improving) * d / ONE_PLY);
}

static void UpdateQuietStats(Stack* ss, Move move) {
	if (ss->killers[0] != move)
	{
		ss->killers[1] = ss->killers[0];
		ss->killers[0] = move;
	}
}

//Quiesce search
template <NodeType nt>
Value SearchQuiescence(Position& pos, Stack* ss, Value alpha, Value beta) {
	if (!(++sd.nodes & 0xffff))
		CheckTime();
	if (info.gameOver)
		return VALUE_ZERO;
	sd.nodesq++;
	(ss + 1)->ply = ss->ply + 1;
	Value staticEval = ss->staticEval = Eval();
	Value value = staticEval;
	Value bestValue = -VALUE_INFINITE;
	//Value bestValue = staticEval;
	Move bestMove = MOVE_NONE;
	if (value >= beta)
		return beta;
	if (value > alpha)
		alpha = value;
	Color color = pos.ColorUs();
	Picker picker;
	pos.MoveList(color, picker.mList, picker.count, false);
	if (!picker.count)
		return alpha;
	picker.Fill();
	constexpr bool pvNode = nt == PV;
	bool inCheck = pos.InCheck();
	bool ttHit = false;
	Value ttValue = VALUE_NONE;
	Move ttMove = MOVE_NONE;
	Hash hash = pos.GetHash();
	if (inCheck)
	{
		staticEval = ss->staticEval = -VALUE_INFINITE;
	}
	else
	{
		CRec* ttE = tt.GetRec(hash);
		if (ttE) {
			ttMove = ttE->move;
			ttValue = ttE->GetValue();
			if (picker.SetBest(ttMove)) {
				ttHit = true;
				if (!pvNode)
					if (ttE->bound == BOUND_EXACT)
						return ttE->GetValue();
					else if (ttE->bound == BOUND_UPPER) {
						if (ttE->score >= beta) {
							if (ttMove.IsQuiet())UpdateQuietStats(ss, ttMove);
							return ttValue;
						}
					}
					else if (ttE->bound == BOUND_LOWER)
						if (ttE->score <= alpha)
							return ttValue;
				if ((ttE->bound == BOUND_EXACT) ||
					(ttE->bound == BOUND_UPPER && staticEval < ttE->score) ||
					(ttE->bound == BOUND_LOWER && staticEval > ttE->score))
					staticEval = ttValue;

			}
		}
	}
	Value oldAlpha = alpha;

	for (int n = 0; n < picker.count; n++)
	{
		PickerE pe = picker.Pick(n);
		Move m = pe.move;
		if ((bestMove != MOVE_NONE) && (pe.value < 0))break;
		pos.MakeMove(m);
		value = -SearchQuiescence<nt>(pos, ss + 1, -beta, -alpha);
		pos.UnmakeMove(m);
		if (value > bestValue) {
			bestValue = value;
			bestMove = m;
			if (value > alpha)
				alpha = value;
			if (value >= beta)
				break;
		}
	}
	Bound rt = bestValue <= oldAlpha ? BOUND_LOWER : bestValue >= beta ? BOUND_UPPER : BOUND_EXACT;
	tt.SetRec(hash, bestValue, bestMove, rt, DEPTH_ZERO);
	return bestValue;
}

//Main search loop
template <NodeType nt>
static Value SearchAlpha(Position& pos, Stack* ss, Depth depth, Value alpha, Value beta, bool doNull = true) {
	// Step 1.
	if ((pos.move50 >= 100) || (pos.IsRepetition()))
		return VALUE_ZERO;
	Value value;
	bool inCheck = pos.InCheck();
	if (inCheck)
		++depth;
	if (depth < ONE_PLY)
		return SearchQuiescence<nt>(pos, ss, alpha, beta);
	if (!(++sd.nodes & 0xffff))
		CheckTime();
	if (info.gameOver)
		return VALUE_ZERO;
	//mate distance pruning
	Value  mate_value = VALUE_MATE - ss->ply;
	if (alpha < -mate_value) alpha = -mate_value;
	if (beta > mate_value - 1) beta = mate_value - 1;
	if (alpha >= beta) return alpha;
	constexpr bool pvNode = nt == PV;

	Value staticEval = ss->staticEval = Eval();
	ss->move = MOVE_NONE;
	(ss + 1)->ply = ss->ply + 1;
	(ss + 2)->staticValue = VALUE_ZERO;
	(ss + 2)->killers[0].move = (ss + 2)->killers[1].move = MOVE_NONE;

	Picker picker;
	pos.MoveList(pos.ColorUs(), picker.mList, picker.count);
	if (!picker.count)
		return inCheck ? -VALUE_MATE + ss->ply : VALUE_ZERO;
	picker.Fill();
	bool ttHit = false;
	Move ttMove = MOVE_NONE;
	Hash hash = pos.GetHash();
	CRec* ttE = tt.GetRec(hash);
	if (ttE) {
		Move ttMove = ttE->move;
		Value ttValue = ttE->GetValue();
		if (picker.SetBest(ttMove)) {
			ttHit = true;
			if (!pvNode && ttE->depth >= depth)
				if (ttE->bound & BOUND_UPPER) {
					if (ttValue >= beta) {
						if (ttMove.IsQuiet())
							UpdateQuietStats(ss, ttMove);
						return ttValue;
					}
				}
				else if (ttE->bound & BOUND_LOWER)
					if (ttE->score <= alpha)
						return ttValue;
			if ((ttE->bound == BOUND_EXACT) ||
				(ttE->bound == BOUND_UPPER && staticEval < ttE->score) ||
				(ttE->bound == BOUND_LOWER && staticEval > ttE->score))
				staticEval = ttE->GetValue();

		}
	}
	if (!picker.best)
		depth -= Depth(depth > 3);
	picker.SetBest(ss->killers[0]);
	picker.SetBest(ss->killers[1]);
	int td = depth + ss->ply;

	bool improving = staticEval >= (ss - 2)->staticEval || (ss - 2)->staticEval == VALUE_NONE;
	//bool improving = ss->staticEval >= (ss - 2)->staticEval || ss->staticEval == VALUE_NONE || (ss - 2)->staticEval == VALUE_NONE;

	if (!inCheck) {

		//razoring
		if (depth < 2 * ONE_PLY
			&& alpha - staticEval >(RAZOR_MARGIN * depth) / td)
			return SearchQuiescence<nt>(pos, ss, alpha, beta);

		//futility pruning
		if (depth < 7 * ONE_PLY
			&& staticEval - beta >(FUTILITY_MARGIN * depth) / td
			&& staticEval - futility_margin(depth, improving) >= beta
			&& staticEval < VALUE_KNOWN_WIN) // Do not return unproven wins
			return staticEval;

		//eval pruning
		if (depth < 3
			&& !pvNode
			&& abs(beta - 1) > VALUE_MATED_IN)
		{
			Value eval = staticEval - RFP_MARGIN * depth;
			if (eval >= beta)
				return eval;
		}

		//null move pruning
		if (doNull
			&& (ss - 1)->staticEval < (NULL_MARGIN * ss->ply) / td
			&& staticEval >= beta
			&& pos.NotOnlyPawns()) {
			pos.MakeNull();
			Depth R = ((NULL_MARGIN + 67 * depth / ONE_PLY) / 256 + std::min(int(staticEval - beta) / 200, 3)) * ONE_PLY;
			Value nullValue = -SearchAlpha<NONPV>(pos, ss + 1, depth - R, -beta, -beta + 1, false);
			pos.UnmakeNull();
			if (nullValue >= beta)
			{
				if (nullValue >= VALUE_MATE_IN)
					nullValue = beta;

				if (abs(beta) < VALUE_KNOWN_WIN && depth < 12 * ONE_PLY)
					return nullValue;

				Value v = SearchAlpha<NONPV>(pos, ss + 1, depth - R, -beta, -beta + 1);

				if (v >= beta)
					return nullValue;
			}
		}
	}

	int futility = staticEval + depth * FUTILITY_MARGIN;
	bool prune = !inCheck && (abs(alpha) < VALUE_MATE_IN) && (ttMove == MOVE_NONE) && (futility < alpha);

	Value bestValue = -VALUE_MATE;
	Value oldAlpha = alpha;
	Move bestMove = MOVE_NONE;
	for (int n = 0; n < picker.count; n++)
	{
		PickerE pe = picker.Pick(n);
		Move m = pe.move;
		bool moveCountPruning = depth < 16 * ONE_PLY && n >= FutilityMoveCounts[improving][depth / ONE_PLY];
		if (prune && moveCountPruning && m.IsQuiet())
			continue;
		pos.MakeMove(m);
		if (!n)
			value = -SearchAlpha<PV>(pos, ss + 1, depth - ONE_PLY, -beta, -alpha);
		else {
			Depth r = DEPTH_ZERO;
			if (pe.value < 0)++r;
			if (nt == NONPV)++r;
			if (m.IsQuiet())++r;
			if (r && pos.InCheck())--r;
			if (r && improving)--r;
			r += reduction<nt>(improving, depth, n);
			value = -SearchAlpha<NONPV>(pos, ss + 1, depth - ONE_PLY - r, -alpha - 1, -alpha);
			if (r && value > alpha)
				value = -SearchAlpha<NONPV>(pos, ss + 1, depth - ONE_PLY, -alpha - 1, -alpha);
			if (value > alpha && value < beta)
				value = -SearchAlpha<PV>(pos, ss + 1, depth - ONE_PLY, -beta, -alpha);
		}
		pos.UnmakeMove(m);
		if (info.gameOver)
			return VALUE_ZERO;
		if (value > bestValue) {
			bestMove = m;
			bestValue = value;
			ss->staticValue = value;
			if (value > alpha)
				alpha = value;
			if (value >= beta)
			{
				if (m.IsQuiet())
					UpdateQuietStats(ss, m);
				break;
			}
		}
	}
	Bound rt = bestValue <= oldAlpha ? BOUND_LOWER : bestValue >= beta ? BOUND_UPPER : BOUND_EXACT;
	tt.SetRec(hash, bestValue, bestMove, rt, depth);
	return bestValue;
}

//search first ply
static Value SearchRoot(Position& pos, Stack* ss, Picker& picker, Depth depth, Value alpha, Value beta) {
	Value oldAlpha = alpha;
	Value value;
	Value bestValue = -VALUE_INFINITE;
	Move bestMove = MOVE_NONE;
	picker.best = sd.multiPV;
	ss->move = MOVE_NONE;
	ss->staticEval = VALUE_ZERO;
	(ss + 1)->ply = ss->ply + 1;
	(ss + 2)->staticValue = VALUE_ZERO;
	(ss + 2)->killers[0].move = (ss + 2)->killers[1].move = MOVE_NONE;
	for (int n = sd.multiPV - 1; n < picker.count; n++) {
		PickerE pe = n < picker.best ? picker.pList[n] : picker.Pick(n);
		Move move = pe.move;
		pos.MakeMove(move);
		if (bestMove == MOVE_NONE)
			value = -SearchAlpha<PV>(pos, ss + 1, depth - ONE_PLY, -beta, -alpha);
		else {
			value = -SearchAlpha<NONPV>(pos, ss + 1, depth - ONE_PLY, -alpha - 1, -alpha);
			if (value > alpha)
				value = -SearchAlpha<PV>(pos, ss + 1, depth - ONE_PLY, -beta, -alpha);
		}
		pos.UnmakeMove(move);
		if (bestValue < value) {

		}
		if (info.gameOver)
			return alpha;
		if (bestValue < value) {
			bestValue = value;
			bestMove = move;
			ss->staticValue = value;
			sd.bestScore = value;
			sd.bestMove = move;
			ShowInfoPv();
			if (value >= beta)
				break;
			alpha = value;
		}
	}
	Bound rt = bestValue <= oldAlpha ? BOUND_LOWER : bestValue >= beta ? BOUND_UPPER : BOUND_EXACT;
	tt.SetRec(pos.GetHash(), bestValue, bestMove, rt, depth);
	return bestValue;
}


static Value SearchWiden(Position& pos, Stack* ss, Picker& picker, Depth depth, Value score) {
	Value alpha = score - ASPIRATION;
	Value beta = score + ASPIRATION;
	score = SearchRoot(pos, ss, picker, depth, alpha, beta);
	if (score <= alpha || score >= beta)
		score = SearchRoot(pos, ss, picker, depth, -VALUE_MATE, VALUE_MATE);
	return score;
}

//start search
void SearchIterate() {
	bool inCheck = g_pos.InCheck();
	sd.Restart();
	tt.age++;
	Picker picker;
	g_pos.MoveList(g_pos.ColorUs(), picker.mList, picker.count);
	if (!picker.count) {
		cout << "info string no moves" << endl;
		return;
	}
	Stack stack[MAX_PLY + 7] = {};
	Stack* ss = stack + 4;
	(ss + 1)->ply = 1;
	std::memset(ss - 4, 0, 7 * sizeof(Stack));
	picker.Fill();
	if (info.rootMoves.size()) {
		for (Move m : info.rootMoves)
			picker.SetBest(m);
		if (picker.best) {
			picker.count = picker.best;
			picker.best = 0;
		}
	}
	sd.depth = ONE_PLY;
	sd.multiPV = 1;
	Value score = SearchAlpha<PV>(g_pos, ss, ONE_PLY, -VALUE_MATE, VALUE_MATE);
	while (!info.gameOver) {
		score = SearchWiden(g_pos, ss, picker, sd.depth, score);
		if (sd.bestMove != MOVE_NONE) {
			picker.best = sd.multiPV - 1;
			picker.SetBest(sd.bestMove);
		}

		sd.multiPV++;
		if ((sd.multiPV > options.multiPV) || (sd.multiPV > picker.count)) {
			sd.multiPV = 1;
			if (++sd.depth > MAX_PLY)
				break;
		}
		if (info.flags & FMOVETIME)
			if (sd.Ms() > info.movetime / 2)
				break;
		if (info.flags & FDEPTH)
			if (sd.depth > info.depth)
				break;
		if (info.flags & FNODES)
			if (sd.nodes >= info.nodes)
				break;
		if (abs(score) > VALUE_MATE_IN)
			break;
		if ((picker.count == 1) && (sd.depth > 8))
			break;
	}
	ShowBestMove();
}