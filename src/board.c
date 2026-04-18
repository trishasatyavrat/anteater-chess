#include "board.h"

struct Piece *make_piece(enum PieceType type, enum PieceColor color)
{
    struct Piece *p = malloc(sizeof(struct Piece));
    if (!p) {
        fprintf(stderr, "FATAL: malloc failed in make_piece\n");
        exit(EXIT_FAILURE);
    }
    p->type  = type;
    p->color = color;
    return p;
}

const char *piece_char(struct Piece *p)
{
    static char buf[3];

    if (!p) {
        buf[0] = ' ';
        buf[1] = ' ';
        buf[2] = '\0';
        return buf;
    }

    buf[0] = (p->color == WHITE) ? 'w' : 'b';

    switch (p->type) {
        case KING:     buf[1] = 'K'; break;
        case QUEEN:    buf[1] = 'Q'; break;
        case ROOK:     buf[1] = 'R'; break;
        case BISHOP:   buf[1] = 'B'; break;
        case KNIGHT:   buf[1] = 'N'; break;
        case PAWN:     buf[1] = 'P'; break;
        case ANTEATER: buf[1] = 'A'; break;
        default:       buf[1] = '?'; break;
    }

    buf[2] = '\0';
    return buf;
}

struct Pos make_pos(int r, int f)
{
    struct Pos p;
    p.r = r;
    p.f = f;
    return p;
}

int pos_valid(struct Pos p)
{
    return (p.r >= 0 && p.r < NUM_RANKS &&
            p.f >= 0 && p.f < NUM_FILES);
}

struct Piece *lookup_piece(struct Piece *board[NUM_RANKS][NUM_FILES],
                           struct Pos pos)
{
    if (!pos_valid(pos))
        return NULL;
    return board[pos.r][pos.f];
}

void assign_piece(struct Piece *board[NUM_RANKS][NUM_FILES],
                  struct Piece *piece,
                  struct Pos pos)
{
    if (!pos_valid(pos)) {
        fprintf(stderr,
                "WARNING: assign_piece — position (%d,%d) out of bounds\n",
                pos.r, pos.f);
        return;
    }
    board[pos.r][pos.f] = piece;
}

void free_board(struct Piece *board[NUM_RANKS][NUM_FILES])
{
    int r, f;
    for (r = 0; r < NUM_RANKS; r++) {
        for (f = 0; f < NUM_FILES; f++) {
            if (board[r][f]) {
                free(board[r][f]);
                board[r][f] = NULL;
            }
        }
    }
}

void init_game(struct GameState *gs)
{
    int r, f;

    if (!gs) {
        fprintf(stderr, "ERROR: init_game called with NULL\n");
        return;
    }

    free_board(gs->board);

    for (r = 0; r < NUM_RANKS; r++)
        for (f = 0; f < NUM_FILES; f++)
            gs->board[r][f] = NULL;

    gs->board[0][0] = make_piece(ROOK,     WHITE);
    gs->board[0][1] = make_piece(KNIGHT,   WHITE);
    gs->board[0][2] = make_piece(BISHOP,   WHITE);
    gs->board[0][3] = make_piece(ANTEATER, WHITE);
    gs->board[0][4] = make_piece(QUEEN,    WHITE);
    gs->board[0][5] = make_piece(KING,     WHITE);
    gs->board[0][6] = make_piece(ANTEATER, WHITE);
    gs->board[0][7] = make_piece(BISHOP,   WHITE);
    gs->board[0][8] = make_piece(KNIGHT,   WHITE);
    gs->board[0][9] = make_piece(ROOK,     WHITE);

    for (f = 0; f < NUM_FILES; f++)
        gs->board[1][f] = make_piece(PAWN, WHITE);

    for (f = 0; f < NUM_FILES; f++)
        gs->board[6][f] = make_piece(PAWN, BLACK);

    gs->board[7][0] = make_piece(ROOK,     BLACK);
    gs->board[7][1] = make_piece(KNIGHT,   BLACK);
    gs->board[7][2] = make_piece(BISHOP,   BLACK);
    gs->board[7][3] = make_piece(ANTEATER, BLACK);
    gs->board[7][4] = make_piece(QUEEN,    BLACK);
    gs->board[7][5] = make_piece(KING,     BLACK);
    gs->board[7][6] = make_piece(ANTEATER, BLACK);
    gs->board[7][7] = make_piece(BISHOP,   BLACK);
    gs->board[7][8] = make_piece(KNIGHT,   BLACK);
    gs->board[7][9] = make_piece(ROOK,     BLACK);

    gs->current_turn      = WHITE;
    gs->white_castle_k    = 1;
    gs->white_castle_q    = 1;
    gs->black_castle_k    = 1;
    gs->black_castle_q    = 1;
    gs->en_passant_target = make_pos(-1, -1);

    gs->history.count = 0;
    memset(gs->history.moves, 0, sizeof(gs->history.moves));
}

void display_board(struct Piece *board[NUM_RANKS][NUM_FILES])
{
    int r, f;
    const char *divider =
        "   +----+----+----+----+----+----+----+----+----+----+";

    printf("\n");

    for (r = NUM_RANKS - 1; r >= 0; r--) {
        printf("%s\n", divider);
        printf(" %d ", r + 1);
        for (f = 0; f < NUM_FILES; f++)
            printf("| %s ", piece_char(board[r][f]));
        printf("|\n");
    }

    printf("%s\n", divider);
    printf("     A    B    C    D    E    F    G    H    I    J\n\n");
}