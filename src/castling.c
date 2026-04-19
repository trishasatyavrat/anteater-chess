#include "rules.h"
#include "board.h"
void castling_function(struct GameState *gs, struct Move *m)
{
	struct Piece *king = lookup_piece(gs->board, m->from);
	if(!king) return;

	m->was_castling = 1;
	int rank = m->from.r;

	//short-side of the board
	if(m->to.f == 7)
	{
		m->rook_start = make_pos(rank, 9);
		m->rook_end = make_pos(rank, 6)
	}
	else if(m->to.f == 3)//long-side of the board
	{
		m->rook_start = make_pos(rank, 0);
		m->rook_end = make_pos(rank, 4);
	}

	//actually moving the PIECES
	struct Piece *rook = lookup_piece(gs->board, m->rook_from);
	if(rook)
	{
		assign_piece(gs->board, rock, m->rock_to); //Moves the ROOK to new location
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


