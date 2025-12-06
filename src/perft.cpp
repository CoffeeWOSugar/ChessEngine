#include <iostream>
#include "perft.h"

u64 Perft(Position &pos, int depth) {
	if (depth == 0)
		return 1ULL;

	size_t stack_before = pos.stateStack.size();

	std::vector<Move> moves;
	GeneratePseudoLegalMoves(pos, moves);

	u64 nodes = 0;
	for (const Move &m : moves) {
		if (!pos.makeMove(m))
			continue;
		nodes += Perft(pos, depth - 1);
		pos.undoMove();
	}

	// Sanity check
	if (pos.stateStack.size() != stack_before) {
		std::cerr << "State stack imbalance detected!\n";
		std::abort();
	}
	return nodes;
}

u64 PerftDivide(Position &pos, int depth) {
	std::vector<Move> moves;
	GeneratePseudoLegalMoves(pos, moves);

	u64 total = 0;
	for (const Move &m : moves) {
		if (!pos.makeMove(m))
			continue;

		u64 nodes = Perft(pos, depth - 1);
		pos.undoMove();

		total += nodes;
		std::cout << MoveToString(m) << ": " << nodes << std::endl;
	}
	std::cout << "Total: " << total << std::endl;
	return total;
}
