#include "program.h"

//Get index of key
static int UciIndex(vector<string> list, string command) {
	for (size_t n = 0; n < list.size() - 1; n++)
		if (list[n] == command)
			return n;
	return -1;
}

//Get next word after uci command
static bool UciValue(vector<string> list, string command, string& value) {
	value = "";
	for (size_t n = 0; n < list.size() - 1; n++)
		if (list[n] == command) {
			value = list[n + 1];
			return true;
		}
	return false;
}

//Get next word after uci command
static bool UciValues(vector<string> list, string command, string& value) {
	bool result = false;
	value = "";
	for (size_t n = 0; n < list.size(); n++) {
		if (result)
			value += " " + list[n];
		else if (list[n] == command)
			result = true;
	}
	value = Trim(value);
	return result;
}

static void UciEval() {
	ShowEval();
}

static void UciPonderhit(){
	info.infinite = false;
	info.ponder = false;
	info.post = true;
	info.stop = false;
	info.timeStart = GetTimeMs();
}

static void UciQuit() {
	exit(0);
}

static void UciStop() {
	info.stop = true;
}

//Performance test
static inline void PerftDriver(int depth)
{
	if (!depth)
	{
		info.nodes++;
		return;
	}
	int count;
	Move list[256];
	g_pos.MoveList(g_pos.ColorUs(), list, count);
	for (int i = 0; i < count; i++)
	{
		g_pos.MakeMove(list[i]);
		PerftDriver(depth - 1);
		g_pos.UnmakeMove(list[i]);
	}
}

static int ShrinkNumber(U64 n) {
	if (n < 1000)
		return 0;
	if (n < 1000000)
		return 1;
	if (n < 1000000000)
		return 2;
	return 3;
}

static void ResetLimit()
{
	info.stop = false;
	info.post = true;
	info.nodes = 0;
	info.multiPV = 1;
	info.depthLimit = (Depth)64;
	info.nodesLimit = 0;
	info.timeLimit = 0;
	info.timeStart = GetTimeMs();
	info.bestMove = MOVE_NONE;
	info.ponderMove = MOVE_NONE;
}


//displays a summary
static void PrintSummary(U64 time, U64 nodes) {
	if (time < 1)
		time = 1;
	U64 nps = (nodes * 1000) / time;
	const char* units[] = { "", "k", "m", "g" };
	int sn = ShrinkNumber(nps);
	U64 p = pow(10, sn * 3);
	printf("-----------------------------\n");
	printf("Time        : %llu\n", time);
	printf("Nodes       : %llu\n", nodes);
	printf("Nps         : %llu (%llu%s/s)\n", nps, nps / p, units[sn]);
	printf("-----------------------------\n");
}

//Start benchamrk test
static void UciBench() {
	ResetLimit();
	printf("Benchmark Test\n");
	g_pos.SetFen();
	info.depthLimit = DEPTH_ZERO;
	info.post = false;
	while (GetTimeMs() - info.timeStart < 3000)
	{
		++info.depthLimit;
		SearchIteratively();
		printf("%2d. %8llu %12llu\n", info.depthLimit, GetTimeMs() - info.timeStart, info.nodes);
	}
	PrintSummary(GetTimeMs() - info.timeStart, info.nodes);
}

//StartPerformance test
static void UciPerformance()
{
	ResetLimit();
	printf("Performance Test\n");
	int depth = 0;
	g_pos.SetFen();
	while (GetTimeMs() - info.timeStart < 3000)
	{
		PerftDriver(++depth);
		printf("%2d. %8llu %12llu\n", depth, GetTimeMs() - info.timeStart, info.nodes);
	}
	PrintSummary(GetTimeMs() - info.timeStart, info.nodes);
}

//Supports all uci commands
void UciCommand(string str) {
	str = Trim(str);
	string value;
	vector<string> split{};
	SplitString(str, split, ' ');
	Move m;
	if (split.empty())
		return;
	string command = split[0];
	if (command == "uci")
	{
		int delta = 50;
		cout << "id name " << NAME << endl;
		cout << "option name hash type spin default " << options.hash << " min 1 max 1024" << endl;
		cout << "option name MultiPV type spin default 1 min 1 max 32" << endl;
		printf("option name UCI_Elo type spin default %d min %d max %d\n", options.eloMax, options.eloMin, options.eloMax);
		printf("option name rfp type spin default %d min %d max %d\n", options.rfp, options.rfp - delta, options.rfp + delta);
		printf("option name futility type spin default %d min %d max %d\n", options.futility, options.futility - delta, options.futility + delta);
		printf("option name razoring type spin default %d min %d max %d\n", options.razoring, options.razoring - delta, options.razoring + delta);
		printf("option name null type spin default %d min %d max %d\n", options.nullMove, options.nullMove - delta, options.nullMove + delta);
		printf("option name LMR type spin default %d min %d max %d\n", options.lmr, options.lmr - delta, options.lmr + delta);
		printf("option name aspiration type spin default %d min %d max %d\n", options.aspiration, options.aspiration - delta, options.aspiration + delta);
		cout << "option name ponder type check default " << (options.ponder ? "true" : "false") << endl;
		cout << "option name material type string default " << options.material << endl;
		cout << "option name mobility type string default " << options.mobility << endl;
		cout << "option name passed type string default " << options.passed << endl;
		cout << "option name pawn type string default " << options.pawn << endl;
		cout << "option name rook type string default " << options.rook << endl;
		cout << "option name king type string default " << options.king << endl;
		cout << "option name outpost type string default " << options.outpost << endl;
		cout << "option name bishop type string default " << options.bishop << endl;
		cout << "option name defense type string default " << options.defense << endl;
		cout << "option name outFile type string default " << options.outFile << endl;
		cout << "option name outRank type string default " << options.outRank << endl;
		cout << "option name tempo type string default " << options.tempo << endl;
		cout << "uciok" << endl;
	}
	else if (command == "isready")
		cout << "readyok" << endl;
	else if (command == "ucinewgame") {
		tt.Clear();
		InitEval();
		InitSearch();
	}
	else if (command == "position") {
		int mark = 0;
		string fen = "";
		vector<string> moves = {};
		for (int i = 1; i < split.size(); i++) {
			if (mark == 1)
				fen += ' ' + split[i];
			if (mark == 2)
				moves.push_back(split[i]);
			if (split[i] == "fen")
				mark = 1;
			else if (split[i] == "moves")
				mark = 2;
		}
		fen = Trim(fen);
		g_pos.SetFen(fen == "" ? DEFAULT_FEN : fen);
		for (string uci : moves) {
			CMoveList list = CMoveList(g_pos);
			for (Move m : list)
				if (m.ToUci() == uci)
					g_pos.MakeMove(m);
		}
	}
	else if (command == "go") {
		ResetLimit();
		int depth = DEPTH_MAX;
		int nodes = 0;
		int wtime = 0;
		int btime = 0;
		int winc = 0;
		int binc = 0;
		int movetime = 0;
		int movestogo = 32;
		string com2;
		if (UciValue(split, "go", com2)) {
			if (com2 == "infinite")
				info.infinite = true;
			if (com2 == "ponder") {
				info.ponder = true;
				info.infinite = true;
			}
			int index = UciIndex(split, "searchmoves");
			if (index > 0) {
				CMoveList list = CMoveList(g_pos);
				for (int n = index + 1; n < split.size(); n++)
					for (Move m : list)
						if (m.ToUci() == split[n])
							info.rootMoves.push_back(m);
			}
		}
		if (UciValue(split, "wtime", value))
			wtime = stoi(value);
		if (UciValue(split, "btime", value))
			btime = stoi(value);
		if (UciValue(split, "winc", value))
			winc = stoi(value);
		if (UciValue(split, "binc", value))
			binc = stoi(value);
		if (UciValue(split, "movestogo", value))
			movestogo = stoi(value);
		if (UciValue(split, "depth", value))
			depth = stoi(value);
		if (UciValue(split, "nodes", value))
			nodes = stoi(value);
		if (UciValue(split, "movetime", value))
			movetime = stoi(value);
		int time = g_pos.color ? btime : wtime;
		int inc = g_pos.color ? binc : winc;
		int st = min(time / movestogo + inc, time / 2);
		info.depthLimit = (Depth)depth;
		info.nodesLimit = nodes;
		info.timeLimit = movetime ? movetime : st;
		SearchIteratively();
	}
	else if (command == "ponderhit")
		UciPonderhit();
	else if (command == "quit")
		UciQuit();
	else if (command == "stop")
		UciStop();
	else if (command == "setoption")
	{
		string name;
		bool isValue = UciValues(split, "value", value);
		if (isValue && UciValue(split, "name", name)) {
			name = StrToLower(name);
			if (name == "ponder")
				options.ponder = value == "true";
			else if (name == "hash")
				tt.Resize(stoi(value));
			else if (name == "multipv")
				options.multiPV = stoi(value);
			else if (name == "uci_elo")
				options.elo = stoi(value);
			else if (name == "rfp")
				options.rfp = stoi(value);
			else if (name == "futility")
				options.futility = stoi(value);
			else if (name == "razoring")
				options.razoring = stoi(value);
			else if (name == "null")
				options.nullMove = stoi(value);
			else if (name == "lmr")
				options.lmr = stoi(value);
			else if (name == "aspiration")
				options.aspiration = stoi(value);
			else if (name == "tempo")
				options.tempo = value;
			else if (name == "material")
				options.material = value;
			else if (name == "mobility")
				options.mobility = value;
			else if (name == "outfile")
				options.outFile = value;
			else if (name == "outrank")
				options.outRank = value;
			else if (name == "passed")
				options.passed = value;
			else if (name == "pawn")
				options.pawn = value;
			else if (name == "rook")
				options.rook = value;
			else if (name == "king")
				options.king = value;
			else if (name == "outpost")
				options.outpost = value;
			else if (name == "bishop")
				options.bishop = value;
			else if (name == "defense")
				options.defense = value;
		}
	}
	else if (command == "bench")
		UciBench();
	else if (command == "perft")
		UciPerformance();
	else if (command == "eval")
		UciEval();
	else if (command == "print")
		g_pos.PrintBoard();
}

//main uci loop
void UciLoop() {
	string line;
	while (true) {
		getline(cin, line);
		UciCommand(line);
	}
}