#include "perft_tests.h"
#include "../src/position.h"
#include "../src/perft.h"
#include <iostream>
#include <string>

void run_perft_tests() {
    Position pos;
    pos.setStartPosition();

    std::cout << "Running Perft tests..." << std::endl;

    auto check = [&](int depth, u64 expected) -> bool {
        u64 nodes = Perft(pos, depth);
        if (nodes != expected) {
            std::cerr << "FAILED: Perft(" << depth << "): expected "
                      << expected << ", got " << nodes << std::endl;
            return false;
        }
        std::cout << "OK: Perft(" << depth << ") = " << nodes << std::endl;
        return true;
    };

    bool all_good = true;
    all_good &= check(1, 20);
    all_good &= check(2, 400);
    all_good &= check(3, 8902);
    all_good &= check(4, 197281);
    all_good &= check(5, 4865609);
    all_good &= check(6, 119060324);

    if (!all_good) {
        std::cerr << "Some Perft tests FAILED!" << std::endl;
    } else {
        std::cout << "All Perft tests passed successfully!" << std::endl;
    }
}

