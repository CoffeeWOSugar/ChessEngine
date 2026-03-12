// engine_session.cpp
#include "engine_session.h"
#include "utils.h" // MoveToString, etc...

#ifdef USE_MPI
#include <mpi.h>
#endif

int EngineSession::parseSquare(const std::string &s) const {
	if (s.size() != 2)
		return -1;
	char fileChar = s[0];
	char rankChar = s[1];
	if (fileChar < 'a' || fileChar > 'h')
		return -1;
	if (rankChar < '1' || rankChar > '8')
		return -1;
	int file = fileChar - 'a';
	int rank = rankChar - '1';
	return Position::makeSquare(file, rank);
}

int EngineSession::promotionFromChar(char c, Color side) const {
	switch (c) {
	case 'q':
	case 'Q':
		return (side == WHITE ? WQ : BQ);
	case 'r':
	case 'R':
		return (side == WHITE ? WR : BR);
	case 'b':
	case 'B':
		return (side == WHITE ? WB : BB);
	case 'n':
	case 'N':
		return (side == WHITE ? WN : BN);
	default:
		return EMPTY;
	}
}

bool EngineSession::applyHumanMove(const std::string &input, Move &outMove, std::string &error) {
	if (input.size() < 4) {
		error = "Move too short";
		return false;
	}

	std::string fromStr = input.substr(0, 2);
	std::string toStr = input.substr(2, 2);

	int fromSq = parseSquare(fromStr);
	int toSq = parseSquare(toStr);
	if (fromSq == -1 || toSq == -1) {
		error = "Invalid square";
		return false;
	}

	int promoPiece = EMPTY;
	if (input.size() >= 5) {
		promoPiece = promotionFromChar(input[4], pos.sideToMove);
		if (promoPiece == EMPTY) {
			error = "Invalid promotion piece";
			return false;
		}
	}

	std::vector<Move> moves;
	GenerateLegalMoves(pos, moves);

	for (const Move &m : moves) {
		if (m.from == fromSq && m.to == toSq) {
			if ((m.flags & MF_PROMOTION) != 0) {
				if (promoPiece == EMPTY)
					continue;
				if (m.promotion != promoPiece)
					continue;
			} else {
				if (promoPiece != EMPTY)
					continue;
			}
			if (!pos.makeMove(m)) {
				error = "Illegal move (makeMove rejected)";
				return false;
			}
			// Record normalised move string (4 chars, or 5 for promotion)
			moveHistory.push_back(input.size() >= 5 ? input.substr(0, 5) : input.substr(0, 4));
			outMove = m;
			return true;
		}
	}

	error = "Move not found in legal moves";
	return false;
}

bool EngineSession::applyEngineMove(Move &outMove) {
	// Consult opening book first
	auto bookMove = lookupOpeningBook(moveHistory);
	if (bookMove) {
		// Validate the book move is actually legal in the current position
		std::string mv = *bookMove;
		std::string fromStr = mv.substr(0, 2);
		std::string toStr = mv.substr(2, 2);
		int fromSq = parseSquare(fromStr);
		int toSq = parseSquare(toStr);
		if (fromSq != -1 && toSq != -1) {
			std::vector<Move> legalMoves;
			GenerateLegalMoves(pos, legalMoves);
			for (const Move &m : legalMoves) {
				if (m.from == fromSq && m.to == toSq) {
					if (pos.makeMove(m)) {
						moveHistory.push_back(mv);
						outMove = m;
						return true;
					}
				}
			}
		}
		// Book move wasn't legal — fall through to search
	}

	Move best{};

#ifdef USE_MPI
	int initialized = 0;
	MPI_Initialized(&initialized);

	int worldSize = 1;
	int worldRank = 0;
	if (initialized) {
		MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
		MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
	}

	// In your architecture, only rank 0 should ever reach here (others are in mpiWorkerLoop()).
	// If not, fail safely.
	if (initialized && worldSize > 1) {
		if (worldRank != 0) {
			return false;
		}

		if (!searchBestMoveMPI(pos, config.maxDepth, config.thinkTimeMs, best)) {
			return false;
		}
	} else
#endif
	{
		SearchLimits limits;
		limits.useTime = true;
		limits.endTime =
		    std::chrono::steady_clock::now() + std::chrono::milliseconds(config.thinkTimeMs);

		if (!searchBestMove(pos, config.maxDepth, limits, best)) {
			return false;
		}
	}

	if (!pos.makeMove(best)) {
		return false;
	}
	moveHistory.push_back(MoveToString(best));
	outMove = best;
	return true;
}
