#pragma once
#include <cstdint>
#include <string>

using u64 = unsigned long long;

struct PerftCase {
    std::string name; // description e.g. "Startpos d1-7"
    int depth;
    u64 expected;
    // later add: std::string fen;
};

void run_perft_tests();

