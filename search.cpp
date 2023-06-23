#include "search.h"

#include "input.h"

#define MAX_DEPTH 100

const int32_t CHECKMATE_MAX = 0x7ff0;
const int32_t CHECKMATE_NEAR = 0x7000;
const int32_t CHECKMATE_INFINITY = 0x7fff;

int LMR_DIV = 198;
//int LMR_DIV = 128;
int lmrReductions[256][256];

using namespace std;
using namespace chrono;

sSearchDriver sd;

void LmrInit() {
	int d, m;
	for (d = 0; d < 256; d++)
		for (m = 0; m < 256; m++)
			lmrReductions[d][m] = 0.5 + log(d) * log(m) * 100 / LMR_DIV;
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
void InfoPv() {
	if (chronos.ponder || !chronos.post)
		return;
	uint64_t ms = sd.Ms();
	uint64_t nps = ms ? (sd.nodes * 1000) / ms : 0;
	string score = sd.bstScore > CHECKMATE_NEAR ? "mate " + to_string((CHECKMATE_MAX - sd.bstScore) >> 1) :
		sd.bstScore < -CHECKMATE_NEAR ? "mate " + to_string((-CHECKMATE_MAX - sd.bstScore + 2) >> 1) :
		"cp " + to_string(sd.bstScore);
	string pv = ExtractPv();
	cout << "info time " << ms << " depth " << sd.depth << " score " << score << " nps " << nps << " nodes " << sd.nodes << " hashfull " << tt.Permill() << " pv " << pv << endl;
}

//Quiesce search
int32_t Quiesce(S32 alpha, S32 beta, NodeTypes nt) {
	if (!(++sd.nodes & 0x1ffff))
		CheckTime();
	if (chronos.gameOver)
		return alpha;
	/*CRec* rec = tt.GetRec(position.GetHash());
	if (rec != nullptr) {
		if (rec->type == NODE_PV) {
			return rec->score;
		}
		else if (rec->type == NODE_CUT) {
			if (rec->score >= beta) {
				return beta;
			}
		}
		else if (rec->type == NODE_ALL) {
			if (rec->score <= alpha) {
				return alpha;
			}
		}
	}*/
	S32 staticEval = Eval();
	S32 score = staticEval;
	if (score >= beta)
		return beta;
	if (score > alpha)
		alpha = score;
	Color color = position.ColorUs();
	Picker picker;
	//position.MoveList(color, picker.list, picker.count, position.inCheck);
	position.MoveList(color, picker.list, picker.count, false);
	if (!picker.count)
		return alpha;
	picker.Fill();
	//if (nt == NTNONPV) {
		CRec* rec = tt.GetRec(position.GetHash());
		if (rec != nullptr)
			if (picker.SetMove(rec->move)) {
				if (rec->type == NODE_PV)
					return rec->score;
				else if (rec->type == NODE_CUT) {
					if (rec->score >= beta)
						return beta;
				}
				else if (rec->type == NODE_ALL)
					if (rec->score <= alpha)
						return alpha;

				if ((rec->type == NODE_PV) ||
					(rec->type == NODE_CUT && staticEval < rec->score) ||
					(rec->type == NODE_ALL && staticEval > rec->score))
					staticEval = rec->score;
			}
	//}
	S32 bestScore = -CHECKMATE_MAX;
	U16 bestMove = 0;
	S32 oldAlpha = alpha;
	for (int n = 0; n < picker.count; n++)
	{
		//PickerE pe = picker.Pick(n);
		//PickerE pe = picker.scores[n];
		PickerE pe = n<picker.index ?  picker.scores[n] : picker.Pick(n);
		Move m = pe.move;
		//if (!position.inCheck && nt == NTNONPV && pe.see < 0)continue;
		//if (Eval(m, true) < 0)
		//if (!position.inCheck && nt == NTNONPV && (staticEval + 325 + pe.see < alpha))continue;
		//if (!position.inCheck && nt == NTNONPV && (staticEval + 225 + pe.see < alpha))continue;
		//if (!position.inCheck && pe.see < -325)continue;
		position.MakeMove(m);
		score = -Quiesce(-beta, -alpha, nt);
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
int32_t Search(S32 depth, S32 ply, S32 alpha, S32 beta, NodeTypes nt, bool doNull) {
	if ((position.move50 >= 100) || (position.IsRepetition()))
		return ply & 1 ? -options.contempt : options.contempt;
	S32 score;
	bool raised_alpha = false;
	Color color = position.ColorUs();
	if (position.inCheck)
		depth++;
	if (depth < 1)
		return Quiesce(alpha, beta, nt);
	if (!(++sd.nodes & 0x1ffff))
		CheckTime();
	if (chronos.gameOver)
		return alpha;
	sd.nodesq++;

	//mate distance pruning
	int  mate_value = CHECKMATE_MAX - ply;
	if (alpha < -mate_value) alpha = -mate_value;
	if (beta > mate_value - 1) beta = mate_value - 1;
	if (alpha >= beta) return alpha;

	//trasposition table pruning
	int16_t staticEval = Eval();
	Picker picker;
	position.MoveList(color, picker.list, picker.count);
	if (!picker.count)return position.inCheck ? -CHECKMATE_MAX + ply : 0;
	picker.Fill();
	if (position.killers[ply][1].move)
		picker.SetMove(position.killers[ply][1]);
	if (position.killers[ply][0].move)
		picker.SetMove(position.killers[ply][0]);
	CRec* rec = tt.GetRec(position.GetHash());
	if (rec != nullptr)
		if (picker.SetMove(rec->move)) {
			if (rec->depth >= depth)
				if (rec->type == NODE_PV)
					return rec->score;
				else if (rec->type == NODE_CUT) {
					if (rec->score >= beta) {
						if (position.killers[ply][0].move !=rec->move && position.killers[ply][1].move != rec->move) {
							position.killers[ply][1].move = position.killers[ply][0].move;
							position.killers[ply][0].move = rec->move;
						}
						return beta;
					}
				}
				else if (rec->type == NODE_ALL)
					if (rec->score <= alpha)
						return alpha;

			if ((rec->type == NODE_PV) ||
				(rec->type == NODE_CUT && staticEval < rec->score) ||
				(rec->type == NODE_ALL && staticEval > rec->score))
				staticEval = rec->score;
		}
	//if (!position.inCheck && position.move50){
	if (!position.inCheck && nt == NTNONPV) {
		// Reverse futility pruning
		if (depth < 5) {
			const int margins[] = { 0, 50, 100, 200, 300 };
			if (staticEval - margins[depth] >= beta) {
				return beta;
			}
			// Null move pruning
			if (depth > 2 && staticEval >= beta && doNull && position.NotOnlyPawns()) {
				position.MakeNull();
				Score score = -Search(depth - 4 - depth / 6, ply + 1, -beta, 1 - beta, NTNONPV, false);
				position.UnmakeNull();
				if (score >= beta)
					return beta;
			}
			// Razoring
			if (depth == 1 && staticEval + 200 < alpha)
				return Quiesce(alpha, beta, nt);
		}
	}
	S16 bestScore = -CHECKMATE_MAX;
	U16 bestMove = 0;
	S16 oldAlpha = alpha;
	for (int n = 0; n < picker.count; n++)
	{
		//PickerE pe = picker.Pick(n);
		//if (picker.index > 3)cout << picker.index << endl;
		//PickerE pe = picker.scores[n];
		PickerE pe = n < picker.index? picker.scores[n] : picker.Pick(n);
		Move m = pe.move;
		int lmr = lmrReductions[depth][n];
		//if (!position.inCheck && lmr && pe.see < -225 && alpha == beta - 1) {
		if (lmr && !position.inCheck && nt == NTNONPV && pe.see < 0)continue;
		//if (lmr && !position.inCheck && nt == NTNONPV && (staticEval + depth*100 + pe.see < alpha))continue;
		//if (!position.inCheck && lmr && pe.see < 0 && alpha == beta - 1)continue;
	//if (!position.inCheck && lmr && pe.see < 0 && nt == NTNONPV)continue;
		//if (depth <= 5 && pe.see <= -100 * depth && nt == NTNONPV && n) continue;
		position.MakeMove(m);
		score = alpha + 1;
		if (!n || position.inCheck)
			score = -Search(depth - 1, ply + 1, -beta, -alpha, NTPV, true);
		else {
			int reduction = lmr;
			//int reduction = position.move50 ? lmr : 0;
			if (pe.see < 0) {
				reduction++;
				if (alpha == beta - 1) {
					reduction++;
					//if (position.move50)reduction++;
				}
			}
			//if(position.move50)
			score = -Search(depth - 1 - reduction, ply + 1, -alpha - 1, -alpha, NTNONPV, true);
			if (reduction && score > alpha)
				score = -Search(depth - 1, ply + 1, -alpha - 1, -alpha, NTNONPV, true);
			if (score > alpha && score < beta)
				score = -Search(depth - 1, ply + 1, -beta, -alpha, NTPV, true);
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
			if (position.killers[ply][0].move != m.move && position.killers[ply][1].move != m.move) {
				position.killers[ply][1].move = position.killers[ply][0].move;
				position.killers[ply][0].move = m.move;
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
int32_t SearchRoot(Picker& picker, S32 depth, S32 alpha, S32 beta) {
	int32_t score;
	int32_t best = -CHECKMATE_MAX;
	Color color = position.ColorUs();
	for (int n = 0; n < picker.count; n++) {
		Move m = picker.scores[n].move;
		position.MakeMove(m);
		score = alpha + 1;
		if (!position.inCheck && (best > -CHECKMATE_MAX))
			score = -Search(depth - 1, 2, -alpha - 1, -alpha, NTNONPV, false);
		if (score > alpha)
			score = -Search(depth - 1, 2, -beta, -alpha, NTPV, false);
		//cout << m.ToUci()<<" "<<score <<" "<<m.s << endl;
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
			InfoPv();
		}
	}
	return alpha;
}

//Attempt to narrow the window
S16 SearchWiden(Picker& picker, S16 depth, S16 score, U32 window) {
	if (position.inCheck)
		return SearchRoot(picker, depth, -CHECKMATE_MAX, CHECKMATE_MAX);
	S32	alpha = score - window;
	S32	beta = score + window;
	score = SearchRoot(picker, depth, alpha, beta);
	if (!chronos.gameOver && (score <= alpha || score >= beta)) {
		window <<= 1;
		return SearchWiden(picker, depth, score, window);
	}
	return score;
}

void BestMove() {
	if (chronos.ponder || !chronos.post)
		return;
	InfoPv();
	uint64_t proNode = sd.nodes ? (sd.nodesq * 100) / sd.nodes : 0;
	int proMove = sd.moveSet ? (sd.moveOk * 100) / sd.moveSet : 0;
	cout << "info string quiesce " << proNode << '%' << " hash " << proMove << '%' << endl;
	cout << "bestmove " << sd.bstMove.ToUci();
	if (options.ponder && sd.ponderMove.move)
		cout << " ponder " << sd.ponderMove.ToUci();
	cout << endl;
}

//Start search
void SearchIterate() {
	//position.phase = position.Phase();
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
	picker.Fill();
	picker.Sort();
	/*for (int n = 0; n < picker.count; n++)
	{
		PickerE pe = picker.scores[n];
		cout << pe.move.ToUci() << " " << pe.score << " " << pe.see << endl;
	}*/
	Score score = Search(1, 0, -CHECKMATE_MAX, CHECKMATE_MAX, NTPV, false);
	for (sd.depth = 1; sd.depth < MAX_DEPTH; sd.depth++) {
		score = SearchWiden(picker, sd.depth, score, 38);
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
	BestMove();
}