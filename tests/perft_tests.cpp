#include "position.h"
#include "perft.h"
#include <cassert>

void test_startpos() {
  Position pos;
  pos.setStartPosition();

  assert(Perft(pos, 1) == 20);
  assert(Perft(pos, 2) == 400);
  assert(Perft(pos, 3) == 8902);
  assert(Perft(pos, 4) == 197281);
  assert(Perft(pos, 5) == 4865609);
  assert(Perft(pos, 6) == 119060324);
}
