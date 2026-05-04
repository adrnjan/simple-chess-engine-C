#include "parser.h"
#include <stdio.h>
#include "panic.h"

int is_space(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
int is_file(char c)  { return c >= 'a' && c <= 'h'; }
int is_rank(char c)  { return c >= '1' && c <= '8'; }

int is_castle_char(char c) { return c == '0' || c == 'O' || c == 'o'; }

bool parse_move(struct chess_move *move)
{
    int c; 

    do { c = getc(stdin); } while (is_space(c));

    if (c == EOF) return false;

    struct chess_move m;

    m.piece_type      = 'P';
    m.start_rank      = -1;
    m.start_file      = -1;
    m.end_rank        = -1;
    m.end_file        = -1;
    m.disambig_file   = -1;
    m.disambig_rank   = -1;
    m.capture_flag    = 0;
    m.castling_type   = CASTLE_NONE;
    m.promotion_piece = '.';

    if (is_castle_char(c)) {
        char n1 = getc(stdin);
        if (n1 != '-') panicf("parse error at character '%c'\n", n1);

        char n2 = getc(stdin);
        if (!is_castle_char(n2)) panicf("parse error at character '%c'\n", n2);

        char n3 = getc(stdin);
        if (n3 == '-') {
            char n4 = getc(stdin);
            if (!is_castle_char(n4)) panicf("parse error at character '%c'\n", n4);
            m.castling_type = CASTLE_QUEENSIDE;
        } else {
            ungetc(n3, stdin);
            m.castling_type = CASTLE_KINGSIDE;
        }

        m.piece_type = 'K';
        *move = m;
        return true;
    }

    if (c == 'K' || c == 'Q' || c == 'R' || c == 'B' || c == 'N') {
        m.piece_type = c;
    } else {
        ungetc(c, stdin);
        m.piece_type = 'P';
    }

    char tokens[5]; 
    int  types[5];   
    int  count = 0;
    int  t; 

    while (1) {
        t = getc(stdin); 

        if (t == 'x') {
            m.capture_flag = 1;
            continue;
        }

        if (t == '=' || t == '+' || t == '#' || is_space(t) || t == EOF) {
            if (t != EOF) ungetc(t, stdin); 
            break;
        }

        int type = -1;
        if (is_file(t))      type = 0;
        else if (is_rank(t)) type = 1;
        else {
            panicf("parse error at character '%c'\n", t);
        }

        if (m.piece_type == 'P') {
            if (type == 0 && count == 1 && types[0] == 0 && m.capture_flag == 0) {
                panicf("parse error at character '%c'\n", t);
            }
            if (count >= 3) {
                panicf("parse error at character '%c'\n", t);
            }
        } else {
            if (count >= 4) {
                panicf("parse error at character '%c'\n", t);
            }
        }

        tokens[count] = t;
        types[count]  = type;
        count++;
    }

    if (count < 2) {
        panicf("parse error at character '%c'\n", t);
    }

    int dest = count - 2;
    if (types[dest] != 0 || types[dest + 1] != 1) {
        panicf("parse error at character '%c'\n", tokens[dest]);
    }

    m.end_file = tokens[dest]     - 'a';
    m.end_rank = tokens[dest + 1] - '1';

    for (int i = 0; i < dest; i++) {
        if (types[i] == 0) m.disambig_file = tokens[i] - 'a';
        else               m.disambig_rank = tokens[i] - '1';
    }

    //Promotion
    c = getc(stdin);
    if (c == '=') {
        m.promotion_piece = getc(stdin);
    } else {
        if (c != EOF) ungetc(c, stdin);
    }

    c = getc(stdin);
    if (c != '+' && c != '#') {
        if (c != EOF) ungetc(c, stdin);
    }

    c = getc(stdin);
    if (!is_space(c) && c != EOF) {
        panicf("parse error at character '%c'\n", c);
    }
    if (c != EOF) ungetc(c, stdin);

    *move = m;
    return true;
}