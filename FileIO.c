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

int load(struct GameState *gs, const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Load failed");
        return 0;
    }

    free_board(gs->board);

    fscanf(fp, "%d %d %d %d %d",
        &gs->current_turn,
        &gs->white_castle_k,
        &gs->white_castle_q,
        &gs->black_castle_k,
        &gs->black_castle_q);

    fscanf(fp, "%d %d",
        &gs->en_passant_target.r,
        &gs->en_passant_target.f);

    for (int r = 0; r < NUM_RANKS; r++) {
        for (int f = 0; f < NUM_FILES; f++) {
            int type, color;
            fscanf(fp, "%d %d", &type, &color);

            if (type == -1)
                gs->board[r][f] = NULL;
            else
                gs->board[r][f] = make_piece(type, color);
        }
    }

    fclose(fp);
    return 1;
}

void list_saves()
{
    DIR *d;
    struct dirent *dir;

    d = opendir(".");
    if (!d) {
        perror("Could not open directory");
        return;
    }

    printf("Available save files:\n");

    while ((dir = readdir(d)) != NULL) {
        if (strstr(dir->d_name, ".txt")) {
            printf(" - %s\n", dir->d_name);
        }
    }

    closedir(d);
}
