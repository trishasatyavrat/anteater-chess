/*
 * test_boarddisplay.c
 * Tests that the board initializes and displays correctly.
 *
 * How to compile (from the chess/ directory):
 *   gcc -o bin/test_boarddisplay src/test_boarddisplay.c src/board.c -I src/
 *
 * How to run:
 *   ./bin/test_boarddisplay
 */

#include "chess.h"   /* gives us GameState, Piece, Pos, etc. */
#include "board.h"   /* gives us init_game, display_board, lookup_piece, etc. */

/* ------------------------------------------------------------------ *
 * Helper: print PASS or FAIL with a message                          *
 * ------------------------------------------------------------------ */
static int tests_run    = 0;  /* total tests attempted */
static int tests_passed = 0;  /* total tests that passed */

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
 * Test 1: init_game does not crash and fills the board               *
 * ------------------------------------------------------------------ */
static void test_init_game(void)
{
    printf("\n--- Test: init_game ---\n");

    struct GameState gs;

    /* Zero out the struct so there are no random garbage pointers */
    memset(&gs, 0, sizeof(gs));

    init_game(&gs);   /* should not crash */

    /* White back rank (row 0) */
    check(lookup_piece(gs.board, make_pos(0, 0)) != NULL,  "White rook   at A1 exists");
    check(lookup_piece(gs.board, make_pos(0, 1)) != NULL,  "White knight at B1 exists");
    check(lookup_piece(gs.board, make_pos(0, 2)) != NULL,  "White bishop at C1 exists");
    check(lookup_piece(gs.board, make_pos(0, 3)) != NULL,  "White anteater at D1 exists");
    check(lookup_piece(gs.board, make_pos(0, 4)) != NULL,  "White queen  at E1 exists");
    check(lookup_piece(gs.board, make_pos(0, 5)) != NULL,  "White king   at F1 exists");

    /* White pawns (row 1) */
    int all_white_pawns = 1;
    int f;
    for (f = 0; f < NUM_FILES; f++) {
        struct Piece *p = lookup_piece(gs.board, make_pos(1, f));
        if (!p || p->type != PAWN || p->color != WHITE)
            all_white_pawns = 0;
    }
    check(all_white_pawns, "All 10 white pawns on rank 2");

    /* Black pawns (row 6) */
    int all_black_pawns = 1;
    for (f = 0; f < NUM_FILES; f++) {
        struct Piece *p = lookup_piece(gs.board, make_pos(6, f));
        if (!p || p->type != PAWN || p->color != BLACK)
            all_black_pawns = 0;
    }
    check(all_black_pawns, "All 10 black pawns on rank 7");

    /* Black back rank (row 7) */
    check(lookup_piece(gs.board, make_pos(7, 5)) != NULL,  "Black king   at F8 exists");
    check(lookup_piece(gs.board, make_pos(7, 4)) != NULL,  "Black queen  at E8 exists");

    /* Middle rows should be empty */
    int middle_empty = 1;
    int r;
    for (r = 2; r <= 5; r++) {
        for (f = 0; f < NUM_FILES; f++) {
            if (lookup_piece(gs.board, make_pos(r, f)) != NULL)
                middle_empty = 0;
        }
    }
    check(middle_empty, "Ranks 3-6 are all empty at start");

    /* Turn should start with WHITE */
    check(gs.current_turn == WHITE, "It is white's turn at start");

    free_board(gs.board);   /* clean up allocated memory */
}

/* ------------------------------------------------------------------ *
 * Test 2: piece colors and types are set correctly after init        *
 * ------------------------------------------------------------------ */
static void test_piece_types(void)
{
    printf("\n--- Test: piece types and colors ---\n");

    struct GameState gs;
    memset(&gs, 0, sizeof(gs));
    init_game(&gs);

    /* Check a few specific pieces */
    struct Piece *p;

    p = lookup_piece(gs.board, make_pos(0, 0));  /* A1 = white rook */
    check(p && p->type == ROOK  && p->color == WHITE, "A1 is white ROOK");

    p = lookup_piece(gs.board, make_pos(0, 1));  /* B1 = white knight */
    check(p && p->type == KNIGHT && p->color == WHITE, "B1 is white KNIGHT");

    p = lookup_piece(gs.board, make_pos(0, 3));  /* D1 = white anteater */
    check(p && p->type == ANTEATER && p->color == WHITE, "D1 is white ANTEATER");

    p = lookup_piece(gs.board, make_pos(0, 5));  /* F1 = white king */
    check(p && p->type == KING  && p->color == WHITE, "F1 is white KING");

    p = lookup_piece(gs.board, make_pos(7, 5));  /* F8 = black king */
    check(p && p->type == KING  && p->color == BLACK, "F8 is black KING");

    p = lookup_piece(gs.board, make_pos(7, 3));  /* D8 = black anteater */
    check(p && p->type == ANTEATER && p->color == BLACK, "D8 is black ANTEATER");

    free_board(gs.board);
}

/* ------------------------------------------------------------------ *
 * Test 3: piece_char returns the right two-letter code               *
 * ------------------------------------------------------------------ */
static void test_piece_char(void)
{
    printf("\n--- Test: piece_char labels ---\n");

    /* Make a few temporary pieces just to test the label function */
    struct Piece *wk = make_piece(KING,     WHITE);
    struct Piece *bq = make_piece(QUEEN,    BLACK);
    struct Piece *wa = make_piece(ANTEATER, WHITE);
    struct Piece *bp = make_piece(PAWN,     BLACK);

    check(strcmp(piece_char(wk), "wK") == 0, "White king   -> 'wK'");
    check(strcmp(piece_char(bq), "bQ") == 0, "Black queen  -> 'bQ'");
    check(strcmp(piece_char(wa), "wA") == 0, "White anteater -> 'wA'");
    check(strcmp(piece_char(bp), "bP") == 0, "Black pawn   -> 'bP'");
    check(strcmp(piece_char(NULL), "  ") == 0, "NULL piece   -> '  ' (two spaces)");

    free(wk);
    free(bq);
    free(wa);
    free(bp);
}

/* ------------------------------------------------------------------ *
 * Test 4: pos_valid rejects out-of-bounds positions                  *
 * ------------------------------------------------------------------ */
static void test_pos_valid(void)
{
    printf("\n--- Test: pos_valid ---\n");

    check(pos_valid(make_pos(0, 0)) == 1,  "A1 (0,0) is valid");
    check(pos_valid(make_pos(7, 9)) == 1,  "J8 (7,9) is valid");
    check(pos_valid(make_pos(4, 4)) == 1,  "E5 (4,4) is valid");
    check(pos_valid(make_pos(-1, 0)) == 0, "(-1,0) is invalid");
    check(pos_valid(make_pos(0, -1)) == 0, "(0,-1) is invalid");
    check(pos_valid(make_pos(8, 0))  == 0, "(8,0)  is invalid (rank too high)");
    check(pos_valid(make_pos(0, 10)) == 0, "(0,10) is invalid (file too high)");
}

/* ------------------------------------------------------------------ *
 * Test 5: visual display (just prints; human checks it looks right)  *
 * ------------------------------------------------------------------ */
static void test_display_board(void)
{
    printf("\n--- Test: display_board (visual check) ---\n");
    printf("The board below should show the standard Anteater Chess start:\n");
    printf("Row 8: bR bN bB bA bQ bK bA bB bN bR\n");
    printf("Row 7: bP x10\n");
    printf("Rows 6-3: empty\n");
    printf("Row 2: wP x10\n");
    printf("Row 1: wR wN wB wA wQ wK wA wB wN wR\n\n");

    struct GameState gs;
    memset(&gs, 0, sizeof(gs));
    init_game(&gs);

    display_board(gs.board);   /* prints to stdout — visually verify */

    check(1, "display_board ran without crashing");

    free_board(gs.board);
}

/* ------------------------------------------------------------------ *
 * main: run all tests and print summary                              *
 * ------------------------------------------------------------------ */
int main(void)
{
    printf("==============================================\n");
    printf("  test_boarddisplay: Board Display Tests\n");
    printf("==============================================\n");

    test_init_game();
    test_piece_types();
    test_piece_char();
    test_pos_valid();
    test_display_board();

    printf("\n==============================================\n");
    printf("  Results: %d / %d tests passed\n", tests_passed, tests_run);
    printf("==============================================\n");

    /* Return 0 (success) only if everything passed */
    return (tests_passed == tests_run) ? 0 : 1;
}
