#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chess.h"

int logfile_function(struct Move move, int turn_number, enum PieceColor color, const char *filename) {
    FILE *fp = fopen(filename, "a");
    if (fp == NULL) { printf("Error: Could not open log file.\n"); return -1; }
    char color_char = (color == WHITE) ? 'W' : 'B';
    char from_file = 'A' + move.from.f;
    char to_file = 'A' + move.to.f;
    int from_rank = move.from.r + 1;
    int to_rank = move.to.r + 1;
    fprintf(fp, "%d.%c %c%d-%c%d\n", turn_number, color_char, from_file, from_rank, to_file, to_rank);
    fclose(fp);
    return 0;
}

int save_function(struct GameState *gs, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) { printf("Error: Could not save game.\n"); return -1; }
    fprintf(fp, "TURN %d\n", gs->current_turn);
    fprintf(fp, "CASTLE %d %d %d %d\n", gs->white_castle_k, gs->white_castle_q, gs->black_castle_k, gs->black_castle_q);
    fprintf(fp, "MOVES %d\n", gs->history.count);
    for (int i = 0; i < gs->history.count; i++) {
        fprintf(fp, "%d %d %d %d\n", gs->history.moves[i].from.f, gs->history.moves[i].from.r, gs->history.moves[i].to.f, gs->history.moves[i].to.r);
    }
    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 10; f++) {
            if (gs->board[r][f] != NULL) {
                fprintf(fp, "PIECE %d %d %d %d\n", r, f, gs->board[r][f]->type, gs->board[r][f]->color);
            }
        }
    }
    fclose(fp);
    printf("Game saved to %s\n", filename);
    return 0;
}

int load_function(struct GameState *gs, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) { printf("Error: Could not load file %s\n", filename); return -1; }
    char line[100];
    for (int r = 0; r < 8; r++)
        for (int f = 0; f < 10; f++)
            gs->board[r][f] = NULL;
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "TURN", 4) == 0) {
            int t; sscanf(line, "TURN %d", &t);
            gs->current_turn = (enum PieceColor)t;
        } else if (strncmp(line, "PIECE", 5) == 0) {
            int r, f, type, color;
            sscanf(line, "PIECE %d %d %d %d", &r, &f, &type, &color);
            struct Piece *p = malloc(sizeof(struct Piece));
            p->type = (enum PieceType)type;
            p->color = (enum PieceColor)color;
            gs->board[r][f] = p;
        }
    }
    fclose(fp);
    printf("Game loaded from %s\n", filename);
    return 0;
}
