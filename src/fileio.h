#ifndef FILEIO_H
#define FILEIO_H

#include "chess.h"

int logfile_function(struct Move move, int turn_number,
                     enum PieceColor color, const char *filename);
int save_function(struct GameState *gs, const char *filename);
int load_function(struct GameState *gs, const char *filename);

#endif
