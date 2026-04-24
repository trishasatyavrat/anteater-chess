#ifndef RULES_H
#define RULES_H

#include "chess.h"

//make a copy of board (uses MALLOC)
struct GameState copy_of_board(struct GameState *gs);

//clears the MEMORY (required for MALLOC)
void clear_board_copy(struct GameState *gs);

//Is This Position Under Attack?: this function will be used for CHECK status
int is_under_attack(struct GameState *gs, struct Pos p, enum PieceColor op_color);
//op = opponent/attacker

//Is the King in Check (Is color specific)
int king_in_check(struct GameState *gs, enum PieceColor color);

//Is this Move Possible Given the Anteater Chess rules? (has to be LEGAL)
int possible_moves(struct GameState *gs, struct Move m);

//Is this Move LEGAL? (Possible Moves that keep the KING safe)
int is_legal_move(struct GameState *gs, struct Move m);

//Function for AI module (Extra parameters)
void legalmoves_function(struct GameState *gs, struct Pos position, struct Move *out, int *count);

//Is the King in Checkmate?
int king_in_checkmate(struct GameState *gs, enum PieceColor color);

//Is the Game in a Statlemate?
int king_in_stalemate(struct GameState *gs, enum PieceColor color);

#endif
