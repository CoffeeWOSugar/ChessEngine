#include "position.h"
#include "move.h"
#include "types.h"

static constexpr int SQ_A1 = Position::makeSquare(0, 0);
static constexpr int SQ_E1 = Position::makeSquare(4, 0);
static constexpr int SQ_H1 = Position::makeSquare(7, 0);
static constexpr int SQ_A8 = Position::makeSquare(0, 7);
static constexpr int SQ_E8 = Position::makeSquare(4, 7);
static constexpr int SQ_H8 = Position::makeSquare(7, 7);

static constexpr int SQ_C1 = Position::makeSquare(2, 0);
static constexpr int SQ_D1 = Position::makeSquare(3, 0);
static constexpr int SQ_F1 = Position::makeSquare(5, 0);
static constexpr int SQ_G1 = Position::makeSquare(6, 0);

static constexpr int SQ_C8 = Position::makeSquare(2, 7);
static constexpr int SQ_D8 = Position::makeSquare(3, 7);
static constexpr int SQ_F8 = Position::makeSquare(5, 7);
static constexpr int SQ_G8 = Position::makeSquare(6, 7);

Position::Position() {
  board.fill(EMPTY);
}

bool Position::inCheck(Color c) const {
  int kingPiece = (c == WHITE ? WK : BK);
  int kingSq = -1;

  for (int sq = 0; sq < 128; ++sq) {
    if (sq & 0x88) {sq += 7; continue;}
    if (board[sq] == kingPiece) {
      kingSq = sq;
      break;
    }
  }

  if (kingSq == -1) {
    return false;
  }

  Color enemy = opposite(c);

  // Pawn Attacks
  if (enemy == WHITE) {
    int sq1 = kingSq + 15;
    int sq2 = kingSq + 17;
    if (Position::isOnBoard(sq1) && board[sq1] == WP) return true;
    if (Position::isOnBoard[sq2] && board[sq2] == WP) return true;
  }
  else {
    int sq1 = kingSq - 17;
    int sq2 = kingSq - 15;
    if (Position::isOnBoard(sq1) && board[sq1] == BP) return true;
    if (Position::isOnBoard[sq2] && board[sq2] == BP) return true;
  }

  // Knight Attacks
  for (int off : KnightOffsets) {
    int sq = kingSq + off;
    if (!Position::isOnBoard(sq)) continue;
    int p = board[sq];
    if (p == EMPTY) continue;
    if (pieceColor(p) == enemy && pieceType(p) == WN) {
      return true;
    }
  }

  // Sliding bishops / queen attacks (diagonal)
  for (int off : BishopOffsets) {
    int sq = kingSq;
    while (true) {
      sq + off;
      if (!Position::isOnBoard(sq)) break;
      int p = board[sq];
      if (p == EMPTY) continue;
      if (pieceColor(p) == enemy && (pieceType(p) == WB || pieceType(p) == WQ)) 
        return true;
      break;
    }
  }

  // Sliding rooks / queen attacks (orthogonal)
  for (int off : RookOffsets) {
    int sq = kingSq;
    while (true) {
      sq += off;
      if (!Position::isOnBoard(sq)) break;
      int p = board[sq];
      if (p == EMPTY) continue;
      if (pieceColor(p) == enemy && (pieceType(p) == WR || pieceType(p) == WQ))
        return true;
      break;
    }
  }

  // King adjacent check
  for (int off : KingOffsets) {
    int sq = kingSq + off;
    if (!Position::isOnBoard(sq)) continue;
    int p = board[sq];
    if (p == EMPTY) continue;
    if (pieceColor(p) == enemy && pieceType(p) == WK)
      return true;
  }

  return false;
}

void Position::setStartPosition() {
  board.fill(EMPTY);
  // Setup starting position

  // --- White pieces ---
  board[makeSquare(0, 0)] = WR;
  board[makeSquare(1, 0)] = WN;
  board[makeSquare(2, 0)] = WB;
  board[makeSquare(3, 0)] = WQ;
  board[makeSquare(4, 0)] = WK;
  board[makeSquare(5, 0)] = WB;
  board[makeSquare(6, 0)] = WN;
  board[makeSquare(7, 0)] = WR;

  for (int file = 0; file < 8; ++file) {
    board[makeSquare(file, 1)] = WP;
  }

  // --- Black pieces ---
  board[makeSquare(0, 7)] = BR;
  board[makeSquare(1, 7)] = BN;
  board[makeSquare(2, 7)] = BB;
  board[makeSquare(3, 7)] = BQ;
  board[makeSquare(4, 7)] = BK;
  board[makeSquare(5, 7)] = BB;
  board[makeSquare(6, 7)] = BN;
  board[makeSquare(7, 7)] = BR;

  for (int file = 0; file < 8; ++file) {
    board[makeSquare(file, 6)] = BP;
  }

  sideToMove     = WHITE;
  castlingRights = WK_CASTLE | WQ_CASTLE | BK_CASTLE | BQ_CASTLE;
  epSquare       = -1;
  halfmoveClock  = 0;
  fullmoveNumber = 1;

  stateStack.clear();
}

bool Position::makeMove(const Move &m) {
  const int from  = m.from;
  const int to    = m.to;
  const int piece = m.piece;
  const uint8_t flags = m.flags;

  int capturedPiece = EMPTY;
  int capturedSq    = to;

  if (flags & MF_EN_PASSANT) {
    Color us = pieceColor(piece);
    capturedSq = (us == WHITE) ? (to - 16) : (to + 16);
    capturedPiece = board[capturedSq];
  }
  else if (flags & MF_CAPTURE) {
    capturedPiece = board[to];
  }

  // Save state for undoMove
  State st{};
  st.castlingRights = castlingRights;
  st.epSquare       = epSquare;
  st.halfmoveClock  = halfmoveClock;
  st.fullmoveNumber = fullmoveNumber;
  st.capturedPiece  = capturedPiece;
  st.move           = m;
  stateStack.push_back(st);

  // --- update halfmove clock --- 
  if (pieceType(piece) == WP || (flags & MF_CAPTURE)) {
    halfmoveClock = 0;
  }
  else {
    ++halfmoveClock;
  }

  // Update fullmove number after blacks turn
  if (sideToMove == BLACK) {
    ++fullmoveNumber;
  }

  epSquare = -1;
  
  if (flags & MF_EN_PASSANT) {
    board[capturedSq] = EMPTY;
  }
  else if (flags & MF_CAPTURE) {
    board[to] = EMPTY;
  }

  board[from] = EMPTY;

  int placedPiece = piece;
  if (flags & MF_PROMOTION) {
    placedPiece = m.promotion;
  }

  board[to] = placedPiece;

  // Update castling rights due to moving piece
  switch (piece) {
    case WK:
      castlingRights &= ~(WK_CASTLE | WQ_CASTLE);
      break;
    case BK:
      castlingRights &= ~(BK_CASTLE | BQ_CASTLE);
      break;
    case WR:
      if (from == SQ_A1) castlingRights &= ~WQ_CASTLE;
      else if (from == SQ_H1) castlingRights &= ~WK_CASTLE;
      break;
    case BR:
      if (from == SQ_A8) castlingRights &= ~BQ_CASTLE;
      else if (from == SQ_H8) castlingRights &= ~BK_CASTLE;
      break;
    default:
      break;
  }

  // Update castling rights due to rook capture
  if (capturedPiece == WR) {
    if (capturedSq == SQ_A1) castlingRights &= ~WQ_CASTLE;
    else if (capturedSq == SQ_H1) castlingRights &= ~WK_CASTLE;
  }
  else if (capturedPiece == BR) {
    if (capturedSq == SQ_A8) castlingRights &= ~BQ_CASTLE;
    else if (capturedSq == SQ_H1) castlingRights &= ~BK_CASTLE;
  }

  // Handling castling rook moves
  if (flags & MF_CASTLING) {
    if (piece == WK) {
      if (to == SQ_G1) {
        board[SQ_F1] = WR;
        board[SQ_H1] = EMPTY;
      }
      else if (to == SQ_C1) {
        board[SQ_D1] = WR;
        board[SQ_A1] = EMPTY;
      }
    }
    else if (piece == BK) {
      if (to == SQ_G8) {
        board[SQ_F8] = BR;
        board[SQ_H8] = EMPTY;
      }
      else if (to == SQ_C8) {
        board[SQ_D8] = BR;
        board[SQ_A8] = EMPTY;
      }
    }
  }

  // Set new en passant square on double pawn push
  if (pieceType(piece) == WP) {
    int diff = to - from;
    if (diff == 32 || diff == -32) {
      epSquare = (from + to) / 2;
    }
  }

  // Swithc side to move
  sideToMove = (sideToMove == WHITE ? BLACK : WHITE);

  // Legality check?
  Color us = (sideToMove == WHITE ? BLACK : WHITE);
  if (inCheck(us)) {
    undoMove(m);
    return false;
  }
  
  return true;
}

void Position::undoMove(const Move &m) {
  if (stateStack.empty())
    return;

  State st = stateStack.back();
  stateStack.pop_back();

  const int from  = m.from;
  const int to    = m.to;
  const int piece = m.piece;
  const uint8_t flags = m.flags;

  // Restore side to move
  sideToMove = (sideToMove == WHITE ? BLACK : WHITE);

  // Restore clocks and global state
  castlingRights = st.castlingRights;
  epSquare       = st.epSquare;
  halfmoveClock  = st.halfmoveClock;
  fullmoveNumber = st.fullmoveNumber;

  // Undo board changes
  if (flags & MF_CASTLING) {
    board[from] = piece;
    board[to]   = EMPTY;

    if (piece == WK) {
      if (to == SQ_G1) {
        board[SQ_G1] = WR;
        board[SQ_F1] = EMPTY;
      }
      else if (to == SQ_C1) {
        board[SQ_A1] = WR;
        board[SQ_D1] = EMPTY;
      }
    }
    else if (piece == BK) {
      if (to == SQ_G8) {
        board[SQ_H8] = BR;
        board[SQ_F8] = EMPTY;
      }
      else if (to == SQ_C8) {
        board[SQ_A8] = BR;
        board[SQ_D8] = EMPTY;
      }
    }
  } else {
    // Non-castling moves

    board[to] = EMPTY;
    board[from] = piece;

    if (flags & MF_EN_PASSANT) {
      Color us = pieceColor(piece);
      int capturedSq = (us == WHITE) ? (to - 16) : (to + 16);
      board[capturedSq] = st.capturedPiece;
    }
    else if (flags & MF_CAPTURE) {
      board[to] = st.capturedPiece;
    }
  }
}
