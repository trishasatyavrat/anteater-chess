#include "board.h"

void log_move(struct GameState *gs, struct Move m)
{
    if (!gs) return;

    if (gs->history.count >= MAX_MOVES) {
        fprintf(stderr,
                "WARNING: move log full (%d entries) — move not recorded\n",
                MAX_MOVES);
        return;
    }

    gs->history.moves[gs->history.count] = m;
    gs->history.count++;
}

void undo_move(struct GameState *gs)
{
    if (!gs) return;

    if (gs->history.count == 0) {
        printf("Nothing to undo.\n");
        return;
    }

    gs->history.count--;
    struct Move m = gs->history.moves[gs->history.count];

    struct Piece *moving = lookup_piece(gs->board, m.to);
    assign_piece(gs->board, moving, m.from);
    assign_piece(gs->board, NULL,   m.to);

    if (m.was_promotion && moving)
        moving->type = PAWN;

    if ((int)m.captured.type != -1) {
        struct Piece *victim =
            make_piece(m.captured.type, m.captured.color);

        if (m.was_en_passant)
            assign_piece(gs->board, victim, m.ep_cap_pos);
        else
            assign_piece(gs->board, victim, m.to);
    }

    if (m.was_castling) {
        struct Piece *rook = lookup_piece(gs->board, m.rook_to);
        assign_piece(gs->board, rook, m.rook_from);
        assign_piece(gs->board, NULL, m.rook_to);
    }

    gs->white_castle_k = m.prev_wck;
    gs->white_castle_q = m.prev_wcq;
    gs->black_castle_k = m.prev_bck;
    gs->black_castle_q = m.prev_bcq;

    gs->en_passant_target = m.prev_ep;

    gs->current_turn = OPPONENT(gs->current_turn);

    printf("Move undone!\n");
}