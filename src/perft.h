#pragma once
#include "position.h"
#include "movegen.h"

u64 Perft(Position &pos, int depth);
u64 PerftDivide(Position &pos, int depth);
