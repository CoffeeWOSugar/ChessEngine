#pragma once
#include <vector>
#include "position.h"

void GenerateLegalMoves(const Position &pos, std::vector<Move> &moves);
void GeneratePseudoLegalMoves(const Position &pos, std::vector<Move> &moves);
