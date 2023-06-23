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
	S32 score = ShowEval();
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
	//Picker picker;
	// picker.Fill();
	//picker.Sort();
	/*for (int n = 0; n < picker.count; n++)
	{
		PickerE pe = picker.scores[n];
		cout << pe.move.ToUci() << " " << pe.score << " " << pe.see << endl;
	}*/
	//position.SetFen("8/3n2pp/8/8/7P/3k1P2/1pp2PK1/2N5 b - - 2 55");
	//UciEval();
	//std::cout << position << std::endl;
	//MoveList moves(position,WHITE);
	//for (Move m : moves)std::cout << "     " << m << " iscapture: " << m.is_capture() << " flags: " << m.flags() << std::endl;

	//UciCommand("position fen rnbq2k1/pppppppp/8/8/8/8/PPPPPPPP/RNBQ2K1 w KQkq - 0 1 ");
	//UciCommand("position fen 1b1rr1k1/3q1pp1/8/NP1p1b1p/1B1Pp1n1/PQR1P1P1/4BP1P/5RK1 w - - 0 1");
	//UciCommand("position startpos moves f2f4 d7d5 d2d3 e7e6 e2e4 f7f6 g2g3 e8f7 f1g2 f6f5 e4e5 g8h6 g1f3 f7g8 a2a3 c7c6 a1a2 d8b6");
	//UciCommand("go wtime 52686 btime 49665 winc 0 binc 0");

//UciCommand("position startpos moves e2e4 g8f6 e4e5 f6d5 d2d4 d7d6 g1f3 b8c6 f1b5 a7a6 b5c6 b7c6 e1g1 c8f5 a2a3 e7e6 d1e2 f8e7 c2c4 d5b6 b1c3 e8g8 c1e3 d6d5 b2b3 a6a5 c4c5 b6d7 a1e1 f7f6 e2d2 a8b8 b3b4 a5b4 a3b4 f5g4 e5f6 e7f6 c3e2 g4f3 g2f3 d8e8 e3f4 b8b7 b4b5 d7b8 b5b6 c7b6 f4d6 f6e7 e2f4 e7d6 c5d6 f8f6 d2e3 b7d7 f4e6 d7d6 e6c7 e8d7 e3e8 f6f8 e8d7 b8d7 e1e7 d6g6 g1h1 f8f7 e7f7 g8f7 f1a1 g6f6 h1g2 f6f4 a1c1 f4d4 c1c6 d4c4 c7e6 f7f6 c6c4 d5c4 e6c7 f6e5 c7b5 e5d5 g2h1 d5c5 b5c3 b6b5 c3e4 c5d4 e4d6 b5b4 d6b5 d4d3 b5c7 b4b3 h1g2 c4c3 c7d5 c3c2 d5f4 d3c4 f4e2 b3b2 h2h4 c4d3 e2c1");
	//UciCommand("go movetime 1000");
	//UciCommand("go depth 1");

	//CMoveList moves(position,BLACK);
	//for (Move m : moves)std::cout << "     " << m << " iscapture: " << m.IsCapture() << " flags: " << m.flags() << std::endl;
	/*for (int n = 0;n < 8;n++) {
		int r = 3 - floor(abs(n - 3.5));
		cout <<r << endl;
	}*/
	string line;
	while (true) {
		getline(cin, line);
		UciCommand(line);
	}
}