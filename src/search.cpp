#include "search.h"
#include "movegen.h"
#include <vector>
#include <limits>
#include <algorithm>
#include <iostream>

static const int PAWN_VALUE = 100;
static const int KNIGHT_VALUE = 320;
static const int BISHOP_VALUE = 330;
static const int ROOK_VALUE = 500;
static const int QUEEN_VALUE = 900;

static const int MATE_SCORE = 100000;
static const int MATE_IN_MAX = MATE_SCORE - 1000; // reserved if needed later

int evaluateMaterial(const Position &pos) {
	int score = 0;

	for (int sq = 0; sq < 128; ++sq) {
		if (sq & 0x88) {
			sq += 7;
			continue;
		}
		int p = pos.board[sq];
		switch (p) {
		case WP:
			score += PAWN_VALUE;
			break;
		case WN:
			score += KNIGHT_VALUE;
			break;
		case WB:
			score += BISHOP_VALUE;
			break;
		case WR:
			score += ROOK_VALUE;
			break;
		case WQ:
			score += QUEEN_VALUE;
			break;
		case WK:
			break; // king value handled via mate scores

		case BP:
			score -= PAWN_VALUE;
			break;
		case BN:
			score -= KNIGHT_VALUE;
			break;
		case BB:
			score -= BISHOP_VALUE;
			break;
		case BR:
			score -= ROOK_VALUE;
			break;
		case BQ:
			score -= QUEEN_VALUE;
			break;
		case BK:
			break;
		}
	}

	// Score from POV of side to move
	if (pos.sideToMove == WHITE)
		return score;
	else
		return -score;
}

int alphaBeta(Position &pos, int depth, int alpha, int beta, const SearchLimits &limits,
              bool &timeUp) {
	// Time check at node entry
	if (limits.useTime) {
		auto now = std::chrono::steady_clock::now();
		if (now >= limits.endTime) {
			timeUp = true;
			return 0; // value will be ignored by caller when timeUp is true
		}
	}

	if (depth == 0) {
		return evaluateMaterial(pos);
	}

	std::vector<Move> moves;
	GenerateLegalMoves(pos, moves);

	if (moves.empty()) {
		if (pos.inCheck(pos.sideToMove)) {
			// Side to move is checkmated -> very bad for them.
			return -MATE_SCORE;
		} else {
			// Stalemate = draw
			return 0;
		}
	}

	int bestScore = std::numeric_limits<int>::min();

	// Simple move ordering: captures first
	std::stable_sort(moves.begin(), moves.end(), [](const Move &a, const Move &b) {
		bool ca = (a.flags & MF_CAPTURE) != 0;
		bool cb = (b.flags & MF_CAPTURE) != 0;
		return ca > cb;
	});

	for (const Move &m : moves) {
		if (!pos.makeMove(m))
			continue;

		int score = -alphaBeta(pos, depth - 1, -beta, -alpha, limits, timeUp);

		pos.undoMove();

		if (timeUp) {
			// Time is up; abort search in this branch
			return 0;
		}

		if (score > bestScore) {
			bestScore = score;
		}
		if (bestScore > alpha) {
			alpha = bestScore;
		}
		if (alpha >= beta) {
			// Beta cutoff
			break;
		}
	}

	return bestScore;
}

bool searchBestMove(Position &pos, int maxDepth, const SearchLimits &limits, Move &bestMove) {
	std::vector<Move> moves;
	GenerateLegalMoves(pos, moves);
	if (moves.empty())
		return false;

	// Root move ordering: captures first
	std::stable_sort(moves.begin(), moves.end(), [](const Move &a, const Move &b) {
		bool ca = (a.flags & MF_CAPTURE) != 0;
		bool cb = (b.flags & MF_CAPTURE) != 0;
		return ca > cb;
	});

	bool timeUp = false;
	bool foundAny = false;
	Move currentBest{};
	int currentBestScore = std::numeric_limits<int>::min();

	// Iterative deepening: 1..maxDepth
	for (int depth = 1; depth <= maxDepth; ++depth) {
		int alpha = std::numeric_limits<int>::min();
		int beta = std::numeric_limits<int>::max();

		int bestScoreThisDepth = std::numeric_limits<int>::min();
		Move bestMoveThisDepth{};

		for (const Move &m : moves) {
			if (!pos.makeMove(m))
				continue;

			int score = -alphaBeta(pos, depth - 1, -beta, -alpha, limits, timeUp);

			pos.undoMove();

			if (timeUp) {
				// Time's up while searching this depth -> discard this partial depth
				// and fall back to the best move from the previous completed depth.
				goto end_search;
			}

			if (score > bestScoreThisDepth) {
				bestScoreThisDepth = score;
				bestMoveThisDepth = m;
			}
			if (score > alpha) {
				alpha = score;
			}
		}

		// Completed this depth fully; update global best
		currentBestScore = bestScoreThisDepth;
		currentBest = bestMoveThisDepth;
		foundAny = true;

		std::cout << "Depth " << depth << " best score = " << currentBestScore << std::endl;
	}

end_search:
	if (!foundAny)
		return false;
	bestMove = currentBest;
	return true;
}
