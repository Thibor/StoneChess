#include "search.h"

#include "input.h"

#define MAX_DEPTH 100

const int32_t CHECKMATE_MAX = 0x7ff0;
const int32_t CHECKMATE_NEAR = 0x7000;
const int32_t CHECKMATE_INFINITY = 0x7fff;

int  RAZOR_MARGIN = 190;
int LMR_DIV = 267;
int  FUTILITY_MARGIN = 68;
int lmrReductions[256][256];

using namespace std;
using namespace chrono;

sSearchDriver sd;

void LmrInit() {
	int d, m;
	for (d = 0; d < 256; d++)
		for (m = 0; m < 256; m++)
			lmrReductions[d][m] = 1.25 + log(d) * log(m) * 100 / LMR_DIV;
}

void SearchInit() {
	LmrInit();
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

//Show depth score and next best move in Principal variation
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
Score QSearch(Score alpha, Score beta, NodeTypes nt) {
	if (!(++sd.nodes & 0x1ffff))
		CheckTime();
	if (chronos.gameOver)
		return alpha;
	Score staticEval = Eval();
	Score score = staticEval;
	if (score >= beta)
		return beta;
	if (score > alpha)
		alpha = score;
	Color color = position.ColorUs();
	Picker picker;
	position.MoveList(color, picker.list, picker.count, false);
	if (!picker.count)
		return alpha;
	picker.Fill();
	if (nt == NTNONPV) {
	CRec* rec = tt.GetRec(position.GetHash());
	if (rec != nullptr)
		if (picker.SetMove(rec->move)) {
			if (rec->type == NODE_PV)
				return rec->score;
			else if (rec->type == NODE_CUT) {
				if (rec->score >= beta)
					return beta;
				//return rec->score;
			}
			else if (rec->type == NODE_ALL)
				if (rec->score <= alpha)
					return alpha;
			//return rec->score;

			if ((rec->type == NODE_PV) ||
				(rec->type == NODE_CUT && staticEval < rec->score) ||
				(rec->type == NODE_ALL && staticEval > rec->score))
				staticEval = rec->score;
		}
	}
	Score bestScore = -CHECKMATE_MAX;
	U16 bestMove = 0;
	Score oldAlpha = alpha;
	bool inCheck = position.InCheck();
	for (int n = 0; n < picker.count; n++)
	{
		PickerE pe = n < picker.index ? picker.scores[n] : picker.Pick(n);
		Move m = pe.move;
		if (n && !inCheck && pe.see < 0)break;
		if (!inCheck && bestMove) {
			if (pe.see < 0)
				continue;
			if (pe.see + staticEval > beta + 200)
				return beta;
		}
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
	if (!chronos.gameOver) {
		RecType rt = bestScore <= oldAlpha ? NODE_ALL : bestScore >= beta ? NODE_CUT : NODE_PV;
		tt.SetRec(position.GetHash(), bestScore, bestMove, rt, 0);
	}
	return alpha;
}

//Main search loop
Score Search(Depth depth, Depth ply, Score alpha, Score beta, NodeTypes nt, bool doNull = true) {
	if ((position.move50 >= 100) || (position.IsRepetition()))
		return ply & 1 ? -options.contempt : options.contempt;
	Score score;
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
		return beta;
	sd.nodesq++;

	//mate distance pruning
	int  mate_value = CHECKMATE_MAX - ply;
	if (alpha < -mate_value) alpha = -mate_value;
	if (beta > mate_value) beta = mate_value;
	if (alpha >= beta) return alpha;

	//trasposition table pruning
	Score staticEval = Eval();
	Picker picker;
	position.MoveList(color, picker.list, picker.count);
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
				else if (rec->type == NODE_CUT) {
					if (rec->score >= beta) {
						if (stack[ply].killer1.move != rec->move && stack[ply].killer2.move != rec->move) {
							stack[ply].killer2.move = stack[ply].killer1.move;
							stack[ply].killer1.move = rec->move;
						}
						return beta;
						//return rec->score;
					}
				}
				else if (rec->type == NODE_ALL)
					if (rec->score <= alpha)
						return alpha;
			//return rec->score;

			if ((rec->type == NODE_PV) ||
				(rec->type == NODE_CUT && staticEval < rec->score) ||
				(rec->type == NODE_ALL && staticEval > rec->score))
				staticEval = rec->score;
		}

	if (!inCheck && nt == NTNONPV) {

		//razoring
		if (depth <= 3 && staticEval + RAZOR_MARGIN * depth < beta) {
			score = QSearch(alpha, beta, nt);
			if (score < beta)
				return score;
		}
		//static null move pruning
		if (depth <= 7 && staticEval >= beta + depth * FUTILITY_MARGIN && staticEval < CHECKMATE_NEAR)
			return staticEval;
		//null move pruning
		if (doNull && staticEval >= beta + (5 > depth ? 30 : 0) && (depth >= 5) && position.NotOnlyPawns()) {
			position.MakeNull();
			Score score = -Search(depth - (depth / 4 + 3) - (staticEval - beta < 300 ? (staticEval - beta) / FUTILITY_MARGIN : 3), ply + 1, -beta, 1 - beta, NTNONPV, false);
			position.UnmakeNull();
			if (score >= beta) {
				// dont return mate/tb scores
				if (score >= CHECKMATE_NEAR)
					score = beta;
				return score;
			}
		}
	}

	/*
	//static null move pruning
	if (depth < 3 && nt == NTNONPV && !inCheck) {
		int evalMargin = 120 * depth;
		if (staticEval - evalMargin >= beta)
			return staticEval - evalMargin;
	}

	//null move pruning
	if (depth > 2 && doNull && nt == NTNONPV && staticEval > beta && !inCheck && position.NotOnlyPawns()) {
		int r = depth > 6 ? 3 : 2;
		position.MakeNull();
		Score score = -Search(depth -1-r, ply + 1, -beta, 1 - beta, NTNONPV, false);
		position.UnmakeNull();
		if (score >= beta) {
			// dont return mate/tb scores
			if (score >= CHECKMATE_NEAR)
				score = beta;
			return score;
		}
	}

	//razoring
	if (nt == NTNONPV && !inCheck && !hashMove && doNull && depth <= 3) {
		int threshold = alpha - 300 - (depth - 1) * 60;
		if (staticEval < threshold) {
			int val = Quiesce(alpha, beta, NTNONPV);
			if (val < threshold) return alpha;
		}
	}*/

	int fmargin[4] = { 0, 200, 300, 500 };

	bool prune = depth <= 3 && !inCheck && abs(alpha) < CHECKMATE_NEAR && staticEval + fmargin[depth] <= alpha;

	if (depth >= 4 && !rec)depth--;

	Score bestScore = -CHECKMATE_MAX;
	Score bestMove = 0;
	Score oldAlpha = alpha;
	for (int n = 0; n < picker.count; n++)
	{
		PickerE pe = n < picker.index ? picker.scores[n] : picker.Pick(n);
		Move m = pe.move;
		int lmr = lmrReductions[depth][n];

		if (bestMove && !inCheck && nt == NTNONPV && pe.see < 0 && prune)continue;

		position.MakeMove(m);
		if (bestMove&&prune &&!inCheck && m.IsQuiet() && !position.InCheck()) {
			position.UnmakeMove(m);
			continue;
		}
		if (!n || inCheck)
			score = -Search(depth - 1, ply + 1, -beta, -alpha, NTPV);
		else {
			int reduction = lmr;
			if (nt == NTNONPV)
				reduction++;
			if (pe.see < 0)
				reduction++;
			score = -Search(depth - 1 - reduction, ply + 1, -alpha - 1, -alpha, NTNONPV);
			if (reduction && score > alpha)
				score = -Search(depth - 1, ply + 1, -alpha - 1, -alpha, NTNONPV);
			if (score > alpha && score < beta)
				score = -Search(depth - 1, ply + 1, -beta, -alpha, NTPV);
		}
		position.UnmakeMove(m);
		if (chronos.gameOver)
			return alpha;
		if (score > bestScore) {
			bestScore = score;
			bestMove = m.move;
		}
		if (score > alpha)
			alpha = score;
		if (score >= beta)
		{
			if (stack[ply].killer1.move != m.move && stack[ply].killer2.move != m.move) {
				stack[ply].killer2.move = stack[ply].killer1.move;
				stack[ply].killer1.move = m.move;
			}
			break;
		}
	}
	if (!chronos.gameOver) {
		RecType rt = bestScore <= oldAlpha ? NODE_ALL : bestScore >= beta ? NODE_CUT : NODE_PV;
		tt.SetRec(position.GetHash(), bestScore, bestMove, rt, depth);
	}
	return alpha;
}

//Search first ply
Score SearchRoot(Picker& picker, Depth depth, Score alpha, Score beta) {
	Score score;
	Score best = -CHECKMATE_MAX;
	Color color = position.ColorUs();
	for (int n = 0; n < picker.count; n++) {
		Move m = picker.scores[n].move;
		position.MakeMove(m);
		bool inCheck = position.InCheck();
		score = alpha + 1;
		if (!inCheck && (best > -CHECKMATE_MAX))
			score = -Search(depth - 1, 1, -alpha - 1, -alpha, NTNONPV, false);
		if (score > alpha)
			score = -Search(depth - 1, 1, -beta, -alpha, NTPV, false);
		position.UnmakeMove(m);
		if (best < score)
			best = score;
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
	return alpha;
}

//Attempt to narrow the window
Score SearchWiden(U16 index, Picker& picker, Depth depth, Depth sDepth, Score score, U32 window) {
	Score alpha = score - window;
	Score beta = score + window;
	if (++index > 16)
		return SearchRoot(picker, depth, -CHECKMATE_MAX, CHECKMATE_MAX);
	sDepth = sDepth < depth - 3 ? depth - 3 : sDepth;
	score = SearchRoot(picker, sDepth, alpha, beta);
	if (!chronos.gameOver && (score <= alpha || score >= beta)) {
		if (score >= beta) {
			beta += window;
			sDepth--;
		}
		else if (score <= alpha) {
			beta = (alpha + beta) / 2;
			alpha -= window;
		}
		score = (alpha + beta) / 2;
		window += 10;
		return SearchWiden(index, picker, depth, sDepth, score, window);
	}
	return score;
}

//Start search
void SearchIterate() {
	bool inCheck = position.InCheck();
	sd.Restart();
	tt.age++;
	Picker picker;
	position.MoveList(position.ColorUs(), picker.list, picker.count);
	if (!picker.count) {
		cout << "info string no moves" << endl;
		return;
	}
	if (picker.count == 1) {
		cout << "bestmove " << picker.list[0].ToUci() << endl;
		return;
	}
	for (int n = 2; n < 128; n++)
		stack[n - 2] = stack[n];
	picker.Fill();
	picker.Sort();
	int index = 0;
	Score score = Search(1, 0, -CHECKMATE_MAX, CHECKMATE_MAX, NTPV, false);
	for (sd.depth = 1; sd.depth < MAX_DEPTH; sd.depth++) {
		score = inCheck ?
			SearchRoot(picker, sd.depth, -CHECKMATE_MAX, CHECKMATE_MAX) :
			SearchWiden(index, picker, sd.depth, sd.depth, score, options.aspiration);
		if (chronos.flags & FMOVETIME)
			if (sd.Ms() > chronos.movetime / 2)
				break;
		if (chronos.flags & FDEPTH)
			if (sd.depth >= chronos.depth)
				break;
		if (chronos.flags & FNODES)
			if (sd.nodes >= chronos.nodes)
				break;
	}
	ShowBestMove();
}