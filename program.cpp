#include "program.h"

using namespace std;

schronos chronos;

static void PrintWelcome() {
	cout << NAME << " " <<VERSION << endl;
}

//Displays a summary
void PrintSummary(uint64_t time, uint64_t nodes) {
	if (time < 1)
		time = 1;
	uint64_t nps = (nodes * 1000) / time;
	cout << "-----------------------------" << endl;
	cout << "Time        : " << thousandSeparator(time) << endl;
	cout << "Nodes       : " << thousandSeparator(nodes) << endl;
	cout << "Nps         : " << thousandSeparator(nps) << endl;
	cout << "-----------------------------" << endl;
}

int main() {
	//setbuf(stdout, 0);
	PrintWelcome();
	InitImput();
	InitEval();
	InitSearch();
	initialise_all_databases();
	zobrist::InitialiseZobristKeys();
	g_pos.SetFen();
	UciLoop();
}
