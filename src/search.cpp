#include "search.h"
#include "movegen.h"
#include <vector>
#include <limits>
#include <algorithm>
#include <iostream>

#ifdef USE_MPI
#include <mpi.h>
#include <cstdint>
#include <cstring>
#endif

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
		currentBest = bestMoveThisDepth;
		foundAny = true;

		// std::cout << "Depth " << depth << " best score = " << currentBestScore << std::endl;
	}

end_search:
	if (!foundAny)
		return false;
	bestMove = currentBest;
	return true;
}

#ifdef USE_MPI

enum class MpiJobType : int { SEARCH = 1, QUIT = 2 };

struct PositionSnapshot {
	int32_t board[128];
	int32_t sideToMove;
	int32_t castlingRights;
	int32_t epSquare;
	int32_t halfmoveClock;
	int32_t fullmoveNumber;
};

static PositionSnapshot makeSnapshot(const Position &p) {
	PositionSnapshot s{};
	for (int i = 0; i < 128; ++i)
		s.board[i] = (int32_t)p.board[i];
	s.sideToMove = (int32_t)p.sideToMove;
	s.castlingRights = (int32_t)p.castlingRights;
	s.epSquare = (int32_t)p.epSquare;
	s.halfmoveClock = (int32_t)p.halfmoveClock;
	s.fullmoveNumber = (int32_t)p.fullmoveNumber;
	return s;
}

static Position fromSnapshot(const PositionSnapshot &s) {
	Position p;
	for (int i = 0; i < 128; ++i)
		p.board[i] = (int)s.board[i];
	p.sideToMove = (Color)s.sideToMove;
	p.castlingRights = (int)s.castlingRights;
	p.epSquare = (int)s.epSquare;
	p.halfmoveClock = (int)s.halfmoveClock;
	p.fullmoveNumber = (int)s.fullmoveNumber;
	p.stateStack.clear(); // critical: workers need a clean undo stack
	return p;
}

static SearchLimits makeLocalTimeLimits(int thinkTimeMs) {
	SearchLimits limits;
	if (thinkTimeMs > 0) {
		limits.useTime = true;
		limits.endTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(thinkTimeMs);
	}
	return limits;
}

// Evaluate only the root moves belonging to this rank: i % worldSize == worldRank.
// depth is the *current iterative deepening depth* at the root (>=1).
static bool evalRootSlice(Position &rootPos, const std::vector<Move> &rootMoves, int depth,
                          const SearchLimits &limits, int worldRank, int worldSize,
                          int &outBestScore, Move &outBestMove) {
	bool timeUp = false;
	bool foundAny = false;

	int bestScore = std::numeric_limits<int>::min();
	Move bestMove{};

	int alpha = std::numeric_limits<int>::min();
	int beta = std::numeric_limits<int>::max();

	for (int i = 0; i < (int)rootMoves.size(); ++i) {
		if ((i % worldSize) != worldRank)
			continue;

		const Move &m = rootMoves[i];
		if (!rootPos.makeMove(m))
			continue;

		int score = -alphaBeta(rootPos, depth - 1, -beta, -alpha, limits, timeUp);

		rootPos.undoMove();

		if (timeUp) {
			// Stop early if local time is up; rank 0 will use the last completed depth
			// behavior via the iterative deepening loop.
			break;
		}

		if (!foundAny || score > bestScore) {
			bestScore = score;
			bestMove = m;
			foundAny = true;
		}
		if (score > alpha)
			alpha = score;
	}

	outBestScore = bestScore;
	outBestMove = bestMove;
	return foundAny;
}

// Rank 0 only: broadcast QUIT to release workers.
void mpiBroadcastQuit() {
	int initialized = 0;
	MPI_Initialized(&initialized);
	if (!initialized)
		return;

	int worldRank = 0;
	MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
	if (worldRank != 0)
		return;

	int job = (int)MpiJobType::QUIT;
	MPI_Bcast(&job, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

// Workers: block forever until QUIT; for SEARCH participate in the same gather/bcast steps.
void mpiWorkerLoop() {
	for (;;) {
		int job = 0;
		MPI_Bcast(&job, 1, MPI_INT, 0, MPI_COMM_WORLD);

		if (job == (int)MpiJobType::QUIT)
			return;
		if (job != (int)MpiJobType::SEARCH)
			continue;

		PositionSnapshot snap{};
		int maxDepth = 0;
		int thinkTimeMs = 0;

		MPI_Bcast(&snap, (int)sizeof(PositionSnapshot), MPI_BYTE, 0, MPI_COMM_WORLD);
		MPI_Bcast(&maxDepth, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&thinkTimeMs, 1, MPI_INT, 0, MPI_COMM_WORLD);

		Position rootPos = fromSnapshot(snap);

		std::vector<Move> moves;
		GenerateLegalMoves(rootPos, moves);

		// Root move ordering must be deterministic across ranks.
		std::stable_sort(moves.begin(), moves.end(), [](const Move &a, const Move &b) {
			bool ca = (a.flags & MF_CAPTURE) != 0;
			bool cb = (b.flags & MF_CAPTURE) != 0;
			return ca > cb;
		});

		int worldRank = 0, worldSize = 1;
		MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
		MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

		for (int depth = 1; depth <= maxDepth; ++depth) {
			SearchLimits limits = makeLocalTimeLimits(thinkTimeMs);

			int localBestScore = std::numeric_limits<int>::min();
			Move localBestMove{};
			bool localFound = evalRootSlice(rootPos, moves, depth, limits, worldRank, worldSize,
			                                localBestScore, localBestMove);

			// Gather results to rank 0.
			// Always send a score; if localFound==false, score is -INF and move is ignored.
			int sendScore = localFound ? localBestScore : std::numeric_limits<int>::min();
			int sendFound = localFound ? 1 : 0;

			MPI_Gather(&sendScore, 1, MPI_INT, nullptr, 0, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Gather(&sendFound, 1, MPI_INT, nullptr, 0, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Gather(&localBestMove, (int)sizeof(Move), MPI_BYTE, nullptr, 0, MPI_BYTE, 0,
			           MPI_COMM_WORLD);

			// Rank 0 decides whether this depth “counts” and broadcasts continuation + best move.
			int cont = 0;
			Move globalBest{};
			MPI_Bcast(&cont, 1, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(&globalBest, (int)sizeof(Move), MPI_BYTE, 0, MPI_COMM_WORLD);

			if (!cont)
				break;
		}
	}
}

// Rank 0: parallel root-split iterative deepening. Other ranks must be in mpiWorkerLoop().
bool searchBestMoveMPI(Position &pos, int maxDepth, int thinkTimeMs, Move &bestMove) {
	int initialized = 0;
	MPI_Initialized(&initialized);
	if (!initialized) {
		// Fallback: not in an MPI run.
		SearchLimits limits = makeLocalTimeLimits(thinkTimeMs);
		return searchBestMove(pos, maxDepth, limits, bestMove);
	}

	int worldRank = 0, worldSize = 1;
	MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

	if (worldRank != 0) {
		// Only rank 0 should call this; others should be in mpiWorkerLoop().
		return false;
	}

	if (worldSize <= 1) {
		SearchLimits limits = makeLocalTimeLimits(thinkTimeMs);
		return searchBestMove(pos, maxDepth, limits, bestMove);
	}

	// Broadcast job header and payload (snapshot).
	int job = (int)MpiJobType::SEARCH;
	MPI_Bcast(&job, 1, MPI_INT, 0, MPI_COMM_WORLD);

	PositionSnapshot snap = makeSnapshot(pos);
	MPI_Bcast(&snap, (int)sizeof(PositionSnapshot), MPI_BYTE, 0, MPI_COMM_WORLD);
	MPI_Bcast(&maxDepth, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&thinkTimeMs, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// Rank 0 builds its own local rootPos from the same snapshot to ensure parity.
	Position rootPos = fromSnapshot(snap);

	std::vector<Move> moves;
	GenerateLegalMoves(rootPos, moves);
	if (moves.empty()) {
		// No legal moves
		int cont = 0;
		Move dummy{};
		// broadcast “stop” once so workers don't hang in the depth loop
		MPI_Bcast(&cont, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&dummy, (int)sizeof(Move), MPI_BYTE, 0, MPI_COMM_WORLD);
		return false;
	}

	std::stable_sort(moves.begin(), moves.end(), [](const Move &a, const Move &b) {
		bool ca = (a.flags & MF_CAPTURE) != 0;
		bool cb = (b.flags & MF_CAPTURE) != 0;
		return ca > cb;
	});

	bool haveGlobal = false;
	Move globalBest{};
	int globalBestScore = std::numeric_limits<int>::min();

	// Buffers for gathered data (rank 0 only)
	std::vector<int> allScores(worldSize);
	std::vector<int> allFound(worldSize);
	std::vector<Move> allMoves(worldSize);

	for (int depth = 1; depth <= maxDepth; ++depth) {
		SearchLimits limits = makeLocalTimeLimits(thinkTimeMs);

		int localBestScore = std::numeric_limits<int>::min();
		Move localBestMove{};
		bool localFound = evalRootSlice(rootPos, moves, depth, limits, worldRank, worldSize,
		                                localBestScore, localBestMove);

		int sendScore = localFound ? localBestScore : std::numeric_limits<int>::min();
		int sendFound = localFound ? 1 : 0;

		MPI_Gather(&sendScore, 1, MPI_INT, allScores.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Gather(&sendFound, 1, MPI_INT, allFound.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Gather(&localBestMove, (int)sizeof(Move), MPI_BYTE, allMoves.data(), (int)sizeof(Move),
		           MPI_BYTE, 0, MPI_COMM_WORLD);

		// Select best across ranks for this depth.
		bool foundThisDepth = false;
		int bestScoreThisDepth = std::numeric_limits<int>::min();
		Move bestMoveThisDepth{};

		for (int r = 0; r < worldSize; ++r) {
			if (!allFound[r])
				continue;
			if (!foundThisDepth || allScores[r] > bestScoreThisDepth) {
				bestScoreThisDepth = allScores[r];
				bestMoveThisDepth = allMoves[r];
				foundThisDepth = true;
			}
		}

		// Decide whether this depth “completed enough” to accept.
		// With your current alphaBeta time checks, some ranks may time out early.
		// We treat “any result found” as acceptable; if you want stricter semantics,
		// require allFound[r]==1 for ranks that own at least one move.
		int cont = 0;
		Move broadcastBest{};

		if (foundThisDepth) {
			globalBest = bestMoveThisDepth;
			globalBestScore = bestScoreThisDepth;
			haveGlobal = true;
			cont = 1;
			broadcastBest = globalBest;
		} else {
			cont = 0;
		}

		MPI_Bcast(&cont, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&broadcastBest, (int)sizeof(Move), MPI_BYTE, 0, MPI_COMM_WORLD);

		if (!cont)
			break;
	}

	if (!haveGlobal)
		return false;
	bestMove = globalBest;
	return true;
}

#endif // USE_MPI
