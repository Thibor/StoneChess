#include "uci.h"

using namespace std;

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

//Get next word after uci command
bool UciValues(vector<string> list, string command, string& value) {
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
	Value score = ShowEval();
	printf("score %d\n", score);
}

void UciTest() {
	position.SetFen("r4rk1/ppp2ppp/1nnb4/8/1P1P3q/PBN1B2P/4bPP1/R2QR1K1 w - - 0 1");
	chronos.Reset();
	//chronos.flags |= FINFINITE;
	chronos.flags |= FMOVETIME;
	chronos.movetime = 1000;
	SearchIterate();
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
		int delta = 50;
		puts("id name StoneChess");
		puts("id author Thibor Raven");
		printf("option name hash type spin default 64 min 1 max 1024\n");
		printf("option name UCI_Elo type spin default %d min %d max %d\n", options.eloMax, options.eloMin, options.eloMax);
		printf("option name LMR type spin default %d min %d max %d\n", options.lmr, options.lmr - delta, options.lmr + delta);
		printf("option name futility type spin default %d min %d max %d\n", options.futility, options.futility - delta, options.futility + delta);
		printf("option name razoring type spin default %d min %d max %d\n", options.razoring, options.razoring - delta, options.razoring + delta);
		printf("option name null type spin default %d min %d max %d\n", options.nullMove, options.nullMove - delta, options.nullMove + delta);
		printf("option name rfp type spin default %d min %d max %d\n", options.rfp, options.rfp - delta, options.rfp + delta);
		printf("option name contempt type spin default %d min %d max %d\n", options.contempt, options.contempt - delta, options.contempt + delta);
		printf("option name aspiration type spin default %d min 16 max 128\n", options.aspiration);
		printf("option name ponder type check default %s\n", options.ponder ? "true" : "false");
		printf("option name material type string default %s\n", options.materialDel.c_str());
		printf("option name outside_rank type string default %s\n", options.outsideRank.c_str());
		printf("option name outside_file type string default %s\n", options.outsideFile.c_str());
		printf("option name mobility type string default %s\n", options.mobility.c_str());
		printf("option name passed type string default %s\n", options.passed.c_str());
		printf("option name pawn type string default %s\n", options.pawn.c_str());
		printf("option name rook type string default %s\n", options.rook.c_str());
		printf("option name king type string default %s\n", options.king.c_str());
		printf("option name outpost type string default %s\n", options.outpost.c_str());
		printf("option name pair type string default %s\n", options.pair.c_str());
		printf("option name tempo type string default %s\n", options.tempo.c_str());
		puts("uciok");
	}
	else if (command == "isready")
		puts("readyok");
	else if (command == "ucinewgame") {
		tt.Clear();
		InitEval();
		InitSearch();
	}
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
		bool isValue = UciValues(split, "value", value);
		if (isValue && UciValue(split, "name", name)) {
			if (name == "ponder")
				options.ponder = value == "true";
			else if (name == "hash")
				tt.Resize(stoi(value));
			else if (name == "contempt")
				options.contempt = stoi(value) * 10;
			else if (StrToLower(name) == "aspiration")
				options.aspiration = stoi(value);
			else if (StrToLower(name) == "uci_elo")
				options.elo = stoi(value);
			else if (StrToLower(name) == "lmr")
				options.lmr = stoi(value);
			else if (StrToLower(name) == "futility")
				options.futility = stoi(value);
			else if (StrToLower(name) == "razoring")
				options.razoring = stoi(value);
			else if (StrToLower(name) == "null")
				options.nullMove = stoi(value);
			else if (StrToLower(name) == "rfp")
				options.rfp = stoi(value);
			else if (StrToLower(name) == "material")
				options.materialDel = value;
			else if (StrToLower(name) == "outside_rank")
				options.outsideRank = value;
			else if (StrToLower(name) == "outside_file")
				options.outsideFile = value;
			else if (StrToLower(name) == "mobility")
				options.mobility = value;
			else if (StrToLower(name) == "passed")
				options.passed = value;
			else if (StrToLower(name) == "pawn")
				options.pawn = value;
			else if (StrToLower(name) == "rook")
				options.rook = value;
			else if (StrToLower(name) == "king")
				options.king = value;
			else if (StrToLower(name) == "outpost")
				options.outpost = value;
			else if (StrToLower(name) == "pair")
				options.pair = value;
			else if (StrToLower(name) == "tempo")
				options.tempo = value;
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
	else if (command == "test")
		UciTest();
}

//main uci loop
void UciLoop() {
	string line;
	while (true) {
		getline(cin, line);
		UciCommand(line);
	}
}