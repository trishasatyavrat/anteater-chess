#include "rules.h"
#include "board.h"
void castling_function(struct GameState *gs, struct Move *m)
{
	//find the KING
	struct Piece *king = lookup_piece(gs->board, m->from);
	if(!king) return; //if the position does not contain a KING, then CASTLING can't occur

	//for undo_move (lets the function know there were MULTIPLE PIECES moved
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
		assign_piece(gs->board, rock, m->rook_to); //Moves the ROOK to new location
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


