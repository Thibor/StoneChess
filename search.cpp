#include "search.h"

#include "input.h"

#define MAX_DEPTH 100

const int32_t CHECKMATE_MAX = 0x7ff0;
const int32_t CHECKMATE_NEAR = 0x7000;
const int32_t CHECKMATE_INFINITY = 0x7fff;


using namespace std;
using namespace chrono;

sSearchDriver sd;

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
	uint64_t hash = position.get_hash();
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
	uint64_t nps = ms?(sd.nodes * 1000) / ms:0;
	string score = sd.bstScore > CHECKMATE_NEAR ? "mate " + to_string((CHECKMATE_MAX - sd.bstScore) >> 1) :
		sd.bstScore < -CHECKMATE_NEAR ? "mate " + to_string((-CHECKMATE_MAX - sd.bstScore + 2) >> 1) :
		"cp " + to_string(sd.bstScore);
	string pv = ExtractPv();
	cout << "info time " << ms << " depth " << sd.depth << " score " << score << " nps " << nps << " nodes " << sd.nodes << " hashfull " << tt.Permill() << " pv " << pv << endl;
}

//Quiesce search
int32_t Quiesce(S32 alpha, S32 beta) {
	if (!(++sd.nodes & 0x1ffff))
		CheckTime();
	if (chronos.gameOver)
		return alpha;
	CRec* rec = tt.GetRec(position.get_hash());
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
	}
	S32 score = Eval();
	if (score >= beta)
		return beta;
	if (score > alpha)
		alpha = score;
	Color color = position.ColorUs();
	Picker picker;
	position.MoveListQ(color, picker.list, picker.count);
	if (!picker.count)
		return alpha;
	picker.Fill(true);
	S32 bestScore = -CHECKMATE_MAX;
	U16 bestMove = 0;
	S32 oldAlpha = alpha;
	for (int n = 0; n < picker.count; n++)
	{
		Move m = (picker.Pick(n)).move;
		position.MakeMove(m);
		score = -Quiesce(-beta, -alpha);
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
		tt.SetRec(position.get_hash(), bestScore, bestMove, rt, 0);
	}
	return alpha;
}

//Main search loop
int32_t Search(S32 depth, S32 ply, S32 alpha, S32 beta,bool doNull) {
	if ((position.move50 >= 100) || (position.IsRepetition()))
		return ply & 1 ? -options.contempt : options.contempt;
	S32 score;
	bool raised_alpha = false;
	Color color = position.ColorUs();
	if (position.inCheck)
		depth++;
	if (depth < 1)
		return Quiesce(alpha, beta);
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
	picker.Fill();
	if (!picker.count)return position.inCheck ? -CHECKMATE_MAX + ply : 0;
	CRec* rec = tt.GetRec(position.get_hash());
	if (rec != nullptr)
		if (picker.SetMove(rec->move)) {
			if (rec->depth >= depth)
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
	if (!position.inCheck && alpha == beta - 1) {
		// Reverse futility pruning
		if (depth < 5) {
			const int margins[] = { 0, 50, 100, 200, 300 };
			if (staticEval - margins[depth] >= beta) {
				return beta;
			}
			// Null move pruning
			if (depth > 2 && staticEval >= beta && doNull && position.NotOnlyPawns()) {
				position.MakeNull();
				Score score = -Search(depth - 4 - depth / 6, ply + 1, -beta, 1-beta, false);
				position.UnmakeNull();
				if (score >= beta)
					return beta;
			}
			// Razoring
			if (depth == 1 && staticEval + 200 < alpha)
				return Quiesce(alpha, beta);
		}
	}
	S16 bestScore = -CHECKMATE_MAX;
	U16 bestMove = 0;
	S16 oldAlpha = alpha;
	for (int n = 0; n < picker.count; n++)
	{
		Move m = (picker.Pick(n)).move;
		position.MakeMove(m);
		score = alpha + 1;
		if (!position.InCheck())
			score = -Search(depth - 1, ply + 1, -alpha - 1, -alpha,true);
		if (score > alpha)
			score = -Search(depth - 1, ply + 1, -beta, -alpha,true);
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
			break;
	}
	if (!chronos.gameOver) {
		RecType rt = bestScore <= oldAlpha ? NODE_ALL : bestScore >= beta ? NODE_CUT : NODE_PV;
		tt.SetRec(position.get_hash(), bestScore, bestMove, rt, depth);
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
			score = -Search(depth - 1, 2, -alpha - 1, -alpha,false);
		if (score > alpha)
			score = -Search(depth - 1, 2, -beta, -alpha,false);
		//cout << m.ToUci()<<" "<<score << endl;
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
	position.phase = position.Phase();
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
	S16 score = Search(1, 0, -CHECKMATE_MAX, CHECKMATE_MAX,false);
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