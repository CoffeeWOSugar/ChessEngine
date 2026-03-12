#pragma once
#include <cstdint>
#include <string>

using u64 = unsigned long long;

struct PerftCase {
	std::string name;
	std::string fen; // empty = start position
	int depth;
	u64 expected;
};

void run_perft_tests();
