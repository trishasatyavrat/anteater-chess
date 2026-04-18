int handle_capture(struct GameState *gs, struct Pos to)
{
    struct Piece *target = lookup_piece(gs->board, to);

    if (target) {
        free(target);
        assign_piece(gs->board, NULL, to);
        return 1;
    }
    return 0;
}
