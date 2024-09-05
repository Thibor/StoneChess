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
	Score score = ShowEval();
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
		printf("option name LMR type spin default %d min %d max %d\n", optionsOrg.lmr, optionsOrg.lmrMin, optionsOrg.lmrMax);
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
			else if (StrToLower(name) == "lmr") {
				options.lmr = stoi(value);
				SearchInit();
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
}

//Main uci loop
void UciLoop() {
	position.SetFen();
	string line;
	while (true) {
		getline(cin, line);
		UciCommand(line);
	}
}