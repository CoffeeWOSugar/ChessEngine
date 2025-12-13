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
		return "♙";
	case WN:
		return "♘";
	case WB:
		return "♗";
	case WR:
		return "♖";
	case WQ:
		return "♕";
	case WK:
		return "♔";

	case BP:
		return "♟";
	case BN:
		return "♞";
	case BB:
		return "♝";
	case BR:
		return "♜";
	case BQ:
		return "♛";
	case BK:
		return "♚";

	case EMPTY:
		return ".";
	default:
		return "?"; // for debugging weird values
	}
}

inline const char *pieceToRepresentation(int p, bool symbol) {
	if (symbol)
		return pieceToSymbol(p);
	return pieceToChar(p);
}

inline void printBoard(const Position &pos, bool symbol, std::ostream &out = std::cout) {
	out << "\n  +-----------------+\n";

	for (int rank = 7; rank >= 0; --rank) {
		out << rank + 1 << " |";
		for (int file = 0; file < 8; ++file) {
			int sq = Position::makeSquare(file, rank);
			int p = pos.board[sq];
			out << " " << pieceToRepresentation(p, symbol);
		}
		out << " |\n";
	}

	out << "  +-----------------+\n";
	out << "		a b c d e f g h\n";

	// Optional extra info:
	out << "Side to move: " << (pos.sideToMove == WHITE ? "White" : "Black") << "\n";

	out << "Castling rights: ";
	if (pos.castlingRights == 0)
		out << "-";
	else {
		if (pos.castlingRights & WK_CASTLE)
			out << "K";
		if (pos.castlingRights & WQ_CASTLE)
			out << "Q";
		if (pos.castlingRights & BK_CASTLE)
			out << "k";
		if (pos.castlingRights & BQ_CASTLE)
			out << "q";
	}
	out << "\n";

	out << "EP square: ";
	if (pos.epSquare == -1) {
		out << "-\n";
	} else {
		int file = pos.epSquare & 7;
		int rank = pos.epSquare >> 4;
		char fileChar = 'a' + file;
		char rankChar = '1' + rank;
		out << fileChar << rankChar << " (" << pos.epSquare << ")\n";
	}

	out << std::endl;
}
