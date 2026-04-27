#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chess.h"
#include "fileio.h"
#include "rules.h"
#include "ai.h"
#ifndef TEXT_ONLY
#include "gui.h"
#endif

void show_main_menu(void) {
    printf("\n========================================\n");
    printf("       ANTEATER CHESS - Alpha           \n");
    printf("       The Knight Owls - Team 3         \n");
    printf("========================================\n");
    printf("  1. Play (Human vs Computer)           \n");
    printf("  2. Quit                               \n");
    printf("========================================\n");
    printf("Enter choice: ");
}

void show_pause_menu(void) {
    printf("\n========================================\n");
    printf("              PAUSED                    \n");
    printf("========================================\n");
    printf("  1. Resume                             \n");
    printf("  2. Save Game                          \n");
    printf("  3. Quit                               \n");
    printf("========================================\n");
    printf("Enter choice: ");
}

struct Move get_user_move(struct GameState *gs) {
    struct Move m;
    char input[20];
    char from[5], to[5];
    printf("Enter your move (example: F2 F4): ");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%s %s", from, to);
    m.from.f = from[0] - 'A';
    m.from.r = from[1] - '1';
    m.to.f   = to[0] - 'A';
    m.to.r   = to[1] - '1';
    return m;
}

int main(void) {
    int choice;
    char input[10];
    struct GameState gs;

    show_main_menu();
    fgets(input, sizeof(input), stdin);
    choice = atoi(input);

    if (choice == 2) {
        printf("Goodbye!\n");
        return 0;
    }

    memset(&gs, 0, sizeof(gs));
    init_game(&gs);

    
    int level;

    printf("Choose difficulty (1 = Beginner, 2 = Intermediate, 3 = Expert): ");
    fgets(input, sizeof(input), stdin);
    level = atoi(input);

    if (level < 1 || level > 3){
    level = 1;
}
    
    difficultyselect_function(level);
    
    
    
    #ifndef TEXT_ONLY
        printf("Game mode: (1) Human vs Computer  (2) Human vs Human: ");
        fgets(input, sizeof(input), stdin);
        int mode = atoi(input);

        enum PieceColor human_color = WHITE;
        if (gui_init() == 0) {
            gui_run(&gs, human_color, get_ai_depth(), mode);
            gui_shutdown();
            return 0;
        }
        fprintf(stderr, "GUI failed, falling back to text mode.\n");
    #endif

    display_board(gs.board);

    printf("You are playing as White.\n");
    printf("Type your moves as: FROM TO (example: F2 F4)\n\n");

    while (1) {
        if (gs.current_turn == WHITE) {
            printf("\nYour turn (White):\n");
            struct Move m = get_user_move(&gs);
            if (!is_legal_move(&gs, m)) {
                printf("Illegal move! Try again.\n");
                continue;
            }
            apply_move(&gs, m);
            gs.current_turn = BLACK;
            logfile_function(m, gs.history.count, WHITE, "chess_log.txt");
            display_board(gs.board);
            if (king_in_checkmate(&gs, BLACK)) {
                printf("Checkmate! White wins!\n");
                break;
            }
            if (king_in_stalemate(&gs, BLACK)) {
                printf("Stalemate! Draw!\n");
                break;
            }
        } else {
            printf("\nComputer is thinking...\n");
            struct Move m = bestmove_function(&gs, BLACK, get_ai_depth()); 
            apply_move(&gs, m);
            gs.current_turn = WHITE;
            logfile_function(m, gs.history.count, BLACK, "chess_log.txt");
            display_board(gs.board);
            if (king_in_checkmate(&gs, WHITE)) {
                printf("Checkmate! Black wins!\n");
                break;
            }
            if (king_in_stalemate(&gs, WHITE)) {
                printf("Stalemate! Draw!\n");
                break;
            }
        }
    }
    return 0;
}
