# ChessEngine
A 0x88 chess engine implementation fully in the terminal and containerized using Docker.
Correctness is verified through Perft to depth of 7 for engine move generation.

This version of the engine works by implementing a simple Alpha-beta search with iterative deepening limited by per move time control.

## Build (Linux)

### Prerequisites

- A Computer.
- C++17-compatible compiler (e.g. `g++` or `clang++`)
- CMake >= 3.16 (Optional)
- Docker (Optional)
- 600 ELO Chess knowledge (Optional)

### Build & Run (Makefile)

#### Play
```bash
# From project root
make
./chess
```

#### Perft tests
```bash
# From project root
make
./chess --run-tests
```

### Build & Run (CMake)

```bash
# From project root
mkdir -p build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release

cmake --build . --config Release -j
```

This will produce ```build/chess``` which can be run as:
```bash
./build/chess
```

### Build & Run (Docker)

#### Build the image
```bash
# From the root where the Dockerfile lives.
docker build -t your-docker-user/chess:latest .
```

#### Run locally
```bash
docker run --rm -it your-docker-user/chess:latest .
```
or
```bash
docker run --rm -p 8000:8000 chess-web
```
## Controls & Display
The board is displayed using prints to standard output using Unicode symbols:
White: ```♔ ♕ ♖ ♗ ♘ ♙```
Black: ```♚ ♛ ♜ ♝ ♞ ♟```

Ranks: 1 -> 8
Files: a -> h

All moves are played by entering the starting square and ending square of the pieces with any promotion appended as the respective piece algebraic notation (e.g. e2e4, a7a8q, g1f3, ...).
This complies with the UCI protocol disgned by Rudolf Huber and Stefan Meyer-Kahlen. [Wikipedia/UCI](https://en.wikipedia.org/wiki/Universal_Chess_Interface)

## Testing & Validation (Perft)
| Depth | Nodes         |
|------:|--------------:|
| 1     | 20            |
| 2     | 400           |
| 3     | 8902          |
| 4     | 197,281       |
| 5     | 4,865,609     |
| 6     | 119,060,324   |
| 7     | 3,195,901,860 |

## Project Structure
```
src/
 ├─ main.cpp
 ├─ position.cpp / position.h
 ├─ move.cpp / move.h
 ├─ movegen.cpp / movegen.h
 ├─ search.cpp / search.h
 ├─ perft.cpp / perft.h
 ├─ utils.cpp / utils.h
tests/
 ├─ perft_tests.cpp
```

## Contributing
Don't. This is purely for my own recreation and learning purposes.
