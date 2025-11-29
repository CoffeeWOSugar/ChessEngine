#include "move.h"
#include <string>

std::string MoveToString(const Move &m) {
  return std::to_string(m.from) + "->" + std::to_string(m.to);
}
