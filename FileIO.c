#include <stdio.h>
#include <stdlib.h>

#include "FileIO.h"
int save(struct GameState *gs, const char *filename) {
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    perror("Save failed");
    return 0;
  }
  fprint(fp, "%d %d %d %d %d\n", gs->current_turn, gs->white_castle_k, gs->white_castle_q, gs->black_castle_k, gs->black_castle_q);
  fprint(fp, "%d %d\n", gs->en_passant_target.r, gs->en_passant_target.f);

  for (int r=0; r<NUM_RANKS; r++) {
    for (int f=0; f < NUM_FILES; f++) {
      struct Piece *p = gs->board[r][f];
        if (p) {
          fprintf(fp, "%d %d ", p->type, p->color);
				} else {
					fprint(fp, "-1 -1 ");
				}
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
	return 1;
}

int load(struct GameState *gs, const char *filename) {
	FILE *fp
