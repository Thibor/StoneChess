#include "uci.h"

using namespace std;

SOptions options;
SOptions optionsOrg;

//Get next word after uci command
bool UciValue(vector<string> list, string command, string& value) {
	value = "";
	for (size_t n = 0; n < list.size() - 1; n++)
		if (list[n] == command) {
			value = list[n + 1];
			return true;
		}
	return false;
}

//Performance test
uint64_t Perft(int depth)
{
	uint64_t nodes = 0;
	int count;
	Move list[256];
	position.MoveList(position.ColorUs(), list, count);
	if (depth == 1)
		return count;
	for (int i = 0; i < count; i++)
	{
		position.MakeMove(list[i]);
		nodes += Perft(depth - 1);
		position.UnmakeMove(list[i]);
	}
	return nodes;
}

//Displays a summary
void ShowInfo(uint64_t time, uint64_t nodes) {
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
void UciPerft(int depth)
{
	printf("Performance Test\n");
	uint64_t time = 0;
	uint64_t nodes = 0;
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	position.SetFen();
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
void UciBench(int depth) {
	printf("Benchmark Test\n");
	uint64_t time = 0;
	uint64_t nodes = 0;
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	position.SetFen();
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

void UciEval() {
	uint16_t score = Eval();
	printf("score %d\n", score);
}

void UciPonderhit()
{
	chronos.ponder = false;
	chronos.flags &= ~FINFINITE;
	sd.timeStart = chrono::steady_clock::now();
}

void UciQuit() {
	exit(0);
}

void UciStop() {
	chronos.gameOver = true;
}

//Supports all uci commands
void UciCommand(string str) {
	str = trim(str);
	string value;
	vector<string> split{};
	splitString(str, split, ' ');
	Move m;
	if (split.empty())
		return;
	string command = split[0];
	if (command == "uci")
	{
		puts("id name StoneChess");
		puts("id author Thibor Raven");
		printf("option name hash type spin default 64 min 1 max 1024\n");
		printf("option name UCI_Elo type spin default %d min %d max %d\n", optionsOrg.eloMax, optionsOrg.eloMin, optionsOrg.eloMax);
		printf("option name contempt type spin default %d min -50 max 50\n", optionsOrg.contempt);
		printf("option name ponder type check default %s\n", optionsOrg.ponder ? "true" : "false");
		puts("uciok");
	}
	else if (command == "isready")
		puts("readyok");
	else if (command == "ucinewgame")
		tt.Clear();
	else if (command == "position") {
		bool bfen = false;
		bool bmov = false;
		int i = 0;
		string fen = "";
		vector<string> moves = {};
		while (i < split.size()) {
			if (bfen)
				fen += ' ' + split[i];
			if (bmov)
				moves.push_back(split[i]);
			if (split[i] == "fen")
				bfen = true;
			else if (split[i] == "moves")
				bmov = true;
			i++;
		}
		fen = trim(fen);
		position.SetFen(fen == "" ? DEFAULT_FEN : fen);
		for (string uci : moves) {
			CMoveList list = CMoveList(position);
			for (Move m : list)
				if (m.ToUci() == uci)
					position.MakeMove(m);
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
		if (!chronos.flags)
			chronos.flags |= FINFINITE;
		if (chronos.flags & FTIME) {
			chronos.flags |= FMOVETIME;
			if (chronos.movestogo)
				chronos.movetime = chronos.time[position.ColorUs()] / chronos.movestogo;
			else
				chronos.movetime = chronos.time[position.ColorUs()] / 32 + chronos.inc[position.ColorUs()] / 2;
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
		bool isValue = UciValue(split, "value", value);
		if (isValue && UciValue(split, "name", name))
			if (name == "ponder")
				options.ponder = value == "true";
			else if (name == "hash")
				tt.Resize(stoi(value));
			else if (name == "contempt")
				options.contempt = stoi(value) * 10;
			else if (StrToLower(name) == "uci_elo") {
				options.elo = stoi(value);
				EvalInit();
			}
	}
	else if (command == "bench") {
		if (UciValue(split, "bench", value))
			UciBench(stoi(value));
		else
			UciBench(8);
	}
	else if (command == "perft") {
		if (UciValue(split, "perft", value))
			UciPerft(stoi(value));
		else
			UciPerft(6);
	}
	else if (command == "eval")
		UciEval();
}

//Main uci loop
void UciLoop() {
	position.SetFen();
	//Picker picker;
	//picker.Pick();
	//position.SetFen("8/8/6r1/K3k3/8/8/2B5/8 w - - 0 79");
	//UciEval();
	//std::cout << position << std::endl;
	//MoveList moves(position,WHITE);
	//for (Move m : moves)std::cout << "     " << m << " iscapture: " << m.is_capture() << " flags: " << m.flags() << std::endl;

	//UciCommand("position fen 4rb1k/1bqn1pp1/p3rn1p/1p2pN2/1PP1p1N1/P1P2Q1P/1BB2PP1/3RR1K1 w - - 0 1");
	//UciCommand("position startpos moves f2f4 d7d5 d2d3 e7e6 e2e4 f7f6 g2g3 e8f7 f1g2 f6f5 e4e5 g8h6 g1f3 f7g8 a2a3 c7c6 a1a2 d8b6");
	//UciCommand("go wtime 52686 btime 49665 winc 0 binc 0");

	//UciCommand("position startpos moves c2c4 c7c5 g1f3 g8f6 d2d4 d7d5 c4d5 c5d4 d1a4 d8d7 a4d4 f6d5 e2e4 d5f6 b1c3 d7d4 f3d4 e7e5 d4b5 b8a6 c1g5 f8c5 g5f6 g7f6 c3d5 e8f8 d5f6 f8g7 f6d5 h8d8 f1c4 f7f5 e4f5 c8f5 e1g1 a6b4 a1d1 f5g4 d1d2 a8c8 c4b3 b4d5 b3d5 c5b4 b5c3 c8c5 f2f3 g4e6 d5e6 d8d2 c3e4 d2b2 e4c5 b4c5 g1h1 b2d2 a2a4 g7f6 e6b3 c5e3 f1d1 d2b2 b3d5 h7h6 h2h3 b7b5 a4b5 b2b5 d5e4 b5b2 d1d7 f6e6 d7g7 a7a5 g7g6 e6e7 e4f5 e7f7 g6a6 e3b6 h1h2 f7f6 f5e4 f6e6 h3h4 e6d6 h4h5 d6e7 h2g3 e7d6 f3f4 b2b3 g3g4 e5f4 g4f4 d6c5 e4f3 b3b2 f4f5 b2d2 a6a8 d2d8 a8d8 b6d8 f3d1 c5b4 f5g6 d8g5 d1f3 a5a4 f3d5 a4a3 g6f7 b4c3 f7g6 c3b2 g6f5 a3a2 d5a2 b2a2 f5g6 a2b3 g6f5 b3c4 f5e4 g5d2 g2g3 c4c3 g3g4 d2g5 e4f3 c3d3 f3f2 d3e4 f2g3 e4e3 g3g2 e3f4 g2f1 f4g4 f1f2 g4h5 f2g3 h5g6 g3f2 h6h5 f2e2 g6f5 e2f1 h5h4 f1e2 h4h3 e2f2 g5h4 f2g1 h4g3 g1f1 f5g4 f1g1 g3e1 g1h2 e1h4 h2h1 h4f2 h1h2 f2e1 h2g1 e1g3 g1h1 g3e5 h1g1 e5f4 g1h1 g4f3 h1g1 f3g3 g1h1 g3g4 h1g1 f4e3 g1h1 e3d4 h1h2 d4f2 h2h1 f2h4 h1g1 h4f6 g1h2 f6d4 h2h1 d4a1 h1h2 a1f6 h2g1 f6e5 g1h1 g4f5 h1g1 e5c7 g1f2 f5f4 f2g1 c7b6 g1h2 f4g4 h2h1 b6d8 h1g1 d8c7 g1h1 g4f5 h1g1 c7d6 g1f2 f5e4 f2g1 e4f3 g1h1 d6e5 h1g1 e5g3 g1h1 f3e4 h1g1 g3d6 g1h1 e4f3 h1g1 d6e5 g1h1 f3e4 h1g1 e4e3 g1f1 e5h2 f1e1 h2f4 e1f1 h3h2 f1g2 f4b8");
	//UciCommand("go movetime 1000");
	//UciCommand("go depth 8");
	string line;
	while (true) {
		getline(cin, line);
		UciCommand(line);
	}
}