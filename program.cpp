#include "program.h"

using namespace std;

#define MONTH (\
  __DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? "01" : "06") \
: __DATE__ [2] == 'b' ? "02" \
: __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? "03" : "04") \
: __DATE__ [2] == 'y' ? "05" \
: __DATE__ [2] == 'l' ? "07" \
: __DATE__ [2] == 'g' ? "08" \
: __DATE__ [2] == 'p' ? "09" \
: __DATE__ [2] == 't' ? "10" \
: __DATE__ [2] == 'v' ? "11" \
: "12")
#define DAY (std::string(1,(__DATE__[4] == ' ' ?  '0' : (__DATE__[4]))) + (__DATE__[5]))
#define YEAR ((__DATE__[7]-'0') * 1000 + (__DATE__[8]-'0') * 100 + (__DATE__[9]-'0') * 10 + (__DATE__[10]-'0') * 1)

string engineName = "StoneChess";

static void PrintWelcome() {
	cout << engineName<<" " << YEAR << "-" << MONTH << "-" << DAY << endl;
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
