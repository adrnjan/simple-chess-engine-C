#ifndef APSC143__BOARD_H
#define APSC143__BOARD_H

#include <stdbool.h>
#define CASTLE_NONE 0
#define CASTLE_KINGSIDE 1
#define CASTLE_QUEENSIDE 2


struct chess_board
{
    int next_move_player;
    char squares[8][8];

    int castle_wk;
    int castle_wq;
    int castle_bk;
    int castle_bq;

    int ep_file;
    int ep_rank;
};


struct chess_move
{
    char piece_type;
    int start_rank;
    int start_file;
    int end_rank;
    int end_file;
    int capture_flag;
    int castling_type;
    char promotion_piece;
    int disambig_rank;
    int disambig_file;
};

void board_initialize(struct chess_board *board);
void board_complete_move(const struct chess_board *board, struct chess_move *move);
void board_apply_move(struct chess_board *board, const struct chess_move *move);
void board_summarize(const struct chess_board *board);

#endif