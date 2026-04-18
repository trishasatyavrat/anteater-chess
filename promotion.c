int handle_promotion(struct GameState *gs, struct Pos to)
{
    struct Piece *p = lookup_piece(gs->board, to);

    if (!p || p->type != PAWN)
        return 0;

    if ((p->color == WHITE && to.r == NUM_RANKS - 1) ||
        (p->color == BLACK && to.r == 0)) {

        /* Auto promote to Queen (can expand later) */
        p->type = QUEEN;
        return 1;
    }

    return 0;
}
