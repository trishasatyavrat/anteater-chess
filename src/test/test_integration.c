/*
 * test_integration.c
 * Integration test: verifies that game state changes correctly
 * when pieces are moved and that undo works.
 *
 * How to compile:
 *   gcc -o bin/test_integration src/test_integration.c src/board.c src/move.c -I src/
 *
 * How to run:
 *   ./bin/test_integration
 */

#include "chess.h"
#include "board.h"

/* ------------------------------------------------------------------ *
 * Simple pass/fail tracker (same pattern as test_boarddisplay.c)     *
 * ------------------------------------------------------------------ */
static int tests_run    = 0;
static int tests_passed = 0;

static void check(int condition, const char *test_name)
{
    tests_run++;
    if (condition) {
        printf("  PASS: %s\n", test_name);
        tests_passed++;
    } else {
        printf("  FAIL: %s\n", test_name);
    }
}

/* ------------------------------------------------------------------ *
 * Helper: build a simple Move struct for a normal (non-special) move *
 *                                                                     *
 * We pass:                                                            *
 *   from_r, from_f  — rank and file of the piece we are moving       *
 *   to_r,   to_f    — rank and file of the destination               *
 *   gs              — the game state (so we can read the board)      *
 * ------------------------------------------------------------------ */
static struct Move make_simple_move(struct GameState *gs,
                                    int from_r, int from_f,
                                    int to_r,   int to_f)
{
    struct Move m;
    memset(&m, 0, sizeof(m));     /* zero everything out first */

    m.from = make_pos(from_r, from_f);
    m.to   = make_pos(to_r,   to_f);

    /* Record what piece (if any) is being captured */
    struct Piece *victim = lookup_piece(gs->board, m.to);
    if (victim) {
        m.captured = *victim;     /* copy the piece value */
    } else {
        /* No capture — use -1 as a sentinel to mean "nothing captured" */
        m.captured.type  = (enum PieceType)-1;
        m.captured.color = WHITE;  /* doesn't matter when type is -1 */
    }

    /* Save castling rights and en-passant so undo_move can restore them */
    m.prev_wck = gs->white_castle_k;
    m.prev_wcq = gs->white_castle_q;
    m.prev_bck = gs->black_castle_k;
    m.prev_bcq = gs->black_castle_q;
    m.prev_ep  = gs->en_passant_target;

    /* Not a special move */
    m.was_en_passant = 0;
    m.was_castling   = 0;
    m.was_promotion  = 0;

    return m;
}

/* ------------------------------------------------------------------ *
 * Helper: apply a simple move to the board manually                  *
 * (We use this because apply_move lives in rules.c which may not be  *
 *  compiled yet — this keeps the test self-contained.)               *
 * ------------------------------------------------------------------ */
static void apply_simple_move(struct GameState *gs, struct Move m)
{
    struct Piece *moving = lookup_piece(gs->board, m.from);
    struct Piece *victim = lookup_piece(gs->board, m.to);

    /* If capturing, free the victim's memory */
    if (victim)
        free(victim);

    assign_piece(gs->board, moving, m.to);   /* put piece on new square */
    assign_piece(gs->board, NULL,   m.from); /* clear old square */

    log_move(gs, m);  /* record it in history so undo_move works */

    gs->current_turn = OPPONENT(gs->current_turn); /* switch turns */
}

/* ------------------------------------------------------------------ *
 * Test 1: moving a pawn updates the board correctly                  *
 * ------------------------------------------------------------------ */
static void test_pawn_move(void)
{
    printf("\n--- Test: pawn move (F2 -> F4) ---\n");

    struct GameState gs;
    memset(&gs, 0, sizeof(gs));
    init_game(&gs);

    /* White pawn starts at F2 = rank index 1, file index 5 */
    struct Piece *before = lookup_piece(gs.board, make_pos(1, 5));
    check(before != NULL && before->type == PAWN && before->color == WHITE,
          "White pawn exists at F2 before move");

    /* Move white pawn from F2 (1,5) to F4 (3,5) */
    struct Move m = make_simple_move(&gs, 1, 5, 3, 5);
    apply_simple_move(&gs, m);

    /* After move: F4 should have the pawn, F2 should be empty */
    struct Piece *at_f4 = lookup_piece(gs.board, make_pos(3, 5));
    struct Piece *at_f2 = lookup_piece(gs.board, make_pos(1, 5));

    check(at_f4 != NULL && at_f4->type == PAWN && at_f4->color == WHITE,
          "White pawn is now at F4 after move");
    check(at_f2 == NULL,
          "F2 is empty after the pawn moved away");
    check(gs.current_turn == BLACK,
          "It is now black's turn after white moved");

    free_board(gs.board);
}

/* ------------------------------------------------------------------ *
 * Test 2: undo_move puts the pawn back                               *
 * ------------------------------------------------------------------ */
static void test_undo_pawn_move(void)
{
    printf("\n--- Test: undo pawn move ---\n");

    struct GameState gs;
    memset(&gs, 0, sizeof(gs));
    init_game(&gs);

    /* Apply a move, then undo it */
    struct Move m = make_simple_move(&gs, 1, 5, 3, 5);  /* F2 -> F4 */
    apply_simple_move(&gs, m);
    undo_move(&gs);

    /* After undo, pawn should be back at F2 */
    struct Piece *at_f2 = lookup_piece(gs.board, make_pos(1, 5));
    struct Piece *at_f4 = lookup_piece(gs.board, make_pos(3, 5));

    check(at_f2 != NULL && at_f2->type == PAWN && at_f2->color == WHITE,
          "Pawn is back at F2 after undo");
    check(at_f4 == NULL,
          "F4 is empty after undo");
    check(gs.current_turn == WHITE,
          "It is white's turn again after undo");

    free_board(gs.board);
}

/* ------------------------------------------------------------------ *
 * Test 3: undo on an empty history does not crash                    *
 * ------------------------------------------------------------------ */
static void test_undo_empty(void)
{
    printf("\n--- Test: undo on empty history ---\n");

    struct GameState gs;
    memset(&gs, 0, sizeof(gs));
    init_game(&gs);

    undo_move(&gs);   /* should print "Nothing to undo." and not crash */
    check(gs.history.count == 0, "History count stays 0 after undo on empty log");

    free_board(gs.board);
}

/* ------------------------------------------------------------------ *
 * Test 4: multiple moves are logged correctly                        *
 * ------------------------------------------------------------------ */
static void test_move_log(void)
{
    printf("\n--- Test: move log records multiple moves ---\n");

    struct GameState gs;
    memset(&gs, 0, sizeof(gs));
    init_game(&gs);

    /* Move 1: white pawn A2 -> A3  (rank 1,file 0) -> (rank 2,file 0) */
    struct Move m1 = make_simple_move(&gs, 1, 0, 2, 0);
    apply_simple_move(&gs, m1);
    check(gs.history.count == 1, "History has 1 move after first move");

    /* Move 2: black pawn A7 -> A6  (rank 6,file 0) -> (rank 5,file 0) */
    struct Move m2 = make_simple_move(&gs, 6, 0, 5, 0);
    apply_simple_move(&gs, m2);
    check(gs.history.count == 2, "History has 2 moves after second move");

    /* Undo the second move */
    undo_move(&gs);
    check(gs.history.count == 1, "History has 1 move after one undo");

    /* Undo the first move */
    undo_move(&gs);
    check(gs.history.count == 0, "History is empty after undoing both moves");

    free_board(gs.board);
}

/* ------------------------------------------------------------------ *
 * Test 5: turn alternates correctly over several moves               *
 * ------------------------------------------------------------------ */
static void test_turn_alternation(void)
{
    printf("\n--- Test: turn alternates white/black ---\n");

    struct GameState gs;
    memset(&gs, 0, sizeof(gs));
    init_game(&gs);

    check(gs.current_turn == WHITE, "Turn 0: white's turn");

    struct Move m1 = make_simple_move(&gs, 1, 0, 2, 0);
    apply_simple_move(&gs, m1);
    check(gs.current_turn == BLACK, "Turn 1: black's turn");

    struct Move m2 = make_simple_move(&gs, 6, 0, 5, 0);
    apply_simple_move(&gs, m2);
    check(gs.current_turn == WHITE, "Turn 2: white's turn again");

    free_board(gs.board);
}

/* ------------------------------------------------------------------ *
 * main                                                               *
 * ------------------------------------------------------------------ */
int main(void)
{
    printf("==============================================\n");
    printf("  test_integration: Integration Tests\n");
    printf("==============================================\n");

    test_pawn_move();
    test_undo_pawn_move();
    test_undo_empty();
    test_move_log();
    test_turn_alternation();

    printf("\n==============================================\n");
    printf("  Results: %d / %d tests passed\n", tests_passed, tests_run);
    printf("==============================================\n");

    return (tests_passed == tests_run) ? 0 : 1;
}
