int handle_en_passant(struct GameState *gs, struct Pos from, struct Pos to)
{
    struct Piece *p = lookup_piece(gs->board, from);

    if (!p || p->type != PAWN)
        return 0;

    // moves to en passant target
    if (to.r == gs->en_passant_target.r &&
        to.f == gs->en_passant_target.f) {

        int dir = (p->color == WHITE) ? -1 : 1;
        struct Pos captured = make_pos(to.r + dir, to.f);

        struct Piece *enemy = lookup_piece(gs->board, captured);

        if (enemy && enemy->type == PAWN) {
            free(enemy);
            assign_piece(gs->board, NULL, captured);

            assign_piece(gs->board, p, to);
            assign_piece(gs->board, NULL, from);

            return 1;
        }
    }

    return 0;
}
