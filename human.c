#include <stdio.h>
#include <string.h>

#include "board.h"
#include "move.h"
#include "FileIO.h"

static struct Pos parse_pos(const char *str) {
  int file = str[0]-'A';
  int rank = str[1] - '1';
  return make_pos(rank, file);
}

void play_game() {
  struct GameState gs;
  init_game(&gs);
  char input[100];
  while (1) {
    display_board(gs.board);
    printf("%s to move\n", (gs.current_turn==WHITE) ? "White": "Black");
    fgets(input, sizeof(input), stdin);

    input[strcspn(input, "\n")] =0;
    if (strcmp(input, "EXIT") == 0) {
      save(&gs, "save.txt");
      printf("Game saved. \n");
      continue;
    }
    if (strcmp(input, "LOAD") ==0) {
      load(&gs, "save.txt");
      printf("Game loaded.\n");
      continue;
    }

    char from_str[3], to_str[3];
    if (sscanf(input, %2s %2s", from_str, to_str) != 2) {
      printf("Invalid input\n");
      continue;
    }
    struct Pos from = parse_pos(from_str);
    struct Pos to = parse_pos(to_str);
    if (!move_piece(&gs, from, to)) {
      printf("Invalid move\n");
    }
  }
  free_board(gs.board);
}
