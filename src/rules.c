#include "rules.h"
#include "board.h"
#include <stdlib.h>
#include <math.h>

//is there a step being made
static int is_step(int from, int to)
{
	if(from == to) return 0; //a PIECE cannot make a MOVE to the SAME position
	return (to > from) ? 1: -1;
	//a 1 -> RIGHT or UP
	//a -1 -> LEFT or DOWN
}//end of is_step FUNCTION

static int is_path_clear(struct GameState *gs, struct Pos from, struct Pos to)
{
 	int dir_r = is_step(from.r, to.r); //direction of the RANK
	int dir_f = is_step(from.f, to.f); //direction of the FILE

	//CHECK if the position closest to the PIECE is occupied
	int r = from.r + dir_r;
	int f = from.f + dir_f;

	//Continue checking until the desired position is checked
	while (r != to.r || f != to.f)
	{
		if(gs->board[r][f])
		//if there's a PIECE in that position, the path is BLOCKED
		{
			return 0; //not CLEAR
		}
		//increment the position if needed
		r += dir_r;
		f += dir_f;
	}

	return 1; //if the PATH is cleared
}//end of is_path_clear FUNCTION

//create a copy of the current board (checking moves without disturbing the actual game)
struct GameState copy_of_board(struct GameState *gs)
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
void clear_board_copy(struct GameState *copy)
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
	}//end of FOR (rank) loop
}//end of clear_board_copy FUNCTION

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
	//LookUp the piece and its starting position
	if(!p) return 0; //if there is NO PIECE at the starting POSITION, a move is NOT POSSIBLE
	if(m.from.r == m.to.r && m.from.f == m.to.f) return 0;
	//NOT Possible to move to the SAME Square

	struct Piece *target = lookup_piece(gs->board, m.to);
	//LoopUp the piece at the desired position
	if (target && target->color == p->color) return 0;
	//if there's a PIECE of the same COLOR, move NOT POSSIBLE
	
	int dis_moved_r = m.to.r - m.from.r; //distance a PIECE moved (ROW)
	int dis_moved_f = m.to.f - m.from.f; //distance a PIECE moved (FILE)
	int adr = abs(dis_moved_r); //absolute value of distance moved (ROW) ignores direction (UP or DOWN)
	int adf = abs(dis_moved_f); //absolute value of distance moved (FILE) ignores direction

	switch (p->type)
	{
		case PAWN:
			{
				int dir = (p->color == WHITE) ? 1: -1; //direction of the PIECE (UP -> WHITE, DOWN -> BLACK)
				int start_rank = (p->color == WHITE) ? 1: 6; //the starting POSITION of a PAWN/ANT
				//it should be WHITE RANK -> 1, BLACK RANK -> 6 on the board

				if (dis_moved_f == 0) //the PAWN can only change RANK except when capturing
				{
					if(dis_moved_r == dir && !target) return 1; //Move FORWARD once
					if(dis_moved_r == 2 * dir && m.from.r == start_rank && !target) //Move FORWARD twice if PAWN is at spawn position
					{
						struct Pos mid = make_pos(m.from.r + dir, m.from.f); //the middle position in the PAWN's path
						if (!lookup_piece(gs->board, mid)) return 1; //if there's a PIECE the PAWN can not move there
					}
				}
				if (adf == 1 && dis_moved_r == dir) //if the PAWN is capturing
				{
					if(target) return 1; //Move is only VALID if there an opposing PIECE
					if (m.to.r == gs->en_passant_target.r && m.to.f == gs->en_passant_target.f) return 1;
					//EN PASSANT
				}
				return 0; //return 0 if move NOT possible
			}//end of PAWN case
		case ANTEATER:
			if(adr > 1 || adf > 1) return 0; //Moves like a KING
			if(target && target->type != PAWN) return 0; //can ONLY capture ANTS (PAWNS)
			return 1;
		case KNIGHT: //L-shape move
			if((adr == 2 && adf == 1) || (adr == 1 && adf == 2))
			{
				return 1;
			}
			else
			{
				return 0; //invalid MOVE if it's not an L-shape
			} //A KNIGHT is allowed to JUMP over PIECES
		case KING:
			if (adr <= 1 && adf <=1) return 1; //Moves ONE space any direction

			//Castling Check
			if (adr == 0 && adf >=2) //if the KING wants to castle (moving more than TWO spaces)
			{
				if(p->color == WHITE && m.from.r == 0 && m.from.f == 5) //The KING has to be UNMOVED from spawn point
				{
					//all positions from the KING to the ROOK have to be EMPTY
					//the flag for CASTLING has to be active
					if(m.to.f == 7 && gs->white_castle_k && !gs->board[0][6] && !gs->board[0][7] && !gs->board[0][8]) return 1;
					if(m.to.f == 3 && gs->white_castle_q && !gs->board[0][4] && !gs->board[0][3] && !gs->board[0][2] && !gs->board[0][1]) return 1;
				} else if(p->color == BLACK && m.from.r == 7 && m.from.f == 5)
                {
                	if(m.to.f == 7 && gs->black_castle_k && !gs->board[7][6] && !gs->board[7][7] && !gs->board[7][8]) return 1;
                    if(m.to.f == 3 && gs->black_castle_q && !gs->board[7][4] &&  !gs->board[7][3] && !gs->board[7][2] && !gs->board[7][1]) return 1;
				}
			}
			return 0;
		case BISHOP: //moves diagonally evenly
			if(adr == adf) 
			{
				if(is_path_clear(gs, m.from, m.to)) //checks if the path is NOT Blocked
				{
					return 1;
				}
			}
			return 0;
		case ROOK: //moves in a straight line in all direction (NOT DIAGONALLY)
			if(dis_moved_r == 0 || dis_moved_f == 0)
			{
				if(is_path_clear(gs, m.from, m.to)) //make sure PATH is CLEAR
				{
					return 1;
				}
			}
			return 0;
		case QUEEN: //straight and diagonally
			if(adr == adf || dis_moved_r == 0 || dis_moved_f ==0) 
			{
				if(is_path_clear(gs, m.from, m.to))
				{
					return 1;
				}
			}
			return 0;
		default: printf("default case."); 
			return 0;
	}//end of CASE statements
}//end of possible_moves FUNCTION

int is_under_attack(struct GameState *gs, struct Pos p, enum PieceColor op_color)
{
	//LOOP the ENTIRE Board
	for (int r = 0; r < NUM_RANKS; r++)
	{
		for (int f = 0; f < NUM_FILES; f++)
		{
			struct Piece *op = gs->board[r][f];//examines a POSITION
			//if the POSITION = NULL or the PIECE is the WRONG color move on to the next POSITION
			if(op && op->color == op_color)//if the position is NOT EMPTY and the color is the OPPOSING team
			{
				struct Move m = {.from = make_pos(r,f), .to = p};
				if(possible_moves(gs, m)) return 1; //if the move is possible then the POSITION is under attack
			}
		}//end of FOR (file) loop
	}//end of FOR (rank) loop
	return 0; //position is SAFE
}//end of is_under_attack FUNCTION

int is_legal_move(struct GameState *gs, struct Move m)
{
	if(!pos_valid(m.from) || !pos_valid(m.to)) return 0; //are the POSITIONS actually valid (in bounds)?
	struct Piece *p = lookup_piece(gs->board, m.from);
	if(!p || p->color != gs->current_turn) return 0; 
	//there has to be a PIECE at the position and you cant move the OPPONENTS piece
	if (!possible_moves(gs,m)) return 0;
	//if the move doesn't follow BASIC rules, the MOVE is not legal

	//check CASTLING
	if(p->type ==KING && abs(m.from.f - m.to.f) >= 2)
	{
		if(king_in_check(gs, p->color)) return 0; //is the KING in check?
		
		int step = (m.to.f > m.from.f)? 1: -1; //is the King going Short or LONG side?
		//if the KING is in CHECK, castling is NOT allowed
		struct Pos jump_sq = make_pos(m.from.r, m.from.f+step);
		if(is_under_attack(gs, jump_sq, OPPONENT(p->color))) return 0;
	}

	//Simulation 
	struct GameState copy = copy_of_board(gs); //create a copy of the current board
	apply_move(&copy, m); //apply the move
	int in_check = king_in_check(&copy, p->color); //is the KING in check?
	clear_board_copy(&copy); //free memory

	return !in_check;
	//return 1: The King is SAFE and the move is LEGAL
}//end of is_legal_move FUNCTION

static int any_moves_left(struct GameState *gs, enum PieceColor color)
{
	//For the calling order in MAIN.c
	//Variable for the ACTUALL current turn
	enum PieceColor actual_turn = gs->current_turn;
	gs->current_turn = color;
	//Looks for POSITIONS throughout the entire board with the current player's piece on it
	for(int fr = 0; fr < NUM_RANKS; fr++)
	{
		for (int ff = 0; ff < NUM_FILES; ff++)
		{
			struct Piece *p = gs->board[fr][ff];
			if(p && p->color == color) //the POSITION can't be empty AND belongs to the current player
			{
				//can these pieces go to ANY OTHER POSITION legally?
				for(int tr = 0; tr < NUM_RANKS; tr++)
				{
					for (int tf = 0; tf < NUM_FILES; tf++)
					{
						struct Move m; //variable to test possibilities
						m.from = make_pos(fr,ff);
						m.to = make_pos(tr,tf);
						if(is_legal_move(gs,m))
						{
							gs->current_turn = actual_turn; 
							return 1;
						}
					}//end of FOR loop
				}//end of FOR loop
			}//end of if statement
		}//end of FOR LOOP
	}//end of FOR Loop
	gs->current_turn = actual_turn;
	return 0; //Return 0: there are no MOVES left
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

//TEMP LegalMoves_Function for AI Module:
void legalmoves_function(struct GameState *gs, struct Pos position, struct Move *out, int *count)
{
    *count = 0;

    struct Piece *p = lookup_piece(gs->board, position);
    if (!p || p->color != gs->current_turn) {
        return;
    }

    for (int tr = 0; tr < NUM_RANKS; tr++)
    {
        for (int tf = 0; tf < NUM_FILES; tf++)
        {
            struct Move m;
            m.from = position;
            m.to = make_pos(tr, tf);

            if (is_legal_move(gs, m))
            {
                out[*count] = m;
                (*count)++;
            }
        }
    }
}



