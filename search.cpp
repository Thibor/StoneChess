#include "search.h"

#include "input.h"

#define MAX_DEPTH 100

const Value CHECKMATE_MAX = 0x7ff0;
const Value CHECKMATE_NEAR = 0x7000;
const Value CHECKMATE_INFINITY = 0x7fff;

Value FUTILITY_MARGIN_ORG = 115;
Value RAZOR_MARGIN_ORG = 230;
Value RFP_MARGIN_ORG = 115;
Value NULL_MARGIN_ORG = 4;
Value LMR_MARGiN_ORG = 86;

Value FUTILITY_MARGIN;
Value RAZOR_MARGIN;
Value RFP_MARGIN;
Value NULL_MARGIN;
U32 LMR[256][256];

sSearchDriver sd;

void InitSearch() {
	int d, m;
	for (d = 0; d < 256; d++)
		for (m = 0; m < 256; m++)
			LMR[d][m] = 1.25 + log(d) * log(m) * (LMR_MARGiN_ORG + options.lmr) / 300;
	FUTILITY_MARGIN = FUTILITY_MARGIN_ORG - options.futility;
	RFP_MARGIN = RFP_MARGIN_ORG - options.rfp;
	RAZOR_MARGIN = RAZOR_MARGIN_ORG - options.razoring;
	NULL_MARGIN = NULL_MARGIN_ORG - options.nullMove;
}


//Check for time
bool CheckTime() {
	string input;
	if (GetInput(input))
		UciCommand(input);
	if (chronos.gameOver)
		return true;
	chronos.gameOver = (chronos.flags & FMOVETIME) && (sd.Ms() > chronos.movetime);
	return chronos.gameOver;
}

void ExtractPv(vector<Move>& list) {
	Hash hash = position.GetHash();
	CRec* rec = tt.GetRec(hash);
	if (rec == nullptr)
		return;
	Move move = rec->move;
	if (!position.IsLegal(move))
		return;
	position.MakeMove(move);
	if (position.IsRepetition()) {
		position.UnmakeMove(move);
		return;
	}
	list.push_back(move);
	ExtractPv(list);
}

string ExtractPv() {
	position.MakeMove(sd.bstMove);
	string pv = "";
	vector<Move> list;
	ExtractPv(list);
	for (int n = list.size() - 1; n >= 0; n--) {
		Move move = list[n];
		if (n == 0)
			sd.ponderMove = move;
		string uci = move.ToUci();
		pv = " " + uci + pv;
		position.UnmakeMove(move);
	}
	position.UnmakeMove(sd.bstMove);
	return sd.bstMove.ToUci() + pv;
}

//show depth score and next best move in principal variation
void ShowInfoPv() {
	if (chronos.ponder || !chronos.post)
		return;
	uint64_t ms = sd.Ms();
	uint64_t nps = ms ? (sd.nodes * 1000) / ms : 0;
	string score = sd.bstScore > CHECKMATE_NEAR ? "mate " + to_string((CHECKMATE_MAX - sd.bstScore + 1) >> 1) :
		sd.bstScore < -CHECKMATE_NEAR ? "mate " + to_string((-CHECKMATE_MAX - sd.bstScore) >> 1) :
		"cp " + to_string(sd.bstScore);
	string pv = ExtractPv();
	cout << "info time " << ms << " depth " << sd.depth << " score " << score << " nps " << nps << " nodes " << sd.nodes << " hashfull " << tt.Permill() << " pv " << pv << endl;
}

void ShowBestMove() {
	if (chronos.ponder || !chronos.post)
		return;
	ShowInfoPv();
	U64 proNode = sd.nodes ? ((sd.nodes - sd.nodesq) * 100) / sd.nodes : 0;
	int proMove = sd.moveSet ? (sd.moveOk * 100) / sd.moveSet : 0;
	cout << "info string quiesce " << proNode << '%' << " hash " << proMove << '%' << endl;
	cout << "bestmove " << sd.bstMove.ToUci();
	if (options.ponder && sd.ponderMove.move)
		cout << " ponder " << sd.ponderMove.ToUci();
	cout << endl;
}

//Quiesce search
Value QSearch(Value alpha, Value beta, NodeTypes nt) {
	if (!(++sd.nodes & 0x1ffff))
		CheckTime();
	if (chronos.gameOver)
		return alpha;
	sd.nodesq++;
	Value staticEval = Eval();
	Value score = staticEval;
	if (score >= beta)
		return beta;
	if (score > alpha)
		alpha = score;
	Color color = position.ColorUs();
	Picker picker;
	position.MoveList(color, picker.mList, picker.count, false);
	if (!picker.count)
		return alpha;
	picker.Fill();
	bool inCheck = position.InCheck();
	if (nt == NTNONPV) {
		CRec* rec = tt.GetRec(position.GetHash());
		if (rec != nullptr)
			if (picker.SetMove(rec->move)) {
				if (rec->type == NODE_PV)
					return rec->score;
				else if (rec->type == NODE_BETA) {
					if (rec->score >= beta)
						return beta;
				}
				else if (rec->type == NODE_ALPHA)
					if (rec->score <= alpha)
						return alpha;
				if (!inCheck)
					if ((rec->type == NODE_PV) ||
						(rec->type == NODE_BETA && staticEval < rec->score) ||
						(rec->type == NODE_ALPHA && staticEval > rec->score))
						staticEval = rec->score;
			}
	}
	Value bestScore = -CHECKMATE_MAX;
	U16 bestMove = 0;
	Value oldAlpha = alpha;
	for (int n = 0; n < picker.count; n++)
	{
		PickerE pe = n < picker.index ? picker.pList[n] : picker.Pick(n);
		Move m = pe.move;
		if (bestMove && (pe.see < 0))break;
		position.MakeMove(m);
		score = -QSearch(-beta, -alpha, nt);
		position.UnmakeMove(m);
		if (chronos.gameOver)
			return alpha;
		if (score > bestScore)
		{
			bestScore = score;
			bestMove = m.move;
		}
		if (score > alpha)
			alpha = score;
		if (score >= beta)
			break;
	}
	//if (!chronos.gameOver && bestMove) {
	RecType rt = bestScore <= oldAlpha ? NODE_ALPHA : bestScore >= beta ? NODE_BETA : NODE_PV;
	tt.SetRec(position.GetHash(), bestScore, bestMove, rt, 0);
	//}
	return alpha;
}

//Main search loop
Value Search(Depth depth, Depth ply, Value alpha, Value beta, NodeTypes nt, bool doNull = true) {
	if ((position.move50 >= 100) || (position.IsRepetition()))
		return ply & 1 ? -options.contempt : options.contempt;
	S32 score;
	bool raised_alpha = false;
	Color color = position.ColorUs();
	bool inCheck = position.InCheck();
	if (inCheck)
		depth++;
	if (depth < 1)
		return QSearch(alpha, beta, nt);
	if (!(++sd.nodes & 0x1ffff))
		CheckTime();
	if (chronos.gameOver)
		return alpha;

	//mate distance pruning
	int  mate_value = CHECKMATE_MAX - ply;
	if (alpha < -mate_value) alpha = -mate_value;
	if (beta > mate_value) beta = mate_value;
	if (alpha >= beta) return alpha;

	//trasposition table pruning
	int staticEval = Eval();
	Picker picker;
	position.MoveList(color, picker.mList, picker.count);
	if (!picker.count)return inCheck ? -CHECKMATE_MAX + ply : 0;
	picker.Fill();
	if (stack[ply].killer1.move)
		picker.SetMove(stack[ply].killer1);
	if (stack[ply].killer2.move)
		picker.SetMove(stack[ply].killer2);
	bool        pv = (beta - alpha) != 1;
	int hashMove = 0;
	CRec* rec = tt.GetRec(position.GetHash());
	if (rec != nullptr)
		if (picker.SetMove(rec->move)) {
			hashMove = rec->move;
			if (rec->depth >= depth && !pv)
				if (rec->type == NODE_PV)
					return rec->score;
				else if (rec->type == NODE_BETA) {
					if (rec->score >= beta) {
						if (stack[ply].killer1.move != rec->move && stack[ply].killer2.move != rec->move) {
							stack[ply].killer2.move = stack[ply].killer1.move;
							stack[ply].killer1.move = rec->move;
						}
						return beta;
					}
				}
				else if (rec->type == NODE_ALPHA)
					if (rec->score <= alpha)
						return alpha;
			if (!inCheck)
				if ((rec->type == NODE_PV) ||
					(rec->type == NODE_BETA && staticEval < rec->score) ||
					(rec->type == NODE_ALPHA && staticEval > rec->score))
					staticEval = rec->score;
		}

	/*if (!inCheck && alpha == beta - 1) {

		//razoring
		if (staticEval < alpha - RAZOR_MARGIN * depth) {
			return QSearch(alpha - 1, alpha, nt);
			//if (score < alpha && abs(score) < CHECKMATE_NEAR)return score;
		}

		//rfp
		if ((staticEval >= beta + RFP_MARGIN * depth)
			//&&(depth<8)
			)
			return staticEval;

		//null move pruning
		if (doNull
			//&& (depth > 4)
			&& (staticEval >= beta + NULL_MARGIN * depth)
			&& position.NotOnlyPawns()) {
			Depth r = depth / 4 + 1;
			position.MakeNull();
			Value score = -Search(depth - r, ply + 1, -beta, 1 - beta, NTNONPV, false);
			position.UnmakeNull();
			if (score >= beta) {
				if (score >= CHECKMATE_NEAR)score = beta;
				return score;
			}
		}

	}*/

	//razoring
	if (staticEval < alpha - RAZOR_MARGIN * (depth + 2)) {
		//QSearch(alpha, beta, nt);
		score = QSearch(alpha - 1, alpha, nt);
		if (score < alpha
			&& abs(score) < CHECKMATE_NEAR)
			return score;
	}
	if (!inCheck) {


		//static null move pruning
		if (staticEval >= beta + depth * RFP_MARGIN
			&& abs(staticEval) < CHECKMATE_NEAR)
			return staticEval;

		//null move pruning
		if (doNull
			&& (staticEval >= beta + NULL_MARGIN * depth)
			&& position.NotOnlyPawns()) {
			position.MakeNull();
			Depth d = 4 + depth / 6;
			Value score = -Search(depth - d, ply + 1, -beta, 1 - beta, NTNONPV, false);
			position.UnmakeNull();
			if (score >= beta) {
				if (score >= CHECKMATE_NEAR)score = beta;
				return score;
			}
		}

	}

	bool prune = !inCheck && abs(alpha) < CHECKMATE_NEAR && staticEval + depth * FUTILITY_MARGIN <= alpha;

	Value bestScore = -CHECKMATE_MAX;
	Value bestMove = 0;
	Value oldAlpha = alpha;
	for (int n = 0; n < picker.count; n++)
	{
		PickerE pe = n < picker.index ? picker.pList[n] : picker.Pick(n);
		Move m = pe.move;
		int lmr = LMR[depth][n];

		if (bestMove && prune && (staticEval + pe.see < alpha - FUTILITY_MARGIN * depth)) {
			bestMove = alpha;
			break;
		}

		position.MakeMove(m);
		if (bestMove && prune && !position.InCheck() && m.IsQuiet()) {
			position.UnmakeMove(m);
			continue;
		}
		if (!n || inCheck)
			score = -Search(depth - 1, ply + 1, -beta, -alpha, NTPV);
		else {
			int reduction = lmr;
			if (pe.see < 0) {
				reduction++;
				if (nt == NTNONPV)
					reduction++;
			}
			score = -Search(depth - 1 - reduction, ply + 1, -alpha - 1, -alpha, NTNONPV);
			if (reduction && score > alpha)
				score = -Search(depth - 1, ply + 1, -alpha - 1, -alpha, NTNONPV);
			if (score > alpha && score < beta)
				score = -Search(depth - 1, ply + 1, -beta, -alpha, NTPV);
		}
		position.UnmakeMove(m);
		if (score > bestScore) {
			bestScore = score;
			bestMove = m.move;
		}
		if (score > alpha)
			alpha = score;
		if (score >= beta)
		{
			stack[ply].killer2.move = stack[ply].killer1.move;
			stack[ply].killer1.move = m.move;
			break;
		}
	}
	RecType rt = bestScore <= oldAlpha ? NODE_ALPHA : bestScore >= beta ? NODE_BETA : NODE_PV;
	tt.SetRec(position.GetHash(), bestScore, bestMove, rt, depth);
	return alpha;
}

//search first ply
Value SearchRoot(Picker& picker, Depth depth, Value alpha, Value beta) {
	Value oldAlpha = alpha;
	Value score = -CHECKMATE_INFINITY;
	Value bstScore = -CHECKMATE_INFINITY;
	U16 bstMove = 0;
	Color color = position.ColorUs();
	for (int n = 0; n < picker.count; n++) {
		Move m = picker.pList[n].move;
		position.MakeMove(m);
		bool inCheck = position.InCheck();
		if (!bstMove)
			score = -Search(depth - 1, 1, -beta, -alpha, NTPV);
		else {
			score = -Search(depth - 1, 1, -alpha - 1, -alpha, NTNONPV);
			if (score > alpha)
				score = -Search(depth - 1, 1, -beta, -alpha, NTPV);
		}
		position.UnmakeMove(m);
		if (bstScore < score) {
			bstScore = score;
			bstMove = m.move;
		}
		if (chronos.gameOver)
			return alpha;
		if (score >= beta)
			return beta;
		if (score > alpha) {
			alpha = score;
			picker.SetBest(n);
			sd.bstMove = m;
			sd.bstScore = score;
			ShowInfoPv();
		}
	}
	if (!chronos.gameOver && bstMove) {
		RecType rt = bstScore <= oldAlpha ? NODE_ALPHA : bstScore >= beta ? NODE_BETA : NODE_PV;
		tt.SetRec(position.GetHash(), bstScore, bstMove, rt, depth);
	}
	return alpha;
}


Value SearchWiden(Picker& picker, Depth depth, Value score) {
	Value alpha = score - options.aspiration;
	Value beta = score + options.aspiration;
	score = SearchRoot(picker, depth, alpha, beta);
	if (score <= alpha || score >= beta)
		score = SearchRoot(picker, depth, -CHECKMATE_MAX, CHECKMATE_MAX);
	return score;
}

//start search
void SearchIterate() {
	bool inCheck = position.InCheck();
	sd.Restart();
	tt.age++;
	Picker picker;
	position.MoveList(position.ColorUs(), picker.mList, picker.count);
	if (!picker.count) {
		cout << "info string no moves" << endl;
		return;
	}
	memset(stack, 0, sizeof(stack));
	picker.Fill();
	picker.Sort();
	Value score = Search(1, 0, -CHECKMATE_MAX, CHECKMATE_MAX, NTPV);
	for (sd.depth = 1; sd.depth < MAX_DEPTH; sd.depth++) {
		score = SearchWiden(picker, sd.depth, score);
		if (chronos.flags & FMOVETIME)
			if (sd.Ms() > chronos.movetime / 2)
				break;
		if (chronos.flags & FDEPTH)
			if (sd.depth >= chronos.depth)
				break;
		if (chronos.flags & FNODES)
			if (sd.nodes >= chronos.nodes)
				break;
		if ((picker.count == 1) && (sd.depth > 4))
			break;
	}
	ShowBestMove();
}
