// main.cpp
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "engine_session.h"
#include "../tests/perft_tests.h"
#include "utils.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Forward declarations
int runCliGame();
int runPerft(int depth);
int runProtocol(); // for web API

int runCliGame() {
	EngineConfig cfg;
	cfg.maxDepth = 10;
	cfg.thinkTimeMs = 2000;

	EngineSession session(cfg);

	// Choose side
	Color humanColor = WHITE;
	while (true) {
		std::cout << "Play as (w)hite or (b)lack?" << std::endl;
		std::string line;
		if (!std::getline(std::cin, line))
			return 0;
		if (line.empty())
			continue;

		char c = std::tolower(line[0]);
		if (c == 'w') {
			humanColor = WHITE;
			break;
		} else if (c == 'b') {
			humanColor = BLACK;
			break;
		} else {
			std::cout << "Please enter 'w' or 'b'." << std::endl;
		}
	}

	session.newGame(humanColor);
	std::cout << "Starting game. You are " << (humanColor == WHITE ? "WHITE" : "BLACK") << "."
	          << std::endl;

	// Game loop
	while (true) {
		printBoard(session.position(), true);

		auto result = session.getGameResult();
		if (result != GameResult::ONGOING) {
			bool in_check = session.position().inCheck(session.position().sideToMove);
			if (in_check) {
				std::cout << "Checkmate! "
				          << (session.sideToMove() == humanColor ? "You lose." : "You win!")
				          << std::endl;
			} else {
				std::cout << "Stalemate. Draw" << std::endl;
			}
			break;
		}

		bool humanToMove = (session.sideToMove() == humanColor);

		if (humanToMove) {
			std::cout << "Your move (e.g., e2e4, e7e8q for promotion, 'quit' to exit): "
			          << std::endl;
			std::string input;
			if (!std::getline(std::cin, input))
				break;

			if (input == "quit" || input == "exit") {
				std::cout << "Goodbye!" << std::endl;
				break;
			}

			Move m{};
			std::string err;
			if (!session.applyHumanMove(input, m, err)) {
				std::cout << "Invalid move: " << err << ". Try again." << std::endl;
				continue;
			}
		} else {
			std::cout << "Engine thinking..." << std::endl;
			Move m{};
			if (!session.applyEngineMove(m)) {
				std::cout << "No moves found for engine (this should not happen here)."
				          << std::endl;
				break;
			}
			std::cout << "Engine plays: " << MoveToString(m) << std::endl;
		}
	}

	return 0;
}

int runPerft(int depth) {
	Position pos;
	pos.setStartPosition();
	// uint64_t nodes = perft(pos, depth);
	// std::cout << nodes << std::endl;
	// TMP
	return depth;
	return 0;
}

static std::string statusToString(GameResult r) {
	switch (r) {
	case GameResult::ONGOING:
		return "ongoing";
	case GameResult::CHECKMATE:
		return "checkmate";
	case GameResult::STALEMATE:
		return "stalemate";
	}
	return "ongoing";
}

static json stateJson(const EngineSession &s) {
	json j;
	j["event"] = "state";
	j["fen"] = s.position().toFEN();
	j["side_to_move"] = (s.sideToMove() == WHITE ? "w" : "b");
	j["status"] = statusToString(s.getGameResult());
	return j;
}

int runProtocol() {
	EngineConfig cfg;
	EngineSession session(cfg);

	std::string line;
	while (std::getline(std::cin, line)) {
		if (line.empty())
			continue;

		json req;
		try {
			req = json::parse(line);
		} catch (...) {
			std::cout << json{{"event", "error"}, {"message", "invalid json"}}.dump() << "\n";
			std::cout.flush();
			continue;
		}

		const std::string cmd = req.value("cmd", "");
		if (cmd == "new-game") {
			std::string hc = req.value("human_color", "w");
			Color human = (hc.size() && (hc[0] == 'b' || hc[0] == 'B')) ? BLACK : WHITE;
			session.newGame(human);

			auto out = stateJson(session);
			std::cout << out.dump() << "\n";
			std::cout.flush();
			continue;
		}

		if (cmd == "move") {
			std::string mv = req.value("move", "");
			Move hm{};
			std::string err;
			if (!session.applyHumanMove(mv, hm, err)) {
				std::cout << json{{"event", "error"}, {"message", err}}.dump() << "\n";
				std::cout.flush();
				continue;
			}

			// If human move ended game, report state immediately.
			if (session.getGameResult() != GameResult::ONGOING) {
				std::cout << stateJson(session).dump() << "\n";
				std::cout.flush();
				continue;
			}

			Move em{};
			if (!session.applyEngineMove(em)) {
				std::cout << json{{"event", "error"}, {"message", "engine failed to move"}}.dump()
				          << "\n";
				std::cout.flush();
				continue;
			}

			auto out = stateJson(session);
			out["engine_move"] = MoveToString(em);
			std::cout << out.dump() << "\n";
			std::cout.flush();
			continue;
		}

		std::cout << json{{"event", "error"}, {"message", "unknown cmd"}}.dump() << "\n";
		std::cout.flush();
	}

	return 0;
}

int main(int argc, char *argv[]) {
	if (argc > 1) {
		std::string arg1 = argv[1];

		if (arg1 == "--run-tests") {
			run_perft_tests();
			return 0;
		}

		if (arg1 == "--perft") {
			if (argc < 3) {
				std::cerr << "Usage: chess --perft <depth>\n";
				return 1;
			}
			int depth = std::stoi(argv[2]);
			return runPerft(depth);
		}

		if (arg1 == "--protocol") {
			return runProtocol();
		}
	}

	// Default: interactive CLI game
	return runCliGame();
}
