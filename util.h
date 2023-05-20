#pragma once
#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <vector> 

std::string trim(const std::string& s);
std::vector<std::string>& splitString(const std::string& txt, std::vector<std::string>& strs, char ch);
std::string thousandSeparator(uint64_t n);
std::string StrToLower(std::string s);
