#include "opening_book.h"
#include <random>
#include <unordered_map>

// Key: space-joined UCI move history. Value: candidate responses (one chosen randomly).
// Covers the main practical openings to ~6 plies.
static const std::unordered_map<std::string, std::vector<std::string>> BOOK = {

    // ── STARTING POSITION ─────────────────────────────────────────────
    {"", {"e2e4", "d2d4"}},

    // ── AFTER 1.e4 ────────────────────────────────────────────────────
    {"e2e4", {"e7e5", "c7c5", "e7e6", "c7c6"}},

    // ── OPEN GAME (1.e4 e5) ───────────────────────────────────────────
    {"e2e4 e7e5", {"g1f3"}},
    {"e2e4 e7e5 g1f3", {"b8c6", "g8f6"}},

    // Italian Game
    {"e2e4 e7e5 g1f3 b8c6", {"f1c4", "f1b5"}},
    {"e2e4 e7e5 g1f3 b8c6 f1c4", {"f8c5", "g8f6"}},
    {"e2e4 e7e5 g1f3 b8c6 f1c4 f8c5", {"c2c3"}},
    {"e2e4 e7e5 g1f3 b8c6 f1c4 f8c5 c2c3", {"g8f6"}},
    {"e2e4 e7e5 g1f3 b8c6 f1c4 f8c5 c2c3 g8f6", {"d2d4"}},
    {"e2e4 e7e5 g1f3 b8c6 f1c4 f8c5 c2c3 g8f6 d2d4", {"e5d4"}},
    {"e2e4 e7e5 g1f3 b8c6 f1c4 f8c5 c2c3 g8f6 d2d4 e5d4", {"c3d4"}},

    // Ruy Lopez
    {"e2e4 e7e5 g1f3 b8c6 f1b5", {"a7a6", "g8f6", "f8c5"}},
    {"e2e4 e7e5 g1f3 b8c6 f1b5 a7a6", {"b5a4"}},
    {"e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5a4", {"g8f6"}},
    {"e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5a4 g8f6", {"e1g1"}},
    {"e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5a4 g8f6 e1g1", {"f8e7"}},
    {"e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5a4 g8f6 e1g1 f8e7", {"f1e1"}},
    {"e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5a4 g8f6 e1g1 f8e7 f1e1", {"b7b5"}},
    {"e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5a4 g8f6 e1g1 f8e7 f1e1 b7b5", {"a4b3"}},

    // Petrov Defense
    {"e2e4 e7e5 g1f3 g8f6", {"f3e5"}},
    {"e2e4 e7e5 g1f3 g8f6 f3e5", {"d7d6"}},
    {"e2e4 e7e5 g1f3 g8f6 f3e5 d7d6", {"e5f3"}},

    // ── SICILIAN (1.e4 c5) ────────────────────────────────────────────
    {"e2e4 c7c5", {"g1f3"}},
    {"e2e4 c7c5 g1f3", {"d7d6", "b8c6", "e7e6"}},

    // Najdorf
    {"e2e4 c7c5 g1f3 d7d6", {"d2d4"}},
    {"e2e4 c7c5 g1f3 d7d6 d2d4", {"c5d4"}},
    {"e2e4 c7c5 g1f3 d7d6 d2d4 c5d4", {"f3d4"}},
    {"e2e4 c7c5 g1f3 d7d6 d2d4 c5d4 f3d4", {"g8f6"}},
    {"e2e4 c7c5 g1f3 d7d6 d2d4 c5d4 f3d4 g8f6", {"b1c3"}},
    {"e2e4 c7c5 g1f3 d7d6 d2d4 c5d4 f3d4 g8f6 b1c3", {"a7a6"}},

    // Dragon / Open Sicilian with Nc6
    {"e2e4 c7c5 g1f3 b8c6", {"d2d4"}},
    {"e2e4 c7c5 g1f3 b8c6 d2d4", {"c5d4"}},
    {"e2e4 c7c5 g1f3 b8c6 d2d4 c5d4", {"f3d4"}},

    // Kan / Taimanov
    {"e2e4 c7c5 g1f3 e7e6", {"d2d4"}},
    {"e2e4 c7c5 g1f3 e7e6 d2d4", {"c5d4"}},
    {"e2e4 c7c5 g1f3 e7e6 d2d4 c5d4", {"f3d4"}},

    // ── FRENCH DEFENSE (1.e4 e6) ──────────────────────────────────────
    {"e2e4 e7e6", {"d2d4"}},
    {"e2e4 e7e6 d2d4", {"d7d5"}},
    {"e2e4 e7e6 d2d4 d7d5", {"b1c3", "b1d2", "e4e5"}},
    {"e2e4 e7e6 d2d4 d7d5 b1c3", {"g8f6", "f8b4", "d5e4"}},
    {"e2e4 e7e6 d2d4 d7d5 b1d2", {"g8f6", "c7c5"}},

    // ── CARO-KANN (1.e4 c6) ───────────────────────────────────────────
    {"e2e4 c7c6", {"d2d4"}},
    {"e2e4 c7c6 d2d4", {"d7d5"}},
    {"e2e4 c7c6 d2d4 d7d5", {"b1c3", "e4e5", "e4d5"}},
    {"e2e4 c7c6 d2d4 d7d5 b1c3", {"d5e4", "g8f6"}},
    {"e2e4 c7c6 d2d4 d7d5 e4e5", {"c8f5"}},

    // ── AFTER 1.d4 ────────────────────────────────────────────────────
    {"d2d4", {"d7d5", "g8f6", "f7f5"}},

    // ── QUEEN'S GAMBIT (1.d4 d5) ──────────────────────────────────────
    {"d2d4 d7d5", {"c2c4"}},
    {"d2d4 d7d5 c2c4", {"e7e6", "c7c6", "d5c4"}},

    // QGD
    {"d2d4 d7d5 c2c4 e7e6", {"b1c3", "g1f3"}},
    {"d2d4 d7d5 c2c4 e7e6 b1c3", {"g8f6", "f8e7", "c7c6"}},
    {"d2d4 d7d5 c2c4 e7e6 b1c3 g8f6", {"c1g5", "g1f3"}},
    {"d2d4 d7d5 c2c4 e7e6 b1c3 g8f6 c1g5", {"f8e7"}},
    {"d2d4 d7d5 c2c4 e7e6 b1c3 g8f6 c1g5 f8e7", {"e2e3"}},
    {"d2d4 d7d5 c2c4 e7e6 b1c3 g8f6 c1g5 f8e7 e2e3", {"e8g8"}},

    // Slav Defense
    {"d2d4 d7d5 c2c4 c7c6", {"g1f3", "b1c3"}},
    {"d2d4 d7d5 c2c4 c7c6 g1f3", {"g8f6"}},
    {"d2d4 d7d5 c2c4 c7c6 g1f3 g8f6", {"b1c3"}},
    {"d2d4 d7d5 c2c4 c7c6 g1f3 g8f6 b1c3", {"d5c4", "e7e6"}},

    // QGA
    {"d2d4 d7d5 c2c4 d5c4", {"g1f3", "e2e3"}},
    {"d2d4 d7d5 c2c4 d5c4 g1f3", {"g8f6", "e7e6"}},

    // ── INDIAN DEFENSES (1.d4 Nf6) ────────────────────────────────────
    {"d2d4 g8f6", {"c2c4"}},
    {"d2d4 g8f6 c2c4", {"e7e6", "g7g6", "c7c5", "e7e5"}},

    // Nimzo-Indian
    {"d2d4 g8f6 c2c4 e7e6", {"b1c3"}},
    {"d2d4 g8f6 c2c4 e7e6 b1c3", {"f8b4", "d7d5"}},
    {"d2d4 g8f6 c2c4 e7e6 b1c3 f8b4", {"e2e3", "d1c2", "g1f3"}},
    {"d2d4 g8f6 c2c4 e7e6 b1c3 f8b4 e2e3", {"e8g8", "c7c5", "b7b6"}},

    // King's Indian Defense
    {"d2d4 g8f6 c2c4 g7g6", {"b1c3", "g2g3"}},
    {"d2d4 g8f6 c2c4 g7g6 b1c3", {"f8g7"}},
    {"d2d4 g8f6 c2c4 g7g6 b1c3 f8g7", {"e2e4"}},
    {"d2d4 g8f6 c2c4 g7g6 b1c3 f8g7 e2e4", {"d7d6"}},
    {"d2d4 g8f6 c2c4 g7g6 b1c3 f8g7 e2e4 d7d6", {"g1f3"}},
    {"d2d4 g8f6 c2c4 g7g6 b1c3 f8g7 e2e4 d7d6 g1f3", {"e8g8"}},
    {"d2d4 g8f6 c2c4 g7g6 b1c3 f8g7 e2e4 d7d6 g1f3 e8g8", {"f1e2"}},

    // Grünfeld
    {"d2d4 g8f6 c2c4 g7g6 b1c3 d7d5", {"c4d5"}},
    {"d2d4 g8f6 c2c4 g7g6 b1c3 d7d5 c4d5", {"f6d5"}},
    {"d2d4 g8f6 c2c4 g7g6 b1c3 d7d5 c4d5 f6d5", {"e2e4"}},
    {"d2d4 g8f6 c2c4 g7g6 b1c3 d7d5 c4d5 f6d5 e2e4", {"d5c3"}},
    {"d2d4 g8f6 c2c4 g7g6 b1c3 d7d5 c4d5 f6d5 e2e4 d5c3", {"b2c3"}},
};

std::optional<std::string> lookupOpeningBook(const std::vector<std::string> &moveHistory) {
	std::string key;
	for (size_t i = 0; i < moveHistory.size(); ++i) {
		if (i > 0)
			key += ' ';
		key += moveHistory[i];
	}

	auto it = BOOK.find(key);
	if (it == BOOK.end())
		return std::nullopt;

	const auto &candidates = it->second;
	if (candidates.empty())
		return std::nullopt;

	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
	return candidates[dist(rng)];
}
