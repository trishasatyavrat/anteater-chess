#ifndef MOVE_H
#define MOVE_H

#include "board.h"
//main
int move_piece(struct GameState *gs, struct Pos from, struct Pos to);

//capture 
int handle_capture(struct GameState *gs, struct Pos to);
//special without castling
int handle_en_passant(struct GameState *gs, struct Pos from, struct Pos to);
int handle_promotion(struct GameState *gs, struct Pos to);
int handle_anteater(struct GameState *gs, struct Pos from, struct Pos to);

#endif
