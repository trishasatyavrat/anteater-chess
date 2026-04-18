#include <stdio.h>
#include <stdlib.h>
#include "move.h"

int move_piece(struct GameState *gs, struct Pos from, struct Pos to)
{
    struct Piece *p = lookup_piece(gs->board, from);

    if (!p) {
        printf("Invalid! Choose a spot with a piece on it.\n");
        return 0;
    }

    if (p->color != gs->current_turn) {
        printf("Not your piece!\n");
        return 0;
    }

    //eat antes
    if (p->type == ANTEATER) {
        if (handle_anteater(gs, from, to)) {
            gs->current_turn = (gs->current_turn == WHITE) ? BLACK : WHITE;
            return 1;
        }
    }

    // en passant
    if (p->type == PAWN) {
        if (handle_en_passant(gs, from, to)) {
            gs->current_turn = (gs->current_turn == WHITE) ? BLACK : WHITE;
            return 1;
        }
    }

    // if capture
    handle_capture(gs, to);

    //move piece
    assign_piece(gs->board, p, to);
    assign_piece(gs->board, NULL, from);

    // ant promotion
    if (p->type == PAWN)
        handle_promotion(gs, to);

    /* Update en passant target */
    if (p->type == PAWN && abs(to.r - from.r) == 2) {
        gs->en_passant_target = make_pos((from.r + to.r) / 2, from.f);
    } else {
        gs->en_passant_target = make_pos(-1, -1);
    }

    // update turn
    gs->current_turn = (gs->current_turn == WHITE) ? BLACK : WHITE;

    return 1;
}
