#pragma once

#include "move.h"
#include "position.h"
#include <chrono>

struct SearchLimits {
	bool useTime = false;
	std::chrono::steady_clock::time_point endTime;
};

int evaluateMaterial(const Position &pos);

// Negamax alpha–beta with time limit support
int alphaBeta(Position &pos, int depth, int alpha, int beta, const SearchLimits &limits,
              bool &timeUp);

// Iterative deepening root search with time limits
bool searchBestMove(Position &pos, int maxDepth, const SearchLimits &limits, Move &bestMove);
