#pragma once
#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <vector> 

std::string trim(const std::string& s);
void splitString(const std::string& txt, std::vector<std::string>& vStr, char ch);
void splitInt(const std::string& txt, std::vector<int>& vInt, char ch);
std::string thousandSeparator(uint64_t n);
std::string StrToLower(std::string s);
