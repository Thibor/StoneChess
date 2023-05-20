#include "uci.h"

using namespace std;

SOptions options;
SOptions optionsOrg;

//Get next word after uci command
bool UciValue(vector<string> list, string command, string& value) {
	value = "";
	for (int n = 0; n < list.size() - 1; n++)
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
			CMoveList list = GetMoveList();
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

	//UciCommand("position fen 8/4Q3/8/4Q3/4K3/7k/8/8 w - - 9 85");
	//UciCommand("position startpos moves f2f4 d7d5 d2d3 e7e6 e2e4 f7f6 g2g3 e8f7 f1g2 f6f5 e4e5 g8h6 g1f3 f7g8 a2a3 c7c6 a1a2 d8b6");
	//UciCommand("go wtime 52686 btime 49665 winc 0 binc 0");

	//UciCommand("position startpos moves g1f3 e7e6 c2c4 c7c5 g2g3 d7d5 c4d5 e6d5 d2d4 g8f6 b1c3 b8c6 f1g2 f8e7 d4c5 d5d4 c3a4 d8a5 c1d2 a5b5 b2b4 e8g8 a2a3 b7b6 a1c1 c8e6 e1g1 e6d5 f1e1 f6e4 d2f4 b6c5 b4c5 d5b3 d1d3 b5a4 d3e4 a8c8 e2e3 d4e3 e4e3 e7f6 g2h3 c8a8 h3d7 b3a2 c1d1 a2c4 e3e4 c4b5 e4a4 b5a4 d1d6 a8d8 e1e3 a4b5 d7c6 d8d6 f4d6 b5c6 d6f8 g8f8 f3e5 f6e5 e3e5 g7g6 e5e2 h7h5 e2d2 f8e7 d2d6 c6b5 g1g2 g6g5 d6h6 g5g4 h2h3 f7f5 h6h5 e7e6 h3g4 f5g4 h5g5 b5e2 g5g6 e6d7 a3a4 a7a5 g6d6 d7c7 f2f4 g4f3 g2f2 e2c4 f2f3 c4b3 d6a6 b3a4 a6a5 a4d1 f3e4 c7c6 a5a1 d1e2 e4d4 e2f3 a1a6 c6b5 a6a7 b5c6 a7g7 c6b5 g3g4 b5c6 g4g5 f3e2 g7h7 e2g4 h7h6 c6d7 g5g6 g4e6 c5c6 d7c7 d4c5 e6b3 h6h7 c7c8 c5b6 c8d8 c6c7 d8c8 h7h8 c8d7 c7c8q d7e7 g6g7 b3f7 c8c7 e7f6 c7f4 f6e7 f4e5 e7d7 e5c7 d7e6 h8h6 e6f5 c7f7 f5e5 f7e7 e5d4 g7g8q d4c3 h6c6 c3b2 g8e6 b2b1 e6e5 b1a2 c6c5 a2b1 b6c6 b1a2 c6d5 a2a3 d5c4 a3a2 c5a5 a2b1 c4d3 b1c1");
	//UciCommand("go movetime 1000");
	string line;
	while (true) {
		getline(cin, line);
		UciCommand(line);
	}
}