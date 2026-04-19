#ifndef RULES_H
#define RULES_H

#include "chess.h"

//Is This Position Under Attack?: this function will be used for CHECK status
int is_under_attack(struct GameState *gs, struct Pos p, enum PieceColor op_color);
//op = opponent/attacker

//Is the King in Check (Is color specific)
int king_in_check(struct GameState *gs, enum PieceColor color);

//Is this Move Possible Given the Anteater Chess rules? (has to be LEGAL)
int possible_moves(struct GameState *gs, struct Move m);

//Is this Move LEGAL? (Possible Moves that keep the KING safe)
int is_legal_move(struct GameState *gs, struct Move m);

//Is the King in Checkmate?
int king_in_checkmate(struct GameState *gs, enum PieceColor color);

//Is the Game in a Statlemate?
int king_in_stalemate(struct GameState *gs, enum PieceColor color);

#endif
