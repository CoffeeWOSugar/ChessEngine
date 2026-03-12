# ChessEngine
A 0x88 chess engine with terminal and web UI, containerized with Docker.
Alpha-beta search with iterative deepening and per-move time control.
Move generation verified via Perft to depth 7 across multiple positions.

## Build (Linux)

### Prerequisites
- C++20-compatible compiler (`g++` or `clang++`)
- CMake >= 3.16 (optional)
- MPI implementation e.g. OpenMPI (optional, for parallel search)
- Docker (optional, for web UI)

### Makefile
```bash
make && ./chess                  # terminal game
./chess --run-tests              # perft suite
make mpi && mpirun -n 4 ./chess_mpi  # MPI parallel build
```

### CMake
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/chess
```

### Docker (web UI)
```bash
docker build -t chess-web:latest .
docker run --rm -p 8000:8000 chess-web
```
Then open **http://localhost:8000**

## Controls
Moves use UCI notation: `e2e4`, `g1f3`, `e7e8q` (promotion).

White: `♔ ♕ ♖ ♗ ♘ ♙`  Black: `♚ ♛ ♜ ♝ ♞ ♟`

## Testing & Validation (Perft from start position)
| Depth | Nodes         |
|------:|--------------:|
| 1     | 20            |
| 2     | 400           |
| 3     | 8,902         |
| 4     | 197,281       |
| 5     | 4,865,609     |
| 6     | 119,060,324   |
| 7     | 3,195,901,860 |

Additional suites: Kiwipete, Position 3, 5, and 6 from [chessprogramming.org/Perft_Results](https://www.chessprogramming.org/Perft_Results).

## Project Structure
```
src/        — C++ engine (position, movegen, search, perft, session)
tests/      — perft test suite
web/        — FastAPI backend + HTML/CSS/JS frontend
include/    — nlohmann JSON header
```

## Contributing
Don't. This is purely for my own recreation and learning purposes.
