#include <iostream>
#include "position.h"
#include "perft.h"

int main() {
  Position pos;
  pos.setStartPosition();

  int depth = 4;
  u64 nodes = PerftDivide(pos, depth);
  std::cout << "Nodes at depth " << depth << ": " << nodes << std::endl;
  depth = 5;
  nodes = PerftDivide(pos, depth);
  std::cout << "Nodes at depth " << depth << ": " << nodes << std::endl;
  return 0;
}
