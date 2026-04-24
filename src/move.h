#ifndef MOVE_H
#define MOVE_H
#include "chess.h"
#include "rules.h"
#include "board.h"
void castling_function(struct GameState *gs, struct Move *m);

//capture
int handle_capture(struct GameState *gs, struct Pos to);

//special without castling
int handle_en_passant(struct GameState *gs, struct Pos from, struct Pos to);
int handle_promotion(struct GameState *gs, struct Pos to);
int handle_anteater(struct GameState *gs, struct Pos from, struct Pos to);

#endif
