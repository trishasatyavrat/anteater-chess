#ifndef FILEIO_H
#define FILEIO_H

#include "board.h"
int save(struct GameState *gs, const char *filename);
int load(struct GameState *gs, const char *filename);

void list_saves();
#endif
