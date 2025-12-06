#pragma once
#include <cstdint>

using u64 = unsigned long long;

enum Piece : int { EMPTY = 0, WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK };

enum Color : int { WHITE = 0, BLACK = 1, NO_COLOR = 2 };

inline Color pieceColor(int p) {
	if (p == EMPTY) {
		return NO_COLOR;
	}
	return (p >= BP) ? BLACK : WHITE;
}

inline int pieceType(int p) {
	if (p == EMPTY) {
		return EMPTY;
	}
	return (p >= BP) ? (p - BP + WP) : p;
}

inline Color opposite(Color c) { return c == WHITE ? BLACK : WHITE; }
