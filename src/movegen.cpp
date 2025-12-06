#include "movegen.h"
#include <iostream>

inline int rank_of(int sq) {
  return sq >> 4;
}

inline int file_of(int sq) {
  return sq & 7;
}

static void gen_white_pawns(const Position &pos, std::vector<Move> &moves, int sq) {
  int piece = pos.board[sq];
  int r = rank_of(sq);

  int oneStep = sq + 16;
  int twoStep = sq + 32;

  // Single push
  if (Position::isOnBoard(oneStep) && pos.board[oneStep] == EMPTY) {
    if (r == 6) {
      // Promotions
      moves.push_back(make_move(sq, oneStep, piece, EMPTY, WQ, MF_PROMOTION));
      moves.push_back(make_move(sq, oneStep, piece, EMPTY, WR, MF_PROMOTION));
      moves.push_back(make_move(sq, oneStep, piece, EMPTY, WB, MF_PROMOTION));
      moves.push_back(make_move(sq, oneStep, piece, EMPTY, WN, MF_PROMOTION));
    }
    else {
      // Double push from rank 1
      moves.push_back(make_move(sq, oneStep, piece));
      if (r == 1 && pos.board[twoStep] == EMPTY) {
        moves.push_back(make_move(sq, twoStep, piece));
      }
    }
  }

  // Captures
  int caps[2] = {sq + 15, sq + 17};
  for (int i = 0; i < 2; ++i) {
    int to = caps[i];
    if (!Position::isOnBoard(to)) continue;
    int target = pos.board[to];
    if (target != EMPTY && pieceColor(target) == BLACK) {
      if (r == 6) {
        // Promotions with captures
        moves.push_back(make_move(sq, to, piece, target, WQ, MF_PROMOTION | MF_CAPTURE));
        moves.push_back(make_move(sq, to, piece, target, WR, MF_PROMOTION | MF_CAPTURE));
        moves.push_back(make_move(sq, to, piece, target, WB, MF_PROMOTION | MF_CAPTURE));
        moves.push_back(make_move(sq, to, piece, target, WN, MF_PROMOTION | MF_CAPTURE));
      }
      else {
        moves.push_back(make_move(sq, to, piece, target, EMPTY, MF_CAPTURE));
      }
    }
  }

  if (pos.epSquare != -1) {
    for (int i = 0; i < 2; ++i) {
      int to = caps[i];
      if (to == pos.epSquare) {
        int behind = to - 16;
        int capturedPiece = pos.board[behind];
        if (capturedPiece == BP) {
          moves.push_back(make_move(sq, to, piece, capturedPiece, EMPTY, MF_EN_PASSANT | MF_CAPTURE));
        }
      }
    }
  }
}

static void gen_black_pawns(const Position &pos, std::vector<Move> &moves, int sq) {
  int piece = pos.board[sq];
  int r = rank_of(sq);

  int oneStep = sq - 16;
  int twoStep = sq - 32;

  // Single push
  if (Position::isOnBoard(oneStep) && pos.board[oneStep] == EMPTY) {
    if (r == 1) {
      // Promotions
      moves.push_back(make_move(sq, oneStep, piece, EMPTY, BQ, MF_PROMOTION));
      moves.push_back(make_move(sq, oneStep, piece, EMPTY, BR, MF_PROMOTION));
      moves.push_back(make_move(sq, oneStep, piece, EMPTY, BB, MF_PROMOTION));
      moves.push_back(make_move(sq, oneStep, piece, EMPTY, BN, MF_PROMOTION));
    }
    else {
      moves.push_back(make_move(sq, oneStep, piece));
      // Double push from rank 6
      if (r == 6 && pos.board[twoStep] == EMPTY) {
        moves.push_back(make_move(sq, twoStep, piece));
      }
    }
  }

  // Captures
  int caps[2] = {sq - 17, sq - 15};
  for (int i = 0; i < 2; ++i) {
    int to = caps[i];
    if (!Position::isOnBoard(to)) continue;
    int target = pos.board[to]; 
    if (target != EMPTY && pieceColor(target) == WHITE) {
      if (r == 1) {
        // Promotions with captures
        moves.push_back(make_move(sq, to, piece, target, BQ, MF_PROMOTION | MF_CAPTURE));
        moves.push_back(make_move(sq, to, piece, target, BR, MF_PROMOTION | MF_CAPTURE));
        moves.push_back(make_move(sq, to, piece, target, BB, MF_PROMOTION | MF_CAPTURE));
        moves.push_back(make_move(sq, to, piece, target, BN, MF_PROMOTION | MF_CAPTURE));
      }
      else {
        moves.push_back(make_move(sq, to, piece, target, EMPTY, MF_CAPTURE));
      }
    }
  }

  // En Passant
  if (pos.epSquare != -1) {
    for (int i = 0; i < 2; ++i) {
      int to = caps[i];
      if (to == pos.epSquare) {
        int behind = to + 16;
        int capturedPiece = pos.board[behind];
        if (capturedPiece == WP) {
          moves.push_back(make_move(sq, to, piece, capturedPiece, EMPTY, MF_EN_PASSANT | MF_CAPTURE));
        }
      }
    }
  }
}

static void gen_knight_moves(const Position &pos, std::vector<Move> &moves, int sq, int piece) {
  Color us = pieceColor(piece);
  for (int i = 0; i < 8; ++i) {
    int to = sq + KnightOffsets[i];
    if (!Position::isOnBoard(to)) continue;
    int target = pos.board[to];
    if (target == EMPTY) {
      moves.push_back(make_move(sq, to, piece));
    }
    else if (pieceColor(target) != us) {
      moves.push_back(make_move(sq, to, piece, target, EMPTY, MF_CAPTURE));
    }
  }
}

static void gen_slider_moves(const Position &pos, std::vector<Move> &moves, int sq, int piece, const int *dirs, int dirCount)
{
  Color us = pieceColor(piece);
  for (int d = 0; d < dirCount; ++d) {
    int delta = dirs[d];
    int to = sq;

    while (true) {
      to += delta;
      if (!Position::isOnBoard(to)) break;
      int target = pos.board[to];
      if (target == EMPTY) {
        moves.push_back(make_move(sq, to, piece));
      }
      else {
        if (pieceColor(target) != us) {
          moves.push_back(make_move(sq, to, piece, target, EMPTY, MF_CAPTURE));
        }
        break;
      }
    }
  }
}

static void gen_king_moves(const Position &pos, std::vector<Move> &moves, int sq, int piece) {
  Color us = pieceColor(piece);
  for (int i = 0; i < 8; ++i) {
    int to = sq + KingOffsets[i];
    if (!Position::isOnBoard(to)) continue;
    int target = pos.board[to];
    if (target == EMPTY) {
      moves.push_back(make_move(sq, to, piece));
    }
    else if (pieceColor(target) != us) {
      moves.push_back(make_move(sq, to, piece, target, EMPTY, MF_CAPTURE));
    }
  }
}

static void gen_castling(const Position &pos, std::vector<Move> &moves, int kingSq, int kingPiece) {
  Color us   = pieceColor(kingPiece);
  Color them = opposite(us);
  int rights = pos.castlingRights;

  if (us == WHITE && kingSq == Position::makeSquare(4, 0)) {
    int e1 = kingSq;
    int f1 = e1 + 1;
    int g1 = e1 + 2;
    int b1 = e1 - 3;
    int c1 = e1 - 2;
    int d1 = e1 - 1;

    if (rights & WK_CASTLE) {
      if (pos.board[f1] == EMPTY && pos.board[g1] == EMPTY
          && !pos.isSquareAttacked(e1, them)
          && !pos.isSquareAttacked(f1, them)
          && !pos.isSquareAttacked(g1, them)) {
        moves.push_back(make_move(e1, g1, kingPiece, EMPTY, EMPTY, MF_CASTLING));
      }
    }
    if (rights & WQ_CASTLE) {
      if (pos.board[d1] == EMPTY && pos.board[c1] == EMPTY && pos.board[b1] == EMPTY
          && !pos.isSquareAttacked(e1, them)
          && !pos.isSquareAttacked(d1, them)
          && !pos.isSquareAttacked(c1, them)) {
        moves.push_back(make_move(e1, c1, kingPiece, EMPTY, EMPTY, MF_CASTLING));
      }
    }
  }
  else if (us == BLACK && kingSq == Position::makeSquare(4, 7)){
    int e8 = kingSq;
    int f8 = e8 + 1;
    int g8 = e8 + 2;
    int b8 = e8 - 3;
    int c8 = e8 - 2;
    int d8 = e8 - 1;

    if (rights & BK_CASTLE) {
      if (pos.board[f8] == EMPTY && pos.board[g8] == EMPTY
          && !pos.isSquareAttacked(e8, them)
          && !pos.isSquareAttacked(f8, them)
          && !pos.isSquareAttacked(g8, them)) {
        moves.push_back(make_move(e8, g8, kingPiece, EMPTY, EMPTY, MF_CASTLING));
      }
    }
    if (rights & BQ_CASTLE) {
      if (pos.board[d8] == EMPTY && pos.board[c8] == EMPTY && pos.board[b8] == EMPTY
          && !pos.isSquareAttacked(e8, them)
          && !pos.isSquareAttacked(d8, them)
          && !pos.isSquareAttacked(c8, them)) {
        moves.push_back(make_move(e8, c8, kingPiece, EMPTY, EMPTY, MF_CASTLING));
      }
    }
  }
}


void GeneratePseudoLegalMoves(const Position &pos, std::vector<Move> &moves) {
  moves.clear();
  // Loop over 0x88 board and if square has "side to move" piece, generate the moves (within bounds...)
  Color us = pos.sideToMove;

  for (int sq = 0; sq < 128; ++sq) {
    if (sq & 0x88) {
      sq += 7;
      continue;
    }

    int piece = pos.board[sq];
    if (piece == EMPTY) continue;
    if (pieceColor(piece) != us) continue;

    int pt = pieceType(piece);

    switch (pt) {
      case WP:
        if (us == WHITE) gen_white_pawns(pos, moves, sq);
        else             gen_black_pawns(pos, moves, sq);
        break;

      case WN:
        gen_knight_moves(pos, moves, sq, piece);
        break;

      case WB:
        gen_slider_moves(pos, moves, sq, piece, BishopOffsets, 4);
        break;

      case WR:
        gen_slider_moves(pos, moves, sq, piece, RookOffsets, 4);
        break;

      case WQ:
        gen_slider_moves(pos, moves, sq, piece, BishopOffsets, 4);
        gen_slider_moves(pos, moves, sq, piece, RookOffsets, 4);
        break;

      case WK:
        gen_king_moves(pos, moves, sq, piece);
        gen_castling(pos, moves, sq, piece);
        break;

      default:
        break;
    }
  }
}


void GenerateLegalMoves(const Position &pos, std::vector<Move> &moves) {
  std::vector<Move> pseudo;
  GeneratePseudoLegalMoves(pos, pseudo);
  moves.clear();
  for (const Move &m : pseudo) {
    Position tmp = pos;
    if (tmp.makeMove(m)) {
      moves.push_back(m);
    }
  }
}
