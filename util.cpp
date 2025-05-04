#include "util.h"

using namespace std;

std::string trim(const std::string& s)
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
    vStr.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));
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
    vInt.push_back(stoi(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1)));
}

// Function to put thousands
// separators in the given integer
string thousandSeparator(uint64_t n)
{
    string ans = "";

    // Convert the given integer
    // to equivalent string
    string num = to_string(n);

    // Initialise count
    int count = 0;

    // Traverse the string in reverse
    for (int i = num.size() - 1;
        i >= 0; i--) {
        count++;
        ans.push_back(num[i]);

        // If three characters
        // are traversed
        if (count == 3) {
            ans.push_back(' ');
            count = 0;
        }
    }

    // Reverse the string to get
    // the desired output
    reverse(ans.begin(), ans.end());

    // If the given string is
    // less than 1000
    if (ans.size() % 4 == 0) {

        // Remove ','
        ans.erase(ans.begin());
    }

    return ans;
}

string StrToLower(string s) {
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}