// engine_session.h
#pragma once
#include <string>
#include <vector>
#include "position.h"
#include "movegen.h"
#include "search.h"

enum class GameResult { ONGOING, CHECKMATE, STALEMATE };

struct EngineConfig {
	int maxDepth = 10;
	int thinkTimeMs = 2000;
};

class EngineSession {
  public:
	EngineSession(const EngineConfig &cfg = EngineConfig()) : config(cfg) {
		pos.setStartPosition();
		humanColor = WHITE;
	}

	void newGame(Color humanSide) {
		pos.setStartPosition();
		humanColor = humanSide;
	}

	const Position &position() const { return pos; }

	Color sideToMove() const { return pos.sideToMove; }
	Color getHumanColor() const { return humanColor; }

	GameResult getGameResult() const {
		std::vector<Move> moves;
		GenerateLegalMoves(pos, moves);
		if (!moves.empty())
			return GameResult::ONGOING;
		bool inCheck = pos.inCheck(pos.sideToMove);
		return inCheck ? GameResult::CHECKMATE : GameResult::STALEMATE;
	}

	// Parse "e2e4", "e7e8q" into a legal Move and apply it
	bool applyHumanMove(const std::string &moveStr, Move &appliedMove, std::string &error);

	// Search and apply engine move
	bool applyEngineMove(Move &appliedMove);

  private:
	EngineConfig config;
	Position pos;
	Color humanColor;

	int parseSquare(const std::string &s) const;
	int promotionFromChar(char c, Color side) const;
};
