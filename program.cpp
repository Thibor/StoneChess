#include "program.h"

SSearchInfo info;

using namespace std;

string trim(const string& s)
{
    if (s.empty())
        return s;
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) {
        start++;
    }

    auto end = s.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
}

void SplitString(const std::string& txt, std::vector<std::string>& vStr, char ch) {
    vStr.clear();
    if (txt == "")
        return;
    size_t pos = txt.find(ch);
    size_t initialPos = 0;

    // Decompose statement
    while (pos != std::string::npos) {
        vStr.push_back(txt.substr(initialPos, pos - initialPos));
        initialPos = pos + 1;

        pos = txt.find(ch, initialPos);
    }

    // Add the last one
    vStr.push_back(txt.substr(initialPos, min(pos, txt.size()) - initialPos + 1));
}

void SplitInt(const string& txt, vector<int>& vInt, char ch) {
    vInt.clear();
    if (txt == "")
        return;
    size_t pos = txt.find(ch);
    size_t initialPos = 0;

    // Decompose statement
    while (pos != std::string::npos) {
        vInt.push_back(stoi(txt.substr(initialPos, pos - initialPos)));
        initialPos = pos + 1;

        pos = txt.find(ch, initialPos);
    }

    // Add the last one
    vInt.push_back(stoi(txt.substr(initialPos, min(pos, txt.size()) - initialPos + 1)));
}

string StrToLower(string s) {
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

static void PrintWelcome() {
	cout << NAME << " " <<VERSION << endl;
}

U64 GetTimeMs() {
#ifdef WIN32
	return GetTickCount64();
#else
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec * 1000 + t.tv_usec / 1000;
#endif
}

U64 ElapsedMs() {
	return GetTimeMs() - info.timeStart;
}

//check if we need to stop the search
bool CheckUp() {
	string input;
	if (GetInput(input))
		UciCommand(input);
	if ((info.timeLimit && GetTimeMs() - info.timeStart > info.timeLimit) || (info.nodesLimit && info.nodes > info.nodesLimit))
		info.stop = true;
	return info.stop;
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
