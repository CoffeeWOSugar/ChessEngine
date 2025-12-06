#include <iostream>
#include "position.h"
#include "perft.h"
#include "../tests/perft_tests.h"
#include "utils.h"

int main(int argc, char* argv[]) {
  if (argc > 1 && std::string(argv[1]) == "--run-tests") {
    run_perft_tests();
    return 0;
  }

  Position pos;
  pos.setStartPosition();
 
  printBoard(pos, true);

  return 0;
}
