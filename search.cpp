#include "search.h"

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
	if (chronos.gameOver)
		return true;
	chronos.gameOver = (chronos.flags & FMOVETIME) && (sd.Ms() > chronos.movetime);
	return chronos.gameOver;
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
	for (int n = list.size() - 1; n >= 0; n--) {
		Move move = list[n];
		pv = " " + move.ToUci() + pv;
		g_pos.UnmakeMove(move);
	}
	g_pos.UnmakeMove(sd.bestMove);
	return sd.bestMove.ToUci() + pv;
}

//show depth score and next best move in principal variation
static void ShowInfoPv() {
	if (chronos.ponder || !chronos.post)
		return;
	U64 ms = sd.Ms();
	U64 nps = ms ? (sd.nodes * 1000) / ms : 0;
	string score = sd.bestScore > VALUE_MATE_IN ? "mate " + to_string((VALUE_MATE - sd.bestScore + 1) >> 1) :
		sd.bestScore < VALUE_MATED_IN ? "mate " + to_string((-VALUE_MATE - sd.bestScore) >> 1) :
		"cp " + to_string(sd.bestScore);
	string pv = ExtractPv();
	//cout << "info string phase " << g_pos.phase << endl;
	cout << "info time " << ms << " depth " << sd.depth << " multipv " << sd.multiPV << " score " << score << " nps " << nps << " nodes " << sd.nodes << " hashfull " << tt.Permill() << " pv " << pv << endl;
}

static void ShowBestMove() {
	if (chronos.ponder || !chronos.post)
		return;
	//ShowInfoPv();
	U64 proNode = sd.nodes ? ((sd.nodes - sd.nodesq) * 100) / sd.nodes : 0;
	int proMove = sd.moveSet ? (sd.moveOk * 100) / sd.moveSet : 0;
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
Value QSearch(Position& pos, Stack* ss, Value alpha, Value beta, Depth depth = DEPTH_ZERO) {
	if (!(++sd.nodes & 0x1ffff))
		CheckTime();
	if (chronos.gameOver)
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
				if (!pvNode
					&& ttE->depth >= depth
					//&& ttValue != VALUE_NONE
					//&& (ttValue >= beta ? (ttE->bound & BOUND_LOWER) : (ttE->bound & BOUND_UPPER))
					)
					//return ttValue;
					/*if (ttE->bound & BOUND_UPPER) {
						if (ttE->score >= beta)
							return ttValue;
					}
					else if (ttE->bound & BOUND_LOWER)
						if (ttE->score <= alpha)
							return ttValue;*/
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
		//PickerE pe = n < picker.best ? picker.pList[n] : picker.Pick(n);
		PickerE pe = picker.Pick(n);
		Move m = pe.move;
		if ((bestMove != MOVE_NONE) && (pe.value < 0))break;
		pos.MakeMove(m);
		value = -QSearch<nt>(pos, ss + 1, -beta, -alpha, depth - ONE_PLY);
		pos.UnmakeMove(m);
		//if (chronos.gameOver)return alpha;
		if (value > bestValue) {
			bestValue = value;
			bestMove = m;
			if (value >= beta)
				break;
			if (value > alpha)
				alpha = value;
		}
	}
	Bound rt = bestValue <= oldAlpha ? BOUND_LOWER : bestValue >= beta ? BOUND_UPPER : BOUND_EXACT;
	tt.SetRec(hash, bestValue, bestMove, rt, DEPTH_ZERO);
	return bestValue;
}

//Main search loop
template <NodeType nt>
static Value Search(Position& pos, Stack* ss, Depth depth, Value alpha, Value beta, bool doNull = true) {
	// Step 1.
	if ((pos.move50 >= 100) || (pos.IsRepetition()))
		return VALUE_ZERO;
	Value value;
	bool inCheck = pos.InCheck();
	if (inCheck)// && depth<ONE_PLY)
		++depth;
	if (depth < ONE_PLY)
		return QSearch<nt>(pos, ss, alpha, beta);
	if (!(++sd.nodes & 0x1ffff))
		CheckTime();
	if (chronos.gameOver)
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
	picker.SetBest(ss->killers[0]);
	picker.SetBest(ss->killers[1]);
	//picker.best = sd.multiPV-1;
	int td = depth + ss->ply;

	bool improving = ss->staticEval >= (ss - 2)->staticEval || (ss - 2)->staticEval == VALUE_NONE;
	if (!inCheck) {

		//razoring
		if (depth < 2 * ONE_PLY
			&& alpha - staticEval >(RAZOR_MARGIN * depth) / td)
			return QSearch<nt>(pos, ss, alpha, beta);

		//futility pruning
		if (depth < 7 * ONE_PLY
			&& staticEval - beta >(FUTILITY_MARGIN * depth) / td
			&& staticEval - futility_margin(depth, improving) >= beta
			&& staticEval < VALUE_KNOWN_WIN) // Do not return unproven wins
			return staticEval;

		//eval pruning
		/*if (depth < 3
			&& !pvNode
			&& !inCheck
			&& abs(beta - 1) > VALUE_MATED_IN)
		{
			int eval_margin = 120 * depth;
			if (staticEval - eval_margin >= beta)
				return staticEval - eval_margin;
		}*/

		//null move pruning
		if (doNull
			&& (ss - 1)->staticEval < (NULL_MARGIN * ss->ply) / td
			&& staticEval >= beta
			&& pos.NotOnlyPawns()) {
			pos.MakeNull();
			Depth R = ((NULL_MARGIN + 67 * depth / ONE_PLY) / 256 + std::min(int(staticEval - beta) / 200, 3)) * ONE_PLY;
			Value nullValue = -Search<NONPV>(pos, ss + 1, depth - R, -beta, -beta + 1, false);
			pos.UnmakeNull();
			if (nullValue >= beta)
			{
				if (nullValue >= VALUE_MATE_IN)
					nullValue = beta;

				if (abs(beta) < VALUE_KNOWN_WIN && depth < 12 * ONE_PLY)
					return nullValue;

				Value v = Search<NONPV>(pos, ss + 1, depth - R, -beta, -beta + 1);

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
		//PickerE pe = n < picker.best ? picker.pList[n] : picker.Pick(n);
		PickerE pe = picker.Pick(n);
		Move m = pe.move;
		bool moveCountPruning = depth < 16 * ONE_PLY && n >= FutilityMoveCounts[improving][depth / ONE_PLY];
		if (prune && moveCountPruning && m.IsQuiet())
			continue;
		pos.MakeMove(m);
		if (!n)
			value = -Search<PV>(pos, ss + 1, depth - ONE_PLY, -beta, -alpha);
		else {
			Depth r = DEPTH_ZERO;
			if (pe.value < 0)++r;
			if (nt == NONPV)++r;
			if (m.IsQuiet())++r;
			//if (pos.move50)++r; else if (r)--r;
			//if (pos.move50)++r;
			if (r && pos.InCheck())--r;
			if (r && improving)--r;
			r += reduction<nt>(improving, depth, n);
			value = -Search<NONPV>(pos, ss + 1, depth - ONE_PLY - r, -alpha - 1, -alpha);
			if (r && value > alpha)
				value = -Search<NONPV>(pos, ss + 1, depth - ONE_PLY, -alpha - 1, -alpha);
			if (value > alpha && value < beta)
				value = -Search<PV>(pos, ss + 1, depth - ONE_PLY, -beta, -alpha);
		}
		pos.UnmakeMove(m);
		if (chronos.gameOver)
			return alpha;
		if (value > bestValue) {
			bestMove = m;
			bestValue = value;
			ss->staticValue = value;
			if (value >= beta)
			{
				if (m.IsQuiet())
					UpdateQuietStats(ss, m);
				break;
			}
			if (value > alpha)
				alpha = value;
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
			value = -Search<PV>(pos, ss + 1, depth - ONE_PLY, -beta, -alpha);
		else {
			value = -Search<NONPV>(pos, ss + 1, depth - ONE_PLY, -alpha - 1, -alpha);
			if (value > alpha)
				value = -Search<PV>(pos, ss + 1, depth - ONE_PLY, -beta, -alpha);
		}
		pos.UnmakeMove(move);
		if (bestValue < value) {

		}
		if (chronos.gameOver)
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
	if (chronos.rootMoves.size()) {
		for (Move m : chronos.rootMoves)
			picker.SetBest(m);
		if (picker.best) {
			picker.count = picker.best;
			picker.best = 0;
		}
	}
	sd.depth = ONE_PLY;
	sd.multiPV = 1;
	Value score = Search<PV>(g_pos, ss, ONE_PLY, -VALUE_MATE, VALUE_MATE);
	while (!chronos.gameOver) {
		score = SearchWiden(g_pos, ss, picker, sd.depth, score);
		if (sd.bestMove != MOVE_NONE) {
			picker.best = sd.multiPV - 1;
			picker.SetBest(sd.bestMove);
		}

		/*for (int i1 = 0; i1<picker.count; i1++)
			for(int i2=i1+1;i2<picker.count;i2++)
				if (picker.pList[i1].move == picker.pList[i2].move)
					cout << "duplicate move " << picker.pList[i1].move.ToUci() << endl;*/

		sd.multiPV++;
		if ((sd.multiPV > options.multiPV) || (sd.multiPV > picker.count)) {
			//for(int i=0;i<sd.multiPV;i++)cout << "move " << (i+1) << " " << picker.pList[i].move.ToUci() << endl;
			sd.multiPV = 1;
			if (++sd.depth > MAX_PLY)
				break;
		}
		if (chronos.flags & FMOVETIME)
			if (sd.Ms() > chronos.movetime / 2)
				break;
		if (chronos.flags & FDEPTH)
			if (sd.depth > chronos.depth)
				break;
		if (chronos.flags & FNODES)
			if (sd.nodes >= chronos.nodes)
				break;
		if (abs(score) > VALUE_MATE_IN)
			break;
		if ((picker.count == 1) && (sd.depth > 8))
			break;
	}
	ShowBestMove();
}