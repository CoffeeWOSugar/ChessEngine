#include "move.h"
#include "types.h"
#include <string>

// 0x88 square: rank = sq >> 4, file = sq & 0xF
static char sqFile(int sq) { return 'a' + (sq & 0xF); }
static char sqRank(int sq) { return '1' + (sq >> 4); }

static char promoChar(int piece) {
	switch (piece) {
	case WQ:
	case BQ:
		return 'q';
	case WR:
	case BR:
		return 'r';
	case WB:
	case BB:
		return 'b';
	case WN:
	case BN:
		return 'n';
	default:
		return '?';
	}
}

std::string MoveToString(const Move &m) {
	std::string s;
	s += sqFile(m.from);
	s += sqRank(m.from);
	s += sqFile(m.to);
	s += sqRank(m.to);
	if (m.flags & MF_PROMOTION)
		s += promoChar(m.promotion);
	return s;
}
