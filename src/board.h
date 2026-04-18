#ifndef BOARD_H
#define BOARD_H

#include "chess.h"

void init_game(struct GameState *gs);

void display_board(struct Piece *board[NUM_RANKS][NUM_FILES]);

struct Piece *lookup_piece(struct Piece *board[NUM_RANKS][NUM_FILES],
                           struct Pos pos);

void assign_piece(struct Piece *board[NUM_RANKS][NUM_FILES],
                  struct Piece *piece, struct Pos pos);

void free_board(struct Piece *board[NUM_RANKS][NUM_FILES]);

int pos_valid(struct Pos p);

struct Pos make_pos(int r, int f);

struct Piece *make_piece(enum PieceType type, enum PieceColor color);

const char *piece_char(struct Piece *p);

#endif