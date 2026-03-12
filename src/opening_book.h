#pragma once

#include <optional>
#include <string>
#include <vector>

// Returns a book move for the given move history, or nullopt if not in book.
// moveHistory: ordered list of UCI move strings played so far by both sides.
std::optional<std::string> lookupOpeningBook(const std::vector<std::string> &moveHistory);
