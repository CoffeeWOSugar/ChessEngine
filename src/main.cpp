#include <iostream>
#include <string>
#include <vector>
#include "position.h"
#include "movegen.h"
#include "../tests/perft_tests.h"
#include "utils.h"
#include "search.h"

int parseSquare(const std::string& s) {
    if (s.size() != 2) return -1;
    char fileChar = s[0];
    char rankChar = s[1];

    if (fileChar < 'a' || fileChar > 'h') return -1;
    if (rankChar < '1' || rankChar > '8') return -1;

    int file = fileChar - 'a';
    int rank = rankChar - '1';
    return Position::makeSquare(file, rank);
}

int promotionFromChar(char c, Color side) {
    switch (c) {
        case 'q': case 'Q': return (side == WHITE ? WQ : BQ);
        case 'r': case 'R': return (side == WHITE ? WR : BR);
        case 'b': case 'B': return (side == WHITE ? WB : BB);
        case 'n': case 'N': return (side == WHITE ? WN : BN);
        default: return EMPTY;
    }
}

bool chooseAIMove(Position& pos, Move& outMove) {
  const int MAX_DEPTH = 10;
  const int THINK_TIME_MS = 2000;

  SearchLimits limits;
  limits.useTime = true;
  limits.endTime = std::chrono::steady_clock::now() +
                   std::chrono::milliseconds(THINK_TIME_MS);

  return searchBestMove(pos, MAX_DEPTH, limits, outMove);
}

bool parseUserMove(const Position& pos, const std::string& input, Move& outMove) {
    if (input.size() < 4) return false;

    std::string fromStr = input.substr(0, 2);
    std::string toStr   = input.substr(2, 2);

    int fromSq = parseSquare(fromStr);
    int toSq   = parseSquare(toStr);
    if (fromSq == -1 || toSq == -1) return false;

    int promoPiece = EMPTY;
    if (input.size() >= 5) {
        promoPiece = promotionFromChar(input[4], pos.sideToMove);
        if (promoPiece == EMPTY) return false;
    }

    std::vector<Move> moves;
    GenerateLegalMoves(pos, moves);

    for (const Move& m : moves) {
        if (m.from == fromSq && m.to == toSq) {
            // Handle promotions if present
            if ((m.flags & MF_PROMOTION) != 0) {
                if (promoPiece == EMPTY) continue; // promotion required but not given
                if (m.promotion != promoPiece) continue;
            } else {
                if (promoPiece != EMPTY) continue; // user gave promotion on non-promo move
            }
            outMove = m;
            return true;
        }
    }

    return false;
}


int main(int argc, char* argv[]) {
  if (argc > 1 && std::string(argv[1]) == "--run-tests") {
    run_perft_tests();
    return 0;
  }

  Position pos;
  pos.setStartPosition();
  
  // Choose side
  Color humanColor = WHITE;
  while (true) {
    std::cout << "Play as (w)hite or (b)lack?" << std::endl;
    std::string line;

    if (!std::getline(std::cin, line)) return 0;
    if (line.empty()) continue;

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

  std::cout << "Starting game. You are " << (humanColor == WHITE ? "WHITE" : "BLACK") << "." << std::endl;

  // Main Game Loop
  while (true) {
    printBoard(pos, true);
 
    std::vector<Move> moves;
    GenerateLegalMoves(pos, moves);

    if (moves.empty()) {
      // Game over: Checkmate or stalemate
      bool in_check = pos.inCheck(pos.sideToMove);
      if (in_check) {
        std::cout << "Checkmate! " << (pos.sideToMove == humanColor ? "You lose." : "You win!") << std::endl;
      } else {
        std::cout << "Stalemate. Draw" << std::endl;
      }
      break;
    }

    bool humanToMove = (pos.sideToMove == humanColor);

    if (humanToMove) {
      std::cout << "Your move (e.g., e2e4, e7e8q for promtion, 'quit' to exit): ";
      std::string input;
      if (!std::getline(std::cin, input)) break;

      if (input == "quit" || input == "exit") {
        std::cout << "Goodbye!" << std::endl;
        break;
      }

      Move m{};
      if (!parseUserMove(pos, input, m)) {
        std::cout << "Invalid move. Try again." << std::endl;
        continue;
      }

      if (!pos.makeMove(m)) {
        // Should not happen if GenerateLegalMoves is used, but just in case
        std::cout << "Move was rejected as illegal. Try again." << std::endl;
        continue;
      }
    } else {
      std::cout << "Engine thinking..." << std::endl;

      Move m{};
      if (!chooseAIMove(pos, m)) {
        std::cout << "No moves found for engine (this should not happen here)." << std::endl;
        break;
      }

    pos.makeMove(m);
    std::cout << "Engine plays: " << MoveToString(m) << std::endl;

    }
  }

  return 0;
}
