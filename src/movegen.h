#pragma once
#include "position.h"
#include <vector>

void GenerateLegalMoves(const Position &pos, std::vector<Move> &moves);
void GeneratePseudoLegalMoves(const Position &pos, std::vector<Move> &moves);
