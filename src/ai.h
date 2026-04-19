//Header Guards
#ifndef AI_H
#define AI_H

//Call struct Move, struct GameState, and enum PieceColor
#include "chess.h"

struct Move bestmove_function(struct GameState *gs, enum PieceColor color, int depth);
void ai_legalmoves_function(struct GameState *gs, enum PieceColor color, struct Move *out, int *count);
int scan_board(struct GameState *gs, enum PieceColor color);
int minimax(struct GameState *gs, enum PieceColor turn_color, enum PieceColor ai_color, int depth);


//End Guard
#endif