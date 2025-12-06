#pragma once
#include "move.h"
#include "types.h"
#include <array>
#include <vector>

static constexpr int KnightOffsets[8] = {31, 33, 18, 14, -31, -33, -18, -14};
static constexpr int BishopOffsets[4] = {17, 15, -17, -15};
static constexpr int RookOffsets[4] = {16, -16, 1, -1};
static constexpr int KingOffsets[8] = {16, -16, 1, -1, 17, 15, -17, -15};

enum CastleRights {
	WK_CASTLE = 1 << 0,
	WQ_CASTLE = 1 << 1,
	BK_CASTLE = 1 << 2,
	BQ_CASTLE = 1 << 3
};

class Position {
  public:
	std::array<int, 128> board{};
	Color sideToMove = WHITE;
	int castlingRights = WK_CASTLE | WQ_CASTLE | BK_CASTLE | BQ_CASTLE;
	int epSquare = -1;
	int halfmoveClock = 0;
	int fullmoveNumber = 1;

	struct State {
		int castlingRights;
		int epSquare;
		int halfmoveClock;
		int fullmoveNumber;
		int capturedPiece;
		Move move;
	};

	std::vector<State> stateStack;

	Position();

	void setStartPosition();

	static inline bool isOnBoard(int sq) { return !(sq & 0x88); }

	static constexpr int makeSquare(int file, int rank) { return (rank << 4) | file; }

	int pieceAt(int sq) const { return board[sq]; }

	void setPiece(int sq, int piece) { board[sq] = piece; }

	bool makeMove(const Move &m); // return false if illegal
	void undoMove();              // undo last move

	bool isSquareAttacked(int sq, Color by) const;

	bool inCheck(Color c) const; // might be needed for legal move generation
};
