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

void play_game()
{
    struct GameState gs;
    init_game(&gs);

    char input[100];

    while (1) {
        display_board(gs.board);

        printf("%s to move\n",
            (gs.current_turn == WHITE) ? "White" : "Black");

        printf("Commands:\n");
        printf(" MOVE: E2 E4\n");
        printf(" SAVE <filename>\n");
        printf(" LOAD <filename>\n");
        printf(" LIST\n");
        printf(" EXIT\n");

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;

        /* EXIT */
        if (strcmp(input, "EXIT") == 0)
            break;

        /* LIST saves */
        if (strcmp(input, "LIST") == 0) {
            list_saves();
            continue;
        }

        /* SAVE */
        if (strncmp(input, "SAVE ", 5) == 0) {
            char filename[50];
            sscanf(input + 5, "%s", filename);

            if (save(&gs, filename))
                printf("Saved to %s\n", filename);

            continue;
        }

        /* LOAD */
        if (strncmp(input, "LOAD ", 5) == 0) {
            char filename[50];
            sscanf(input + 5, "%s", filename);

            if (load(&gs, filename))
                printf("Loaded %s\n", filename);

            continue;
        }

        /* MOVE */
        char from_str[3], to_str[3];
        if (sscanf(input, "%2s %2s", from_str, to_str) == 2) {

            struct Pos from = make_pos(from_str[1] - '1', from_str[0] - 'A');
            struct Pos to   = make_pos(to_str[1] - '1', to_str[0] - 'A');

            if (!move_piece(&gs, from, to)) {
                printf("Invalid move\n");
            }

            continue;
        }

        printf("Invalid command\n");
    }

    free_board(gs.board);
}
