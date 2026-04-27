#include "board.h"
#include "move.h"

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

//SPECIAL MOVE: castling
void castling_function(struct GameState *gs, struct Move *m)
{
        //find the KING
        struct Piece *king = lookup_piece(gs->board, m->from);
        if(!king) return; //if the position does not contain a KING, then CASTLING can't occur

        //for undo_move (lets the function know there were MULTIPLE PIECES moved)
        m->was_castling = 1;
        int rank = m->from.r; //the RANK should stay 0 or 7 (black or white edge)

        //short-side of the board
        if(m->to.f == 7) //if the KING wants to move to FILE 7, if not CASTLING can't occur
        {
                m->rook_from = make_pos(rank, 9); //the ROOK should be at FILE 9 for castling to occur
                m->rook_to = make_pos(rank, 6); //the ROOK moves to FILE 6 when castling
        }
        //long-side of the board
        else if(m->to.f == 3) //if the KING wants to move to FILE 3, if not CASTLING can't occur
        {
                m->rook_from = make_pos(rank, 0); //the ROOK should be at FILE 0
                m->rook_to = make_pos(rank, 4); //the ROOK moves to FILE 4
                //(the QUEEN should already be moved)
        }

        //actually moving the PIECES
        struct Piece *rook = lookup_piece(gs->board, m->rook_from);
        if(rook)
        {
                assign_piece(gs->board, rook, m->rook_to); //Moves the ROOK to new location
                assign_piece(gs->board, NULL, m->rook_from); //Clears the ROOKfrom previous location
        }

        assign_piece(gs->board, king, m->to); //Moves the KING to new location
        assign_piece(gs->board, NULL, m->from); //Clears the KING from previous location

        //clears Castling Rights
        if (king->color == WHITE)
        {
                gs->white_castle_k = 0;
                gs->white_castle_q = 0;
        }
        else
        {
                gs->black_castle_k = 0;
                gs->black_castle_q = 0;
        }
}//end of castling_function FUNCTION

//SPECIAL MOVE: en_passant
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

//SPECIAL MOVE: ANTEATING
int handle_anteater(struct GameState *gs, struct Pos from, struct Pos to)
{
    //looks up the piece the player chooses
    struct Piece *p = lookup_piece(gs->board, from);

    //if the PIECE is NULL(EMPTY) or NOT an ANTEATER then do not proceed with the function
    if (!p || p->type != ANTEATER)
        return 0;

    //creates a 2D array to 'find' every ANT (PAWN)
    int dist[NUM_RANKS][NUM_FILES];
    for (int r = 0; r < NUM_RANKS; r++)
        for (int f = 0; f < NUM_FILES; f++)
            dist[r][f] = -1; //initialize the board spaces to NULL

    dist[from.r][from.f] = 0; //sets distance of the ANTEATER to 0
    int changed = 1; //new space with ANTs
    int current_d = 0; //distance to desired space

    //up, down, left, right
    int d_ortho[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    //directions with diagonals
    int d_all[8][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

    //loops to 'find' ANTS in the desired path
    while (changed && dist[to.r][to.f] == -1) {
        changed = 0;
        for (int r = 0; r < NUM_RANKS; r++) {
            for (int f = 0; f < NUM_FILES; f++) {
                if (dist[r][f] == current_d) {
                    //inital move
                    //initially the PAWN can only move like a KING unless CAPTURING ANTS
                    int dirs = (current_d == 0) ? 8 : 4;

                    //calculates coordinates of adjacent spaces
                    for (int i = 0; i < dirs; i++) {
                        int nr = r + (current_d == 0 ? d_all[i][0] : d_ortho[i][0]);
                        int nf = f + (current_d == 0 ? d_all[i][1] : d_ortho[i][1]);

                        if (pos_valid(make_pos(nr, nf)) && dist[nr][nf] == -1) {
                            struct Piece *target = gs->board[nr][nf]; //what's on the adjacent space
                            
                            //ANTEATER is only allowed to capture an OPPONENTS ANTS
                            if (target && target->type == PAWN && target->color != p->color) {
                                dist[nr][nf] = current_d + 1; //allows the ANTEATER to go there if desired
                                changed = 1;
                            }
                        }
                    }
                }
            }
        }
        current_d++;
    }

    if (dist[to.r][to.f] == -1) return 0; //makes sure the ANTEATER is moving to an ANT(PAWN)

    struct Pos curr = to;
    //loops from the desired space to the starting position
    while (curr.r != from.r || curr.f != from.f) {
        int d = dist[curr.r][curr.f];

        // Eat the ant
        free(gs->board[curr.r][curr.f]); //free the memory
        gs->board[curr.r][curr.f] = NULL; //deletes the piece from the board


        int dirs = (d == 1) ? 8 : 4;
        for (int i = 0; i < dirs; i++) {
            int nr = curr.r + (d == 1 ? d_all[i][0] : d_ortho[i][0]);
            int nf = curr.f + (d == 1 ? d_all[i][1] : d_ortho[i][1]);

            if (pos_valid(make_pos(nr, nf)) && dist[nr][nf] == d - 1) {
                curr = make_pos(nr, nf);
                break;
            }
        }
    }

    
    assign_piece(gs->board, p, to); //places the ANTEATER on the desired space
    assign_piece(gs->board, NULL, from); //deletes the ANTEATER at the start

    return 1;
}



//SPECIAL MOVE: PROMOTION
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

//MOVE: CAPTURE
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

void apply_move(struct GameState *gs, struct Move m) {
    struct Piece *p = lookup_piece(gs->board, m.from);
    if (!p) return;

    int move_handled = 0;

    //SPECIAL MOVES:
    if (p->type == ANTEATER) {
        move_handled = handle_anteater(gs, m.from, m.to);
    }
    else if (p->type == PAWN) {
        move_handled = handle_en_passant(gs, m.from, m.to);
    }
    else if (p->type == KING && abs(m.to.f - m.from.f) >= 2) {
        castling_function(gs, &m);
        move_handled = 1;
    }

    // 2. BASIC MOVES
    if (!move_handled) {
        struct Piece *target = lookup_piece(gs->board, m.to);
        if (target) free(target);

        assign_piece(gs->board, p, m.to);
        assign_piece(gs->board, NULL, m.from);
    }

    //PROMOTION:
    if (p->type == PAWN) {
        handle_promotion(gs, m.to);
    }

    //Update En_passant Status
    if (p->type == PAWN && abs(m.to.r - m.from.r) == 2) {
        gs->en_passant_target = make_pos((m.from.r + m.to.r) / 2, m.from.f);
    } else {
        gs->en_passant_target = make_pos(-1, -1);
    }
}

