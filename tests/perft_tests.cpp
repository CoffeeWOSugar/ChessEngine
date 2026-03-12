#include "perft_tests.h"
#include "../src/position.h"
#include "../src/perft.h"
#include <iostream>
#include <vector>

void run_perft_tests() {
	std::cout << "Running Perft tests..." << std::endl;

	// Node counts from https://www.chessprogramming.org/Perft_Results
	std::vector<PerftCase> cases = {
	    // Position 1: start position
	    {"Startpos", "", 1, 20ULL},
	    {"Startpos", "", 2, 400ULL},
	    {"Startpos", "", 3, 8902ULL},
	    {"Startpos", "", 4, 197281ULL},
	    {"Startpos", "", 5, 4865609ULL},
	    {"Startpos", "", 6, 119060324ULL},
	    {"Startpos", "", 7, 3195901860ULL},

	    // Position 2: Kiwipete — stresses castling, en passant, and promotions
	    {"Kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 1,
	     48ULL},
	    {"Kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 2,
	     2039ULL},
	    {"Kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 3,
	     97862ULL},
	    {"Kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 4,
	     4085603ULL},
	    {"Kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 5,
	     193690690ULL},

	    // Position 3: endgame with en passant and passed pawns
	    {"Position3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 1, 14ULL},
	    {"Position3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 2, 191ULL},
	    {"Position3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 3, 2812ULL},
	    {"Position3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 4, 43238ULL},
	    {"Position3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 5, 674624ULL},
	    {"Position3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 6, 11030083ULL},

	    // Position 5: stresses promotions
	    {"Position5", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -", 1, 44ULL},
	    {"Position5", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -", 2, 1486ULL},
	    {"Position5", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -", 3, 62379ULL},
	    {"Position5", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -", 4, 2103487ULL},
	    {"Position5", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -", 5, 89941194ULL},

	    // Position 6: complex middlegame
	    {"Position6", "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -", 1,
	     46ULL},
	    {"Position6", "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -", 2,
	     2079ULL},
	    {"Position6", "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -", 3,
	     89890ULL},
	    {"Position6", "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -", 4,
	     3894594ULL},
	};

	bool all_good = true;

	for (const auto &tc : cases) {
		Position pos;

		if (tc.fen.empty())
			pos.setStartPosition();
		else
			pos.setFromFEN(tc.fen);

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
