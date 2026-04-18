#ifndef CHESS_H
#define CHESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_RANKS  8
#define NUM_FILES  10
#define MAX_MOVES  512

enum PieceType {
    KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN, ANTEATER
};

enum PieceColor {
    WHITE, BLACK
};

struct Piece {
    enum PieceType  type;
    enum PieceColor color;
};

struct Pos {
    int r;
    int f;
};

struct Move {
    struct Pos from;
    struct Pos to;

    struct Piece   captured;

    int            was_en_passant;
    int            was_castling;
    int            was_promotion;

    enum PieceType promoted_to;
    struct Pos     rook_from;
    struct Pos     rook_to;
    struct Pos     ep_cap_pos;

    int prev_wck;
    int prev_wcq;
    int prev_bck;
    int prev_bcq;

    struct Pos prev_ep;
};

struct MoveLog {
    struct Move moves[MAX_MOVES];
    int         count;
};

struct GameState {
    struct Piece   *board[NUM_RANKS][NUM_FILES];
    enum PieceColor current_turn;
    struct MoveLog  history;
    int             white_castle_k;
    int             white_castle_q;
    int             black_castle_k;
    int             black_castle_q;
    struct Pos      en_passant_target;
};

#define OPPONENT(c)  ((c) == WHITE ? BLACK : WHITE)

void          init_game    (struct GameState *gs);
void          display_board(struct Piece *board[NUM_RANKS][NUM_FILES]);
struct Piece *lookup_piece (struct Piece *board[NUM_RANKS][NUM_FILES],
                            struct Pos pos);
void          assign_piece (struct Piece *board[NUM_RANKS][NUM_FILES],
                            struct Piece *piece, struct Pos pos);
void          free_board   (struct Piece *board[NUM_RANKS][NUM_FILES]);
int           pos_valid    (struct Pos p);
struct Pos    make_pos     (int r, int f);
struct Piece *make_piece   (enum PieceType type, enum PieceColor color);
const char   *piece_char   (struct Piece *p);

void log_move  (struct GameState *gs, struct Move m);
void undo_move (struct GameState *gs);

void apply_move       (struct GameState *gs, struct Move m);
int  is_legal_move    (struct GameState *gs, struct Move m);
int  king_in_checkmate(struct GameState *gs, enum PieceColor color);
int  king_in_stalemate(struct GameState *gs, enum PieceColor color);

int logfile_function(struct Move move, int turn_number,
                     enum PieceColor color, const char *filename);
int save_function   (struct GameState *gs, const char *filename);
int load_function   (struct GameState *gs, const char *filename);

struct Move bestmove_function(struct GameState *gs,
                              enum PieceColor color, int depth);

#endif