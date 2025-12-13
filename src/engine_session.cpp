// engine_session.cpp
#include "engine_session.h"
#include "utils.h" // MoveToString, etc.

int EngineSession::parseSquare(const std::string& s) const {
    if (s.size() != 2) return -1;
    char fileChar = s[0];
    char rankChar = s[1];
    if (fileChar < 'a' || fileChar > 'h') return -1;
    if (rankChar < '1' || rankChar > '8') return -1;
    int file = fileChar - 'a';
    int rank = rankChar - '1';
    return Position::makeSquare(file, rank);
}

int EngineSession::promotionFromChar(char c, Color side) const {
    switch (c) {
        case 'q': case 'Q': return (side == WHITE ? WQ : BQ);
        case 'r': case 'R': return (side == WHITE ? WR : BR);
        case 'b': case 'B': return (side == WHITE ? WB : BB);
        case 'n': case 'N': return (side == WHITE ? WN : BN);
        default: return EMPTY;
    }
}

bool EngineSession::applyHumanMove(const std::string& input, Move& outMove, std::string& error) {
    if (input.size() < 4) {
        error = "Move too short";
        return false;
    }

    std::string fromStr = input.substr(0, 2);
    std::string toStr   = input.substr(2, 2);

    int fromSq = parseSquare(fromStr);
    int toSq   = parseSquare(toStr);
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

    for (const Move& m : moves) {
        if (m.from == fromSq && m.to == toSq) {
            if ((m.flags & MF_PROMOTION) != 0) {
                if (promoPiece == EMPTY) continue;
                if (m.promotion != promoPiece) continue;
            } else {
                if (promoPiece != EMPTY) continue;
            }
            if (!pos.makeMove(m)) {
                error = "Illegal move (makeMove rejected)";
                return false;
            }
            outMove = m;
            return true;
        }
    }

    error = "Move not found in legal moves";
    return false;
}

bool EngineSession::applyEngineMove(Move& outMove) {
    SearchLimits limits;
    limits.useTime = true;
    limits.endTime = std::chrono::steady_clock::now() +
                     std::chrono::milliseconds(config.thinkTimeMs);

    Move best{};
    if (!searchBestMove(pos, config.maxDepth, limits, best)) {
        return false;
    }
    if (!pos.makeMove(best)) {
        return false;
    }
    outMove = best;
    return true;
}

