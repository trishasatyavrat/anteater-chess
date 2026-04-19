#include "rules.h"
#include "board.h"
#include <stdlib.h>
#include <math.h>

//create a copy of the current board (checking moves without disturbing the acutal game)
static struct GameState copy_of_board(struct GameState *gs)
{
	struct GameState copy = *gs;
	for (int r = 0; r < NUM_RANKS; r++) //loops for every RANK (MAX: 8)
	{
		for (int f = 0; f < NUM_FILES; f++) //loops for every FILE (MAX: 10)
		{
			//checks to see if theres a PIECE at that POSITION
			if(gs->board[r][f])
			{
				copy.board[r][f] = malloc(sizeof(struct Piece));
				//allocate memory for that position the size of a PIECE
				*(copy.board[r][f]) = *(gs->board[r][f]); 
				//copy parameters of the PIECE from original board
			}
			else //if NO PIECE then the Position is EMPTY = NULL
			{
				copy.board[r][f] = NULL;
			}//end of IF statements
		}//end of FOR (file) loop
	}//end of FOR (rank) loop
	return copy;
}//end of copy_of_board function

//clears the allocated memory
static void clear_board_copy(struct GameState *copy)
{
	for (int r = 0; r < NUM_RANKS; r++) //loops for the # of RANKS (8)
	{
		for (int f = 0; f < NUM_FILES; f++) //loops of the # of files (10)
		{
			if (copy->board[r][f])
			{
				free(copy->board[r][f]);
			}//end of IF statement
		}//end of FOR (file) loop
	}//end of FOR (rank) looop
}//end of clear_board_copy FUNCTION

int is_under_attack(struct GameState *gs, struct Pos p, enum PieceColor op_color)
{
	struct Piece *target_piece = lookup_piece(gs->board, p);
	for (int r = 0; r < NUM_RANKS; r++)
	{
		for (int f = 0; f < NUM_FILES; f++)
		{
			struct Piece *op = gs->board[r][f];//examines a POSITION
			//if the POSITION = NULL or the PIECE is the WRONG color move on to the next POSITION
			if(!op || op->color != op_color) continue;

			//an ANTEATER can't attack the KING

			int dr = p.r - r;
			int df = p.f - f;
			int adr = abs(dr);
			int adf = abs(df);

			switch(op->type)
			{
				case PAWN:
					if(op_color == WHITE && dr == 1 && adf == 1) return 1;
					if(op_color == BLACK && dr == -1 && adf == 1) return 1;
					break;
				case KNIGHT:
					if((adr == 2 && adf == 1) || (adr == 1 && adf == 2)) return 1;
					break;
				case KING:
					if(adr <= 1 && adf <= 1) return 1;
					break;
				case ANTEATER:
					if(target_piece && target_piece->type == PAWN && adr <= 1 && adf <= 1) return 1;
                                        break;
				case BISHOP:
				case ROOK:
				case QUEEN:
					if(op->type == BISHOP && adr != adf) break;
					if (op->type == ROOK && adr != 0 && adf != 0) break;
					if (op->type == QUEEN && adr != adf && adr != 0 && adf != 0) break;

					int step_r = (dr == 0) ? 0:(dr > 0 ? 1 : -1);
					int step_f = (df == 0) ? 0:(df > 0 ? 1 : -1);
					int cr = r + step_r;
					int cf = f + step_f;
					int blocked = 0;

					while (cr != p.r || cf != p.f)
					{
						if (gs->board[cr][cf])
						{
							blocked = 1;
							break;
						}
						cr += step_r;
						cf += step_f;
					}//end of WHILE loop

					if (!blocked) return 1;
					break;
			}//end of SWITCH statements
		}//end of FOR (file) loop
	}//end of FOR (rank) loop
	return 0;
}//end of is_under_attack FUNCTION

int king_in_check(struct GameState *gs, enum PieceColor color)
{
	struct Pos pos_of_king = make_pos(-1,-1); //starting position to look for the KING

	//finding the position of the KING
	for (int r = 0; r <NUM_RANKS; r++)
	{
		for (int f = 0; f < NUM_FILES; f++)
		{
			struct Piece *p = gs->board[r][f];//assign each POSITION a PIECE/NULL
			if (p && p->type == KING && p->color == color)
			//if the POSITION has a PIECE that meets the paramters of the KING we are looking for
			{
				pos_of_king = make_pos(r,f);//save the POSITION
				break;
			}
		}//end of FOR (file) loop
	}//end of FOR (rank) loop
	if(pos_of_king.r == -1) return 0; //BUG: the King is Missing
	return is_under_attack(gs, pos_of_king, OPPONENT(color));
	//calls this function to see if any PIECES are attacking the FOUND KING
	//Return 1: in CHECK
	//Return 0: not in CHECK
}//end of king_in_check FUNCTION

int possible_moves(struct GameState *gs, struct Move m)
{
	struct Piece *p = lookup_piece(gs->board, m.from);
	//LoopUp the piece and it's starting position
	if(!p) return 0; //if there is NO Pieces at the starting POSITION, a move is NOT POSSIBLE
	if(m.from.r == m.to.r && m.from.f == m.to.f) return 0;
	//NOT Possible to move to the SAME Square

	struct Piece *target = lookup_piece(gs->board, m.to);
	//LoopUp the piece at the desired position
	if (target && target->color == p->color) return 0;
	//if theres a PIECE of the same COLOR, move NOT POSSIBLE
	
	int dr = m.to.r - m.from.r;
	int df = m.to.f - m.from.f;
	int adr = abs(dr);
	int adf = abs(df);

	switch (p->type)
	{
		case PAWN:
			{
				int dir = (p->color == WHITE) ? 1: -1;
				int start_rank = (p->color == WHITE) ? 1: 6;

				if (df == 0)
				{
					if(dr == dir && !target) return 1;
					if(dr == 2 * dir && m.from.r == start_rank && !target)
					{
						struct Pos mid = make_pos(m.from.r + dir, m.from.f);
						if (!lookup_piece(gs->board, mid)) return 1;
					}
				}
				if (adf == 1 && dr == dir)
				{
					if(target) return 1;
					if (m.to.r == gs->en_passant_target.r && m.to.f == gs->en_passant_target.f) return 1;
				}
				return 0;
			}//end of PAWN case
		case ANTEATER:
			if(adr > 1 || adf > 1) return 0;

			if(target && target->type != PAWN) return 0;
			return 1;
		case KNIGHT:
			if((adr == 2 && adf == 1) || (adr == 1 && adf == 2))
			{
				return 1;
			}
			else
			{
				return 0;
			}
		case KING:
			if (adr <= 1 && adf <=1) return 1;

			//Castling Check
			if (adr == 0 && adf >=2)
			{
				if(p->color == WHITE && m.from.r == 0 && m.from.f == 5)
				{
					if(m.to.f == 7 && gs->white_castle_k && !gs->board[0][6] && !gs->board[0][7] && !gs->board[0][8]) return 1;
					if(m.to.f == 3 && gs->white_castle_k && !gs->board[0][4] &&  !gs->board[0][3] && !gs->board[0][2] && !gs->board[0][1]) return 1;
				} else if(p->color == BLACK && m.from.r == 7 && m.from.f == 5)
                                       	{
                                               	if(m.to.f == 7 && gs->black_castle_k && !gs->board[7][6] && !gs->board[7][7] && !gs->board[7][8]) return 1;
                                                if(m.to.f == 3 && gs->black_castle_k && !gs->board[7][4] &&  !gs->board[7][3] && !gs->board[7][2] && !gs->board[7][1]) return 1;
					}
			}
			return 0;
		case BISHOP: if(adr != adf) return 0;  break;
		case ROOK: if(adr != 0 && adf !=0) return 0;  break;
		case QUEEN: if(adr != adf && adr != 0 && adf !=0) return 0; break;
	}//end of CASE statements

	//Checking the Path to see if it's clear
	int path_r = (dr == 0) ? 0: (dr > 0 ? 1: -1);
	int path_f = (df == 0) ? 0: (dr > 0 ? 1: -1);
	int r = m.from.r + path_r;
	int f = m.from.f + path_f;
	while (r != m.to.r || f!= m.to.f)
	{
		if(gs->board[r][f]) return 0;
		r += path_r;
		f += path_f;
	}
	return 1;
}//end of possible_moves FUNCTION

int is_legal_move(struct GameState *gs, struct Move m)
{
	if(!pos_valid(m.from) || !pos_valid(m.to)) return 0;
	struct Piece *p = lookup_piece(gs->board, m.from);
	if(!p || p->color != gs->current_turn) return 0;
	if (!possible_moves(gs,m)) return 0;

	//check CASTLING
	if(p->type ==KING && abs(m.from.f - m.to.f) >= 2)
	{
		int move = (m.to.f > m.from.f) ? 1 : -1;
		struct Pos pass_sq = make_pos(m.from.r, m.from.f + move);
		if (is_under_attack(gs, pass_sq, OPPONENT(p->color))) return 0;
		if(king_in_check(gs, p->color)) return 0;
	}

	struct GameState copy = copy_of_board(gs);
	apply_move(&copy, m);
	int in_check = king_in_check(&copy, p->color);
	clear_board_copy(&copy);

	return !in_check;
}//end of is_legal_move FUNCTION

static int any_moves_left(struct GameState *gs, enum PieceColor color)
{
	for(int fr = 0; fr < NUM_RANKS; fr++)
	{
		for (int ff = 0; ff < NUM_FILES; ff++)
		{
			struct Piece *p = gs->board[fr][ff];
			if(p && p->color == color)
			{
				for(int tr = 0; tr < NUM_RANKS; tr++)
				{
					for (int tf = 0; tf < NUM_FILES; tf++)
					{
						struct Move m;
						m.from = make_pos(fr,ff);
						m.to = make_pos(tr,tf);
						if(is_legal_move(gs,m)) return 1;
					}//end of FOR loop
				}//end of FOR loop
			}//end of if statement
		}//end of FOR LOOP
	}//end of FOR Loop
	return 0;
}//end of any_moves_left FUNCTION

int king_in_checkmate(struct GameState *gs, enum PieceColor color)
{
	if(!king_in_check(gs, color)) return 0;
	return !any_moves_left(gs,color);
}//end of king_in_checkmate FUNCTION

int king_in_stalemate(struct GameState *gs, enum PieceColor color)
{
	if(king_in_check(gs, color)) return 0;
	return !any_moves_left(gs, color);
}//end of king_in_stalemate FUNCTION


