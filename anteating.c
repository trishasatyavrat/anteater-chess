int handle_anteater(struct GameState *gs, struct Pos from, struct Pos to)
{
    struct Piece *p = lookup_piece(gs->board, from);

    if (!p || p->type != ANTEATER)
        return 0;

    int dr = to.r - from.r;
    int df = to.f - from.f;

    // moves straight/dia
    if (dr != 0) dr = dr / abs(dr);
    if (df != 0) df = df / abs(df);

    int r = from.r + dr;
    int f = from.f + df;

    int ate = 0;

    while (pos_valid(make_pos(r, f))) {
        struct Piece *target = gs->board[r][f];

        if (target && target->type == PAWN) {
            free(target);
            gs->board[r][f] = NULL;
            ate = 1;

            if (r == to.r && f == to.f)
                break;

            r += dr;
            f += df;
        } else {
            break;
        }
    }

    if (!ate)
        return 0;

    //  anteater at end of board
    assign_piece(gs->board, p, make_pos(r, f));
    assign_piece(gs->board, NULL, from);

    return 1;
}
