#include <iostream>
#include "position.h"
#include "perft.h"
#include "../tests/perft_tests.h"

int main(int argc, char* argv[]) {
  if (argc > 1 && std::string(argv[1]) == "--run-tests") {
    run_perft_tests();
    return 0;
  }

  Position pos;
  pos.setStartPosition();

  int depth = 4;
  u64 nodes = PerftDivide(pos, depth);
  std::cout << "Nodes at depth " << depth << ": " << nodes << std::endl;
  return 0;
}
