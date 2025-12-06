#include "perft_tests.h"
#include "../src/position.h"
#include "../src/perft.h"
#include <iostream>
#include <vector>

void run_perft_tests() {
	std::cout << "Running Perft tests..." << std::endl;

	// 1) Define test cases
	std::vector<PerftCase> cases = {
	    {"Startpos", 1, 20ULL},         {"Startpos", 2, 400ULL},     {"Startpos", 3, 8902ULL},
	    {"Startpos", 4, 197281ULL},     {"Startpos", 5, 4865609ULL}, {"Startpos", 6, 119060324ULL},
	    {"Startpos", 7, 3195901860ULL},
	};

	bool all_good = true;

	// 2) Loop over cases
	for (const auto &tc : cases) {
		Position pos;

		pos.setStartPosition();

		u64 nodes = Perft(pos, tc.depth);

		if (nodes != tc.expected) {
			std::cerr << "FAILED: [" << tc.name << "] Perft(" << tc.depth << "): expected "
			          << tc.expected << ", got " << nodes << '\n';
			all_good = false;
		} else {
			std::cout << "OK: [" << tc.name << "] Perft(" << tc.depth << ") = " << nodes << '\n';
		}
	}

	if (!all_good) {
		std::cerr << "Some Perft tests FAILED!" << std::endl;
	} else {
		std::cout << "All Perft tests passed successfully!" << std::endl;
	}
}
