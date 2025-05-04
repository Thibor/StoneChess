#pragma once
#include <iostream>
#include <string>
#include <chrono>

#include "position.h"
#include "tables.h"
#include "types.h"
#include "uci.h"
#include "input.h"

extern string engineName;
void PrintSummary(uint64_t time, uint64_t nodes);