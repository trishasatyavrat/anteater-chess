#include "ai.h"
#include "rules.h"

int ai_depth = 1;

int piece_value(enum PieceType type){
    if (type == PAWN){
        return 1;
    }
    if (type == KNIGHT){
        return 3;
    }
    if (type == BISHOP){
        return 3;
    }
    if (type == ROOK){
        return 5;
    }
    if (type == QUEEN){
        return 9;
    }
    if (type == ANTEATER){
        return 2;
    }
    if (type == KING){
        return 0;
    }
    return 0;
}

void difficultyselect_function(int level){
    if (level == 1){
        ai_depth = 1;
    }
    else if (level == 2){
        ai_depth = 3;
    }
    else{
        ai_depth = 5;
    }
}

int get_ai_depth(void){
    return ai_depth;
}

struct Move bestmove_function(struct GameState *gs, enum PieceColor color, int depth){
    struct Move moves[512]; //stores moves
    int count = 0;


    //Board Space Boundaries
    struct Move invalid_move;
    invalid_move.from.r=0;
    invalid_move.from.f=0;
    invalid_move.to.r=9;
    invalid_move.to.f=11;


    gs->current_turn = color;
    ai_legalmoves_function(gs, color, moves, &count); //&count = Pass Address of Count



    //No Possible Moves
    if (count==0){
        return invalid_move;
    }

    struct Move best_move = moves[0];
    
    //Placeholder score
    int best_score = -10000;


    int i;
    for (i=0; i < count; i++){

        struct GameState temp_gs = copy_of_board(gs);
        apply_move(&temp_gs, moves[i]);
        
        enum PieceColor next_color;
        if (color == WHITE){
            next_color = BLACK;
        }
        else{
            next_color = WHITE;
        }
        
        int temp_score = minimax(&temp_gs, next_color, color, depth - 1, -10000, 10000); //Set bounds for alpha and beta

      

       if (temp_score > best_score){
        
            best_score = temp_score;
            best_move = moves[i];

       }

       clear_board_copy(&temp_gs);


    }

    
    return best_move;



}

 void ai_legalmoves_function(struct GameState *gs, enum PieceColor color, struct Move *out, int *count){
        *count = 0;
        int r;
        int f;

        int i;

        for (r = 0; r < 8; r++){
            for (f = 0; f < 10; f++){
                if (gs->board[r][f] == NULL){
                    continue;
                }
                if (gs->board[r][f]->color != color){
                    continue;
                }
                struct Pos position;
                position.r = r;
                position.f = f;
                
                struct Move temp_moves[512];
                int temp_count = 0;
                
                legalmoves_function(gs, position, temp_moves, &temp_count);

                for (i = 0; i < temp_count; i++){
                    out[*count] = temp_moves[i];
                    *count = *count + 1;


                }


        }


        }
    }

    int scan_board(struct GameState *gs, enum PieceColor color){
        int r;    
        int f;
        int score = 0;


            for (r = 0; r < 8; r++){
                for (f = 0; f < 10; f++){
                if (gs->board[r][f] == NULL){
                    continue;
                }

                int value = piece_value(gs->board[r][f]->type);

                if (gs->board[r][f]->color != color){
                    score = score - value;
                }
                if (gs->board[r][f]->color == color){
                    score = score + value;
                }
          
        }
        
    }
    return score;
}


int minimax(struct GameState *gs, enum PieceColor turn_color, enum PieceColor ai_color, int depth, int alpha, int beta){
    if (depth <= 0){
    return scan_board(gs, ai_color);
    }   
    
    struct Move moves[512];
    int count = 0;

    gs->current_turn = turn_color;
    ai_legalmoves_function(gs, turn_color, moves, &count);

    if (count==0){

        return scan_board(gs, ai_color);
    }

    enum PieceColor next_color;
        if (turn_color == WHITE){
            next_color = BLACK;
        }
        else{
            next_color = WHITE;
        }
    int best_score;

    int i;


    if (turn_color == ai_color){
        best_score = -10000;

        for (i = 0; i < count; i++){
            struct GameState temp_gs = copy_of_board(gs);
            apply_move(&temp_gs, moves[i]);

            int temp_score = minimax(&temp_gs, next_color, ai_color, depth - 1, alpha, beta);

            if (temp_score > best_score){
                best_score = temp_score;
            }

            if (best_score > alpha){
                alpha = best_score;
            }

            clear_board_copy(&temp_gs);

            if (alpha >= beta){
                break;
            }

        }
    }
    else{
        best_score = 10000;

        for (i = 0; i < count; i++){
            struct GameState temp_gs = copy_of_board(gs);
            apply_move(&temp_gs, moves[i]);

            int temp_score = minimax(&temp_gs, next_color, ai_color, depth - 1, alpha, beta);

            if (temp_score < best_score){
                best_score = temp_score;
            }

            if (best_score < beta){
                beta = best_score;
            }

            clear_board_copy(&temp_gs);

            if (alpha >= beta){
                break;
            }

        }
    }

    return best_score;
}
