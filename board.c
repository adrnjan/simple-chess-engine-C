#include "board.h"
#include <stdio.h>
#include "panic.h"

// Helper: Absolute value
int my_abs(int a) { return (a < 0) ? -a : a; }

// Helper: Get human-readable name for panic messages
const char* get_piece_name(char type) {
    char t = type;
    if (t >= 'a' && t <= 'z') t = t - 'a' + 'A'; 
    if (t == 'P') return "pawn";
    if (t == 'N') return "knight";
    if (t == 'B') return "bishop";
    if (t == 'R') return "rook";
    if (t == 'Q') return "queen";
    if (t == 'K') return "king";
    return "piece";
}

int is_white_piece(char p) {
    return (p == 'P' || p == 'N' || p == 'B' || p == 'R' || p == 'Q' || p == 'K');
}

int is_black_piece(char p) {
    return (p == 'p' || p == 'n' || p == 'b' || p == 'r' || p == 'q' || p == 'k');
}

int is_same_color(char p1, int player_is_white) {
    if (p1 == '.') return 0;
    if (player_is_white) return is_white_piece(p1);
    else return is_black_piece(p1);
}

char to_upper(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a' + 'A';
    return c;
}


int is_path_clear(const struct chess_board *board, int r1, int c1, int r2, int c2) {
    struct chess_board b = *board;
    int dr = (r2 > r1) ? 1 : ((r2 < r1) ? -1 : 0);
    int dc = (c2 > c1) ? 1 : ((c2 < c1) ? -1 : 0);
    int r = r1 + dr;
    int c = c1 + dc;
    while (r != r2 || c != c2) {
        if (b.squares[r][c] != '.') return 0; 
        r += dr;
        c += dc;
    }
    return 1;
}


int is_geometry_valid(const struct chess_board *board, int r1, int c1, int r2, int c2) {
    struct chess_board b = *board;
    char p = b.squares[r1][c1];
    char target = b.squares[r2][c2];
    char type = to_upper(p);
    int is_white = is_white_piece(p);
    
    // Cannot capture own piece
    if (target != '.' && is_same_color(target, is_white)) return 0;

    int dr = r2 - r1;
    int dc = c2 - c1;
    int abs_dr = my_abs(dr);
    int abs_dc = my_abs(dc);

    if (type == 'P') { 
        int forward = (is_white) ? 1 : -1;
        int start_row = (is_white) ? 1 : 6;
        
        // 1. Single step forward (non-capture)
        if (dc == 0 && dr == forward) return (target == '.');
        
        // 2. Double step forward (start rank only, path must be clear)
        if (dc == 0 && dr == 2 * forward) {
            if (r1 != start_row) return 0;
            if (target != '.') return 0;
            if (b.squares[r1 + forward][c1] != '.') return 0;
            return 1;
        }
        
        // 3. Capture (Diagonal)
        if (abs_dc == 1 && dr == forward) {
            if (target != '.') return 1; // Normal capture
            if (b.ep_file == c2 && b.ep_rank == r2) return 1; // En Passant
        }
        return 0;
    }
    
    if (type == 'N') return (abs_dr == 2 && abs_dc == 1) || (abs_dr == 1 && abs_dc == 2);
    if (type == 'B') {
        if (abs_dr != abs_dc) return 0;
        return is_path_clear(board, r1, c1, r2, c2);
    }
    if (type == 'R') {
        if (dr != 0 && dc != 0) return 0;
        return is_path_clear(board, r1, c1, r2, c2);
    }
    if (type == 'Q') {
        int diagonal = (abs_dr == abs_dc);
        int straight = (dr == 0 || dc == 0);
        if (!diagonal && !straight) return 0;
        return is_path_clear(board, r1, c1, r2, c2);
    }
    if (type == 'K') return (abs_dr <= 1 && abs_dc <= 1);
    
    return 0;
}

void find_king(const struct chess_board *board, int is_white, int *kr, int *kc) {
    struct chess_board b = *board; 
    char k_char = (is_white) ? 'K' : 'k';
    for(int r=0; r<8; r++){
        for(int c=0; c<8; c++){
            if (b.squares[r][c] == k_char) {
                *kr = r; *kc = c; return;
            }
        }
    }
}


int is_attacked(const struct chess_board *board, int r, int c, int by_white) {
    struct chess_board b = *board; 
    for(int y=0; y<8; y++){
        for(int x=0; x<8; x++){
            char p = b.squares[y][x];
            if (p == '.') continue;
            if (by_white && !is_white_piece(p)) continue;
            if (!by_white && !is_black_piece(p)) continue;
            
            char type = to_upper(p);

            if (type == 'P') {
                int forward = (by_white) ? 1 : -1;
                if ((r == y + forward) && (my_abs(c - x) == 1)) return 1;
            } else {

                if (is_geometry_valid(board, y, x, r, c)) return 1;
            }
        }
    }
    return 0;
}

int is_in_check(const struct chess_board *board, int white_player) {
    int kr, kc;
    find_king(board, white_player, &kr, &kc);
    return is_attacked(board, kr, kc, !white_player);
}

void board_initialize(struct chess_board *board)
{
    struct chess_board b;

    b.next_move_player = 1; // White starts
    b.castle_wk = 1; b.castle_wq = 1;
    b.castle_bk = 1; b.castle_bq = 1;
    b.ep_file = -1; b.ep_rank = -1;

    for(int r=0; r<8; r++) {
        for(int c=0; c<8; c++) {
            b.squares[r][c] = '.';
        }
    }

    char white_back[] = {'R','N','B','Q','K','B','N','R'};
    char black_back[] = {'r','n','b','q','k','b','n','r'};

    for(int c=0; c<8; c++) {
        b.squares[0][c] = white_back[c];
        b.squares[1][c] = 'P';
        b.squares[6][c] = 'p';
        b.squares[7][c] = black_back[c];
    }
    *board = b;
}

void board_complete_move(const struct chess_board *board, struct chess_move *move)
{
    struct chess_board b = *board; 
    struct chess_move m = *move; 
    const char* p_name = get_piece_name(m.piece_type);


    if (m.castling_type != CASTLE_NONE) {
        int kr, kc;
        find_king(board, b.next_move_player, &kr, &kc);
        m.start_rank = kr; m.start_file = kc;
        m.end_rank = kr;
        m.end_file = (m.castling_type == CASTLE_KINGSIDE) ? 6 : 2; // g-file or c-file
        *move = m; 
        return;
    }


    int geom_count = 0;
    int geom_r = -1, geom_c = -1;

    char target_type = m.piece_type; 

    for(int r=0; r<8; r++) {
        for(int c=0; c<8; c++) {
            char p = b.squares[r][c];
            if (p == '.') continue;
            if (!is_same_color(p, b.next_move_player)) continue;
            if (to_upper(p) != target_type) continue;
            

            if (m.disambig_rank != -1 && m.disambig_rank != r) continue;
            if (m.disambig_file != -1 && m.disambig_file != c) continue;



            if (is_geometry_valid(board, r, c, m.end_rank, m.end_file)) {
                geom_count++;
                geom_r = r;
                geom_c = c;
            }
        }
    }

    if (geom_count != 1) {
        panicf("move completion error: %s %s to %c%c\n", 
               (b.next_move_player ? "white" : "black"),
               p_name, 
               m.end_file + 'a', m.end_rank + '1');
    }


    struct chess_board temp = b;
    temp.squares[m.end_rank][m.end_file] = temp.squares[geom_r][geom_c];
    temp.squares[geom_r][geom_c] = '.';
    
    // Handle En Passant capture on temp board
    if (target_type == 'P' && geom_c != m.end_file && temp.squares[m.end_rank][m.end_file] == '.') {
         int capture_rank = m.end_rank + ((b.next_move_player) ? -1 : 1);
         temp.squares[capture_rank][m.end_file] = '.';
    }

    if (is_in_check(&temp, b.next_move_player)) {
         panicf("illegal move: %s %s from %c%c to %c%c\n", 
                (b.next_move_player ? "white" : "black"),
                p_name, 
                geom_c + 'a', geom_r + '1', m.end_file + 'a', m.end_rank + '1');
    }

    m.start_rank = geom_r;
    m.start_file = geom_c;
    *move = m; 
}

void board_apply_move(struct chess_board *board, const struct chess_move *move)
{
    struct chess_board b = *board; 
    struct chess_move m = *move;
    struct chess_board next = b;   
    next.ep_file = -1; next.ep_rank = -1; // Reset EP by default

    int sr = m.start_rank, sf = m.start_file;
    int er = m.end_rank, ef = m.end_file;
    const char* p_name = get_piece_name(m.piece_type);

    if (m.castling_type != CASTLE_NONE) {
        int row = (b.next_move_player) ? 0 : 7;
        
        // Check availability flags
        int available = 0;
        if (b.next_move_player) { 
            available = (m.castling_type == CASTLE_KINGSIDE) ? b.castle_wk : b.castle_wq;
        } else { 
            available = (m.castling_type == CASTLE_KINGSIDE) ? b.castle_bk : b.castle_bq;
        }
        if (!available) 
             panicf("illegal move: %s king from %c%c to %c%c\n", (b.next_move_player?"white":"black"), sf+'a', sr+'1', ef+'a', er+'1');

        // Check path is clear
        int rook_file = (m.castling_type == CASTLE_KINGSIDE) ? 7 : 0;
        int step = (m.castling_type == CASTLE_KINGSIDE) ? 1 : -1;
        int c = 4 + step;
        while(c != rook_file) {
            if (b.squares[row][c] != '.') 
                 panicf("illegal move: %s king from %c%c to %c%c\n", (b.next_move_player?"white":"black"), sf+'a', sr+'1', ef+'a', er+'1');
            c += step;
        }

        // Check for attacks (Start, Path, Destination)
        int opponent_is_white = !b.next_move_player;
        if (is_in_check(board, b.next_move_player) ||
            is_attacked(board, row, 4 + step, opponent_is_white) ||
            is_attacked(board, row, ef, opponent_is_white)) {
             panicf("illegal move: %s king from %c%c to %c%c\n", (b.next_move_player?"white":"black"), sf+'a', sr+'1', ef+'a', er+'1');
        }

        // Apply castling
        next.squares[row][4] = '.'; 
        next.squares[row][rook_file] = '.'; 
        next.squares[row][ef] = (b.next_move_player) ? 'K' : 'k';
        next.squares[row][ef - step] = (b.next_move_player) ? 'R' : 'r';


        if (b.next_move_player) { next.castle_wk = 0; next.castle_wq = 0; }
        else { next.castle_bk = 0; next.castle_bq = 0; }

    } else {

        char p = b.squares[sr][sf];
        

        if (!is_geometry_valid(board, sr, sf, er, ef)) {
             panicf("illegal move: %s %s from %c%c to %c%c\n", (b.next_move_player?"white":"black"), p_name, sf+'a', sr+'1', ef+'a', er+'1');
        }


        int actual_capture = 0;
        if (b.squares[er][ef] != '.') {
            actual_capture = 1; 
        } else if (m.piece_type == 'P' && sf != ef) {
            // Diagonal pawn move to empty square is En Passant, which is a capture
            actual_capture = 1;
        }

        if (m.capture_flag && !actual_capture) {
            panicf("illegal move: %s %s from %c%c to %c%c\n", (b.next_move_player?"white":"black"), p_name, sf+'a', sr+'1', ef+'a', er+'1');
        }
        if (!m.capture_flag && actual_capture) {
            panicf("illegal move: %s %s from %c%c to %c%c\n", (b.next_move_player?"white":"black"), p_name, sf+'a', sr+'1', ef+'a', er+'1');
        }

        // Handle En Passant capture removal
        if (m.piece_type == 'P' && sf != ef && b.squares[er][ef] == '.') {
            int cap_r = er + ((b.next_move_player) ? -1 : 1);
            next.squares[cap_r][ef] = '.';
        }

        next.squares[sr][sf] = '.';
        next.squares[er][ef] = p;

        // Handle Promotion
        if (m.piece_type == 'P') {
            int last_rank = (b.next_move_player) ? 7 : 0;
            if (er == last_rank) {
                if (m.promotion_piece == '.' || m.promotion_piece == 0) {
                     panicf("illegal move: %s %s from %c%c to %c%c\n", (b.next_move_player?"white":"black"), p_name, sf+'a', sr+'1', ef+'a', er+'1');
                }
                char promo = m.promotion_piece;
                if (!b.next_move_player && promo >= 'A' && promo <= 'Z') promo += ('a' - 'A');
                next.squares[er][ef] = promo;
            }
        }

        // Set En Passant target if double move
        if (m.piece_type == 'P' && my_abs(er - sr) == 2) {
            next.ep_file = sf;
            next.ep_rank = sr + ((b.next_move_player) ? 1 : -1);
        }

        if (p == 'K') { next.castle_wk = 0; next.castle_wq = 0; }
        if (p == 'k') { next.castle_bk = 0; next.castle_bq = 0; }
   
        if ((sr==0 && sf==0) || (er==0 && ef==0)) next.castle_wq = 0;
        if ((sr==0 && sf==7) || (er==0 && ef==7)) next.castle_wk = 0;
        if ((sr==7 && sf==0) || (er==7 && ef==0)) next.castle_bq = 0;
        if ((sr==7 && sf==7) || (er==7 && ef==7)) next.castle_bk = 0;
    }

    // final check
    if (is_in_check(&next, b.next_move_player)) {
         panicf("illegal move: %s %s from %c%c to %c%c\n", (b.next_move_player?"white":"black"), p_name, sf+'a', sr+'1', ef+'a', er+'1');
    }

    next.next_move_player = !b.next_move_player;
    *board = next;
}

void board_summarize(const struct chess_board *board)
{
    struct chess_board b = *board;
    int current = b.next_move_player;
    int has_legal = 0;

    // check if any piece can make any move
    for(int r=0; r<8 && !has_legal; r++){
        for(int c=0; c<8 && !has_legal; c++){
            char p = b.squares[r][c];
            if (p == '.') continue;
            if (!is_same_color(p, current)) continue;

            for(int rr=0; rr<8 && !has_legal; rr++){
                for(int cc=0; cc<8 && !has_legal; cc++){
                    if (is_geometry_valid(board, r, c, rr, cc)) {
                        struct chess_board temp = b;
                        temp.squares[rr][cc] = p;
                        temp.squares[r][c] = '.';
                        if (to_upper(p) == 'P' && c != cc && b.squares[rr][cc] == '.') {
                            temp.squares[r][cc] = '.';
                        }
                        if (!is_in_check(&temp, current)) {
                            has_legal = 1; 
                        }
                    }
                }
            }
        }
    }


    if (!has_legal) {
        if (current) {
            if (b.castle_wk) {
                if (b.squares[0][5] == '.' && b.squares[0][6] == '.' && 
                    !is_in_check(board, 1) && !is_attacked(board, 0, 5, 0) && !is_attacked(board, 0, 6, 0)) has_legal = 1;
            }
            if (!has_legal && b.castle_wq) {
                if (b.squares[0][1] == '.' && b.squares[0][2] == '.' && b.squares[0][3] == '.' &&\
                    !is_in_check(board, 1) && !is_attacked(board, 0, 3, 0) && !is_attacked(board, 0, 2, 0)) has_legal = 1;
            }
        } else {
            if (b.castle_bk) {
                if (b.squares[7][5] == '.' && b.squares[7][6] == '.' && 
                    !is_in_check(board, 0) && !is_attacked(board, 7, 5, 1) && !is_attacked(board, 7, 6, 1)) has_legal = 1;
            }
            if (!has_legal && b.castle_bq) {
                if (b.squares[7][1] == '.' && b.squares[7][2] == '.' && b.squares[7][3] == '.' &&
                    !is_in_check(board, 0) && !is_attacked(board, 7, 3, 1) && !is_attacked(board, 7, 2, 1)) has_legal = 1;
            }
        }
    }

    if (has_legal) {
        printf("game incomplete\n");
    } else {
        if (is_in_check(board, current)) {
            printf("%s wins by checkmate\n", (current ? "black" : "white"));
        } else {
            printf("draw by stalemate\n");
        }
    }
}