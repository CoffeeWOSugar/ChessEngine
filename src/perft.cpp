#include <iostream>
#include "perft.h"

u64 Perft(Position &pos, int depth) {
  if (depth == 0)
    return 1ULL;

  std::vector<Move> moves;
  GenerateLegalMoves(pos, moves);

  if (depth == 1)
    return (u64)moves.size();

  u64 nodes = 0;
  for (const Move &m : moves) {
    if (!pos.makeMove(m))
      continue;
    nodes += Perft(pos, depth - 1);
    pos.undoMove(m);
  }
  return nodes;
}

u64 PerftDivide(Position &pos, int depth) {
  std::vector<Move> moves;
  GenerateLegalMoves(pos, moves);

  u64 total = 0;
  for (const Move &m : moves) {
    if (!pos.makeMove(m))
      continue;

    u64 nodes = Perft(pos, depth-1);
    pos.undoMove(m);

    total += nodes;
    std::cout << MoveToString(m) << ": " << nodes << std::endl;
  }
  std::cout << "Total: " << total << std::endl;
  return total;
}
