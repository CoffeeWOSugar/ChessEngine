#pragma once
#include "position.h"
#include <iostream>

inline const char *pieceToChar(int p) {
	switch (p) {
	case WP:
		return "P";
	case WN:
		return "N";
	case WB:
		return "B";
	case WR:
		return "R";
	case WQ:
		return "Q";
	case WK:
		return "K";

	case BP:
		return "p";
	case BN:
		return "n";
	case BB:
		return "b";
	case BR:
		return "r";
	case BQ:
		return "q";
	case BK:
		return "k";

	case EMPTY:
		return ".";
	default:
		return "?"; // for debugging weird values
	}
}

inline const char *pieceToSymbol(int p) {
	switch (p) {
	case WP:
		return u8"♙";
	case WN:
		return u8"♘";
	case WB:
		return u8"♗";
	case WR:
		return u8"♖";
	case WQ:
		return u8"♕";
	case WK:
		return u8"♔";

	case BP:
		return u8"♟";
	case BN:
		return u8"♞";
	case BB:
		return u8"♝";
	case BR:
		return u8"♜";
	case BQ:
		return u8"♛";
	case BK:
		return u8"♚";

	case EMPTY:
		return u8".";
	default:
		return u8"?"; // for debugging weird values
	}
}

inline const char *pieceToRepresentation(int p, bool symbol) {
	if (symbol)
		return pieceToSymbol(p);
	return pieceToChar(p);
}

inline void printBoard(const Position &pos, bool symbol) {
	std::cout << "\n  +-----------------+\n";

	for (int rank = 7; rank >= 0; --rank) {
		std::cout << rank + 1 << " |";
		for (int file = 0; file < 8; ++file) {
			int sq = Position::makeSquare(file, rank);
			int p = pos.board[sq];
			std::cout << " " << pieceToRepresentation(p, symbol);
		}
		std::cout << " |\n";
	}

	std::cout << "  +-----------------+\n";
	std::cout << "		a b c d e f g h\n";

	// Optional extra info:
	std::cout << "Side to move: " << (pos.sideToMove == WHITE ? "White" : "Black") << "\n";

	std::cout << "Castling rights: ";
	if (pos.castlingRights == 0)
		std::cout << "-";
	else {
		if (pos.castlingRights & WK_CASTLE)
			std::cout << "K";
		if (pos.castlingRights & WQ_CASTLE)
			std::cout << "Q";
		if (pos.castlingRights & BK_CASTLE)
			std::cout << "k";
		if (pos.castlingRights & BQ_CASTLE)
			std::cout << "q";
	}
	std::cout << "\n";

	std::cout << "EP square: ";
	if (pos.epSquare == -1) {
		std::cout << "-\n";
	} else {
		int file = pos.epSquare & 7;
		int rank = pos.epSquare >> 4;
		char fileChar = 'a' + file;
		char rankChar = '1' + rank;
		std::cout << fileChar << rankChar << " (" << pos.epSquare << ")\n";
	}

	std::cout << std::endl;
}
