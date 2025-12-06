#pragma once
#include "types.h"
#include <string>

struct Move {
	uint16_t from;
	uint16_t to;
	uint8_t piece;
	uint8_t captured;
	uint8_t promotion;
	uint8_t flags;
};

enum MoveFlags : uint8_t {
	MF_NONE = 0,
	MF_CAPTURE = 1 << 0,
	MF_EN_PASSANT = 1 << 1,
	MF_CASTLING = 1 << 2,
	MF_PROMOTION = 1 << 3
};

inline Move make_move(int from, int to, int piece, int captured = EMPTY, int promotion = EMPTY,
                      uint8_t flags = MF_NONE) {
	return Move{static_cast<uint16_t>(from),     static_cast<uint16_t>(to),
	            static_cast<uint8_t>(piece),     static_cast<uint8_t>(captured),
	            static_cast<uint8_t>(promotion), flags};
}

std::string MoveToString(const Move &m);
