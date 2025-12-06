#pragma once
#include "movegen.h"
#include "position.h"

u64 Perft(Position &pos, int depth);
u64 PerftDivide(Position &pos, int depth);
