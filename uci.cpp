#include "uci.h"
#include "types.h"

using namespace std;

SOptions options;

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
	value = trim(value);
	return result;
}

//Performance test
static uint64_t Perft(int depth)
{
	uint64_t nodes = 0;
	int count;
	Move list[256];
	g_pos.MoveList(g_pos.ColorUs(), list, count);
	if (depth == 1)
		return count;
	for (int i = 0; i < count; i++)
	{
		g_pos.MakeMove(list[i]);
		nodes += Perft(depth - 1);
		g_pos.UnmakeMove(list[i]);
	}
	return nodes;
}

//Displays a summary
static void ShowInfo(uint64_t time, uint64_t nodes) {
	if (time < 1)
		time = 1;
	uint64_t nps = (nodes * 1000) / time;
	printf("-----------------------------\n");
	cout << "Time        : " << thousandSeparator(time) << endl;
	cout << "Nodes       : " << thousandSeparator(nodes) << endl;
	cout << "Nps         : " << thousandSeparator(nps) << endl;
	printf("-----------------------------\n");
}

//StartPerformance test
static void UciPerft(int depth)
{
	printf("Performance Test\n");
	uint64_t time = 0;
	uint64_t nodes = 0;
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	g_pos.SetFen();
	for (int d = 1; d <= depth; d++)
	{
		nodes += Perft(d);
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
		time = duration.count();
		cout << d << ".\t" << thousandSeparator(time) << "\t" << thousandSeparator(nodes) << endl;
	}
	ShowInfo(time, nodes);
}

//Start benchamrk test
static void UciBench(int depth) {
	printf("Benchmark Test\n");
	uint64_t time = 0;
	uint64_t nodes = 0;
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	g_pos.SetFen();
	chronos.post = false;
	chronos.flags = FDEPTH;
	for (int d = 1; d <= depth; d++)
	{
		chronos.depth = d;
		SearchIterate();
		nodes = sd.nodes;
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
		time = duration.count();
		cout << d << ".\t" << thousandSeparator(time) << "\t" << thousandSeparator(nodes) << endl;
	}
	ShowInfo(time, nodes);
}

static void UciEval() {
	ShowEval();
}

static void UciTest() {
	Score i = S(-54, 2) * -3;
	//cout << Mg(i) << " " << Eg(i) << endl;
	//UciCommand("position startpos moves e2e4 e7e5 g1f3 b8c6 d2d4 g8f6 d4e5 f8b4 c2c3 e8g8 e5f6 d7d5 c3b4 c8g4 f1e2 d5d4 e1g1 d4d3 e2d3 c6e5 d3e2 d8d1 f1d1 g4f3 e2f3 f8d8 d1d8 a8d8 f3e2 a7a6 b1c3 c7c6 c1e3 b7b6 e3b6 d8d2 b2b3 c6c5 b4c5 h7h6 a1d1 d2d1 e2d1 e5c6 d1e2 g7g6 e2a6 g8f8 a2a3 f8g8 b3b4 g8h7 c3d5 h7h8 d5e7 c6e5 c5c6 e5c6 e7c6 g6g5 b6e3 h8h7 a6d3 h7h8 b4b5 h8h7 b5b6 h7g6 b6b7 g6f6 b7b8q f6g7 a3a4 g7h7 a4a5 g5g4 a5a6 f7f6 a6a7 f6f5 e4f5");
	UciCommand("setoption name MultiPV value 4");
	UciCommand("position fen 5r1k/6pp/1n2Q3/4p3/8/7P/PP4PK/R1B1q3 b - - 0 1");
	//UciCommand("go movetime 3000");
	UciCommand("go depth 3");
	/*rootPos.SetFen("8/6K1/8/8/4Q3/2k5/8/8 w - - 0 20");
	chronos.Reset();
	//chronos.flags |= FINFINITE;
	chronos.flags |= FMOVETIME;
	chronos.depth = 3;
	SearchIterate();*/
}

static void UciPonderhit()
{
	chronos.ponder = false;
	chronos.flags &= ~FINFINITE;
	sd.timeStart = chrono::steady_clock::now();
}

static void UciQuit() {
	exit(0);
}

static void UciStop() {
	chronos.gameOver = true;
}

//Supports all uci commands
void UciCommand(string str) {
	str = trim(str);
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
		cout << "id name " << engineName << endl;
		cout << "option name hash type spin default " << options.hash << " min 1 max 1024" << endl;
		cout << "option name MultiPV type spin default 1 min 1 max 32" << endl;
		printf("option name UCI_Elo type spin default %d min %d max %d\n", options.eloMax, options.eloMin, options.eloMax);
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
		fen = trim(fen);
		g_pos.SetFen(fen == "" ? DEFAULT_FEN : fen);
		for (string uci : moves) {
			CMoveList list = CMoveList(g_pos);
			for (Move m : list)
				if (m.ToUci() == uci)
					g_pos.MakeMove(m);
		}
	}
	else if (command == "go") {
		chronos.Reset();
		string com2;
		if (UciValue(split, "go", com2)) {
			if (com2 == "infinite")
				chronos.flags |= FINFINITE;
			if (com2 == "ponder") {
				chronos.ponder = true;
				chronos.flags |= FINFINITE;
			}
			int index = UciIndex(split, "searchmoves");
			if (index > 0) {
				CMoveList list = CMoveList(g_pos);
				for (int n = index + 1; n < split.size(); n++)
					for (Move m : list)
						if (m.ToUci() == split[n])
							chronos.rootMoves.push_back(m);
			}
		}
		if (UciValue(split, "wtime", value))
		{
			chronos.flags |= FTIME;
			chronos.time[WHITE] = stoi(value);
		}
		if (UciValue(split, "btime", value))
		{
			chronos.flags |= FTIME;
			chronos.time[BLACK] = stoi(value);
		}
		if (UciValue(split, "winc", value))
		{
			chronos.flags |= FINC;
			chronos.inc[WHITE] = stoi(value);
		}
		if (UciValue(split, "binc", value))
		{
			chronos.flags |= FINC;
			chronos.inc[BLACK] = stoi(value);
		}
		if (UciValue(split, "movestogo", value))
		{
			chronos.flags |= FMOVESTOGO;
			chronos.movestogo = stoi(value);
		}
		if (UciValue(split, "depth", value))
		{
			chronos.flags |= FDEPTH;
			chronos.depth = stoi(value);
		}
		if (UciValue(split, "nodes", value))
		{
			chronos.flags |= FNODES;
			chronos.nodes = stoi(value);
		}
		if (UciValue(split, "movetime", value))
		{
			chronos.flags |= FMOVETIME;
			chronos.movetime = stoi(value);
		}
		if (UciValue(split, "searchmoves", value))
		{
		}
		if (!chronos.flags)
			chronos.flags |= FINFINITE;
		if (chronos.flags & FTIME) {
			chronos.flags |= FMOVETIME;
			if (chronos.movestogo)
				chronos.movetime = chronos.time[g_pos.ColorUs()] / chronos.movestogo;
			else
				chronos.movetime = chronos.time[g_pos.ColorUs()] / 32 + chronos.inc[g_pos.ColorUs()] / 2;
		}
		SearchIterate();
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
				options.tempo = stoi(value);
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
	else if (command == "bench") {
		if (UciValue(split, "bench", value))
			UciBench(stoi(value));
		else
			UciBench(10);
	}
	else if (command == "perft") {
		if (UciValue(split, "perft", value))
			UciPerft(stoi(value));
		else
			UciPerft(6);
	}
	else if (command == "eval")
		UciEval();
	else if (command == "print")
		g_pos.PrintBoard();
	else if (command == "test")
		UciTest();
}

//main uci loop
void UciLoop() {
	//UciCommand("position fen 3k4/5Q2/3Np1p1/1pPp4/6n1/P3PN2/5PPP/R3K2R b KQ - 0 25");
	//std::cout << position << std::endl;
	//UciCommand("go infinite");
	/*Score s = S(3200, -3200);
	//Score s = S(0, -32768);
	s -= S(100, -100)*2;
	Value mg = Mg(s);
	Value eg = Eg(s);
	cout << mg << " " << eg << endl;*/
	string line;
	while (true) {
		getline(cin, line);
		UciCommand(line);
	}
}