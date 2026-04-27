/*
 * gui.c  —  SDL2 Graphical User Interface for Anteater Chess
 * EECS 22L, Spring 2026 — Team 3 (The Knight Owls)
 */

#ifndef TEXT_ONLY

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gui.h"
#include "chess.h"
#include "rules.h"
#include "ai.h"
#include "fileio.h"

/* ================================================================== *
 *  Internal state                                                     *
 * ================================================================== */

static SDL_Window   *g_win  = NULL;
static SDL_Renderer *g_rend = NULL;

static TTF_Font *g_font_piece  = NULL;
static TTF_Font *g_font_label  = NULL;
static TTF_Font *g_font_status = NULL;
static TTF_Font *g_font_panel  = NULL;

static int        g_sel_active = 0;
static struct Pos g_sel        = {-1,-1};

static int        g_drag_active  = 0;
static struct Pos g_drag_from    = {-1,-1};
static int        g_drag_mouse_x = 0;
static int        g_drag_mouse_y = 0;

#define MAX_LEGAL 256
static struct Move g_legal[MAX_LEGAL];
static int         g_legal_count = 0;

#define MAX_LOG_LINES 128
static char g_log_lines[MAX_LOG_LINES][64];
static int  g_log_count = 0;

static char g_status[128] = "Anteater Chess — The Knight Owls";

static const char *FONT_PATH_PIECE = "/System/Library/Fonts/Supplemental/Arial Unicode.ttf";
static const char *FONT_PATH_LABEL = "/System/Library/Fonts/Helvetica.ttc";

/* ================================================================== *
 *  Unicode chess glyphs                                               *
 * ================================================================== */

static void codepoint_to_utf8(unsigned int cp, char *buf)
{
    if (cp <= 0x7F) {
        buf[0] = (char)cp; buf[1] = '\0';
    } else if (cp <= 0x7FF) {
        buf[0] = (char)(0xC0 | (cp >> 6));
        buf[1] = (char)(0x80 | (cp & 0x3F));
        buf[2] = '\0';
    } else {
        buf[0] = (char)(0xE0 | (cp >> 12));
        buf[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        buf[2] = (char)(0x80 | (cp & 0x3F));
        buf[3] = '\0';
    }
}

static const char *piece_glyph(struct Piece *p)
{
    static char buf[8];
    if (!p) { buf[0] = '\0'; return buf; }

    unsigned int cp;
    if (p->color == WHITE) {
        switch (p->type) {
            case KING:     cp = 0x2654; break;
            case QUEEN:    cp = 0x2655; break;
            case ROOK:     cp = 0x2656; break;
            case BISHOP:   cp = 0x2657; break;
            case KNIGHT:   cp = 0x2658; break;
            case PAWN:     cp = 0x2659; break;
            case ANTEATER: cp = 0x26C9; break;
            default:       cp = '?';    break;
        }
    } else {
        switch (p->type) {
            case KING:     cp = 0x265A; break;
            case QUEEN:    cp = 0x265B; break;
            case ROOK:     cp = 0x265C; break;
            case BISHOP:   cp = 0x265D; break;
            case KNIGHT:   cp = 0x265E; break;
            case PAWN:     cp = 0x265F; break;
            case ANTEATER: cp = 0x26CA; break;
            default:       cp = '?';    break;
        }
    }

    codepoint_to_utf8(cp, buf);
    return buf;
}

/* ================================================================== *
 *  Coordinate helpers                                                 *
 * ================================================================== */

#define BOARD_OX  GUI_LABEL_W
#define BOARD_OY  GUI_LABEL_H

static SDL_Rect square_rect(int rank, int file)
{
    SDL_Rect r;
    r.x = BOARD_OX + file * GUI_CELL_SIZE;
    r.y = BOARD_OY + (GUI_BOARD_ROWS - 1 - rank) * GUI_CELL_SIZE;
    r.w = GUI_CELL_SIZE;
    r.h = GUI_CELL_SIZE;
    return r;
}

static struct Pos pixel_to_pos(int px, int py)
{
    int f = (px - BOARD_OX) / GUI_CELL_SIZE;
    int r = GUI_BOARD_ROWS - 1 - (py - BOARD_OY) / GUI_CELL_SIZE;
    struct Pos p = {r, f};
    if (!pos_valid(p) || px < BOARD_OX || py < BOARD_OY)
        p.r = p.f = -1;
    return p;
}

/* ================================================================== *
 *  Text rendering helpers                                             *
 * ================================================================== */

static void render_text_centred(TTF_Font *font, const char *text,
                                 SDL_Color col, SDL_Rect rect)
{
    if (!font || !text || !text[0]) return;
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, col);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(g_rend, surf);
    if (tex) {
        SDL_Rect dst;
        dst.w = surf->w;
        dst.h = surf->h;
        dst.x = rect.x + (rect.w - dst.w) / 2;
        dst.y = rect.y + (rect.h - dst.h) / 2;
        SDL_RenderCopy(g_rend, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

static void render_text_at(TTF_Font *font, const char *text,
                             SDL_Color col, int x, int y)
{
    if (!font || !text || !text[0]) return;
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, col);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(g_rend, surf);
    if (tex) {
        SDL_Rect dst = {x, y, surf->w, surf->h};
        SDL_RenderCopy(g_rend, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

/* ================================================================== *
 *  Legal-move helpers                                                 *
 * ================================================================== */

static void compute_legal_moves(struct GameState *gs, struct Pos from)
{
    g_legal_count = 0;
    if (!pos_valid(from)) return;
    legalmoves_function(gs, from, g_legal, &g_legal_count);
}

static int is_legal_target(struct Pos to)
{
    for (int i = 0; i < g_legal_count; i++)
        if (g_legal[i].to.r == to.r && g_legal[i].to.f == to.f)
            return 1;
    return 0;
}

static int get_legal_move(struct Pos to, struct Move *out)
{
    for (int i = 0; i < g_legal_count; i++) {
        if (g_legal[i].to.r == to.r && g_legal[i].to.f == to.f) {
            *out = g_legal[i];
            return 1;
        }
    }
    return 0;
}

/* ================================================================== *
 *  Move-log panel helpers                                             *
 * ================================================================== */

static const char *file_letter(int f)
{
    static const char *letters = "ABCDEFGHIJ";
    if (f < 0 || f >= 10) return "?";
    static char s[2] = {0, 0};
    s[0] = letters[f];
    return s;
}

static void log_append(struct Move m, enum PieceColor color, int turn)
{
    if (g_log_count >= MAX_LOG_LINES) {
        memmove(g_log_lines[0], g_log_lines[1],
                sizeof(g_log_lines[0]) * (MAX_LOG_LINES - 1));
        g_log_count = MAX_LOG_LINES - 1;
    }
    const char *who = (color == WHITE) ? "W" : "B";
    snprintf(g_log_lines[g_log_count], 64, "%2d. %s  %s%d->%s%d",
             turn, who,
             file_letter(m.from.f), m.from.r + 1,
             file_letter(m.to.f),   m.to.r   + 1);
    g_log_count++;
}

/* ================================================================== *
 *  Promotion dialog                                                   *
 * ================================================================== */

static enum PieceType promotion_dialog(enum PieceColor color)
{
    int bw = GUI_BOARD_COLS * GUI_CELL_SIZE;
    int bh = GUI_BOARD_ROWS * GUI_CELL_SIZE;
    int box_w = 80, box_h = 90;
    int total_w = 4 * (box_w + 10) - 10;
    int start_x = BOARD_OX + (bw - total_w) / 2;
    int start_y = BOARD_OY + (bh - box_h)   / 2;

    static const enum PieceType opts[4] = {QUEEN, ROOK, BISHOP, KNIGHT};
    static const char *labels[4] = {"Queen", "Rook", "Bishop", "Knight"};

    SDL_Color white_col = {255, 255, 255, 255};
    SDL_Color dark_col  = {30,  30,  40,  255};

    while (1) {
        SDL_SetRenderDrawBlendMode(g_rend, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(g_rend, 0, 0, 0, 160);
        SDL_Rect overlay = {0, 0, GUI_WIN_W, GUI_WIN_H};
        SDL_RenderFillRect(g_rend, &overlay);

        for (int i = 0; i < 4; i++) {
            SDL_Rect box = {start_x + i * (box_w + 10), start_y, box_w, box_h};
            SDL_SetRenderDrawColor(g_rend, 60, 60, 80, 255);
            SDL_RenderFillRect(g_rend, &box);
            SDL_SetRenderDrawColor(g_rend, 180, 180, 220, 255);
            SDL_RenderDrawRect(g_rend, &box);

            struct Piece tmp = {opts[i], color};
            SDL_Rect glyph_rect = {box.x, box.y, box_w, box_h - 24};
            render_text_centred(g_font_piece, piece_glyph(&tmp),
                                white_col, glyph_rect);

            SDL_Rect label_rect = {box.x, box.y + box_h - 22, box_w, 22};
            render_text_centred(g_font_status, labels[i], dark_col, label_rect);
        }

        SDL_Rect title_rect = {BOARD_OX, start_y - 36, bw, 32};
        render_text_centred(g_font_status, "Choose promotion piece:",
                            white_col, title_rect);

        SDL_RenderPresent(g_rend);

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return QUEEN;
            if (e.type == SDL_MOUSEBUTTONDOWN &&
                e.button.button == SDL_BUTTON_LEFT) {
                int mx = e.button.x, my = e.button.y;
                for (int i = 0; i < 4; i++) {
                    SDL_Rect box = {start_x + i * (box_w + 10), start_y,
                                    box_w, box_h};
                    if (mx >= box.x && mx < box.x + box.w &&
                        my >= box.y && my < box.y + box.h)
                        return opts[i];
                }
            }
        }
        SDL_Delay(16);
    }
}

/* ================================================================== *
 *  Rendering                                                          *
 * ================================================================== */

static void draw_board_squares(void)
{
    for (int rank = 0; rank < GUI_BOARD_ROWS; rank++) {
        for (int file = 0; file < GUI_BOARD_COLS; file++) {
            SDL_Rect sq = square_rect(rank, file);
            if ((rank + file) % 2 == 0)
                SDL_SetRenderDrawColor(g_rend, GUI_COL_LIGHT);
            else
                SDL_SetRenderDrawColor(g_rend, GUI_COL_DARK);
            SDL_RenderFillRect(g_rend, &sq);
        }
    }
}

static void draw_highlights(struct GameState *gs)
{
    SDL_SetRenderDrawBlendMode(g_rend, SDL_BLENDMODE_BLEND);

    for (int color = 0; color <= 1; color++) {
        if (king_in_check(gs, (enum PieceColor)color)) {
            for (int r = 0; r < NUM_RANKS; r++)
                for (int f = 0; f < NUM_FILES; f++) {
                    struct Piece *p = gs->board[r][f];
                    if (p && p->type == KING && (int)p->color == color) {
                        SDL_Rect sq = square_rect(r, f);
                        SDL_SetRenderDrawColor(g_rend, GUI_COL_CHECK);
                        SDL_RenderFillRect(g_rend, &sq);
                    }
                }
        }
    }

    if (g_sel_active && pos_valid(g_sel)) {
        SDL_Rect sq = square_rect(g_sel.r, g_sel.f);
        SDL_SetRenderDrawColor(g_rend, GUI_COL_SELECT);
        SDL_RenderFillRect(g_rend, &sq);
    }

    for (int i = 0; i < g_legal_count; i++) {
        struct Pos to = g_legal[i].to;
        SDL_Rect sq = square_rect(to.r, to.f);
        struct Piece *victim = gs->board[to.r][to.f];

        SDL_SetRenderDrawColor(g_rend, GUI_COL_LEGAL);
        if (victim) {
            for (int t = 0; t < 5; t++) {
                SDL_Rect ring = {sq.x + t, sq.y + t,
                                 sq.w - 2*t, sq.h - 2*t};
                SDL_RenderDrawRect(g_rend, &ring);
            }
        } else {
            int rad = GUI_CELL_SIZE / 6;
            int cx = sq.x + sq.w / 2, cy = sq.y + sq.h / 2;
            for (int dy = -rad; dy <= rad; dy++) {
                int dx = (int)SDL_sqrt((double)(rad*rad - dy*dy));
                SDL_Rect line = {cx - dx, cy + dy, 2*dx + 1, 1};
                SDL_RenderFillRect(g_rend, &line);
            }
        }
    }
}

static void draw_pieces(struct GameState *gs)
{
    SDL_Color white_piece = {255, 255, 255, 255};
    SDL_Color black_piece = {20,  20,  20,  255};
    SDL_Color white_shad  = {80,  60,  20,  200};
    SDL_Color black_shad  = {200, 200, 200, 120};

    for (int rank = 0; rank < NUM_RANKS; rank++) {
        for (int file = 0; file < NUM_FILES; file++) {
            if (g_drag_active &&
                rank == g_drag_from.r && file == g_drag_from.f)
                continue;

            struct Piece *p = gs->board[rank][file];
            if (!p) continue;

            SDL_Rect sq = square_rect(rank, file);
            const char *glyph = piece_glyph(p);

            SDL_Rect shadow_r = {sq.x + 2, sq.y + 2, sq.w, sq.h};
            render_text_centred(g_font_piece, glyph,
                                (p->color == WHITE) ? white_shad : black_shad,
                                shadow_r);
            render_text_centred(g_font_piece, glyph,
                                (p->color == WHITE) ? white_piece : black_piece,
                                sq);
        }
    }
}

static void draw_drag_piece(struct GameState *gs)
{
    if (!g_drag_active || !pos_valid(g_drag_from)) return;
    struct Piece *p = gs->board[g_drag_from.r][g_drag_from.f];
    if (!p) return;

    SDL_Color col = (p->color == WHITE) ?
                    (SDL_Color){255,255,255,255} :
                    (SDL_Color){20,20,20,255};
    int half = GUI_CELL_SIZE / 2;
    SDL_Rect r = {g_drag_mouse_x - half, g_drag_mouse_y - half,
                  GUI_CELL_SIZE, GUI_CELL_SIZE};
    render_text_centred(g_font_piece, piece_glyph(p), col, r);
}

static void draw_labels(void)
{
    SDL_Color lc = {GUI_COL_LABEL};
    const char *fnames = "ABCDEFGHIJ";

    for (int f = 0; f < GUI_BOARD_COLS; f++) {
        char s[2] = {fnames[f], 0};
        SDL_Rect tr = {BOARD_OX + f * GUI_CELL_SIZE, 0,
                        GUI_CELL_SIZE, GUI_LABEL_H};
        render_text_centred(g_font_label, s, lc, tr);

        SDL_Rect br = {BOARD_OX + f * GUI_CELL_SIZE,
                        BOARD_OY + GUI_BOARD_ROWS * GUI_CELL_SIZE,
                        GUI_CELL_SIZE, GUI_LABEL_H};
        render_text_centred(g_font_label, s, lc, br);
    }

    for (int r = 0; r < GUI_BOARD_ROWS; r++) {
        char s[3];
        snprintf(s, sizeof(s), "%d", r + 1);
        SDL_Rect lr = {0, BOARD_OY + (GUI_BOARD_ROWS - 1 - r) * GUI_CELL_SIZE,
                        GUI_LABEL_W, GUI_CELL_SIZE};
        render_text_centred(g_font_label, s, lc, lr);
    }
}

static void draw_status(void)
{
    int bar_y = BOARD_OY + GUI_BOARD_ROWS * GUI_CELL_SIZE + GUI_LABEL_H;
    SDL_Rect bar = {0, bar_y,
                    BOARD_OX + GUI_BOARD_COLS * GUI_CELL_SIZE, GUI_STATUS_H};
    SDL_SetRenderDrawColor(g_rend, GUI_COL_STATUS);
    SDL_RenderFillRect(g_rend, &bar);

    SDL_Color tc = {GUI_COL_TEXT};
    render_text_at(g_font_status, g_status, tc,
                   bar.x + 10,
                   bar_y + (GUI_STATUS_H - GUI_STATUS_FONT_SIZE) / 2);
}

static void draw_panel(void)
{
    int px = BOARD_OX + GUI_BOARD_COLS * GUI_CELL_SIZE;
    SDL_Rect panel = {px, 0, GUI_PANEL_W, GUI_WIN_H};
    SDL_SetRenderDrawColor(g_rend, GUI_COL_PANEL);
    SDL_RenderFillRect(g_rend, &panel);

    SDL_Color title_col = {180, 180, 255, 255};
    SDL_Color log_col   = {200, 200, 200, 255};

    render_text_at(g_font_status, "Move Log", title_col, px + 10, 10);

    SDL_SetRenderDrawColor(g_rend, 80, 80, 120, 255);
    SDL_RenderDrawLine(g_rend, px + 5, 34, px + GUI_PANEL_W - 5, 34);

    int line_h   = GUI_PANEL_FONT_SIZE + 4;
    int max_lines = (GUI_WIN_H - 50) / line_h;
    int start    = (g_log_count > max_lines) ? g_log_count - max_lines : 0;

    for (int i = start; i < g_log_count; i++) {
        int ly = 42 + (i - start) * line_h;
        render_text_at(g_font_panel, g_log_lines[i], log_col, px + 10, ly);
    }
}

static void render_frame(struct GameState *gs)
{
    SDL_SetRenderDrawColor(g_rend, 18, 18, 28, 255);
    SDL_RenderClear(g_rend);
    draw_board_squares();
    draw_highlights(gs);
    draw_pieces(gs);
    draw_drag_piece(gs);
    draw_labels();
    draw_status();
    draw_panel();
    SDL_RenderPresent(g_rend);
}

/* ================================================================== *
 *  Input handling                                                     *
 * ================================================================== */

static int try_move(struct GameState *gs, struct Pos from, struct Pos to,
                    enum PieceColor human_color)
{
    if (!pos_valid(from) || !pos_valid(to)) return 0;

    struct Move m;
    if (!get_legal_move(to, &m)) return 0;

    struct Piece *moving = gs->board[from.r][from.f];
    if (moving && moving->type == PAWN) {
        int last_rank = (moving->color == WHITE) ? (NUM_RANKS - 1) : 0;
        if (to.r == last_rank) {
            render_frame(gs);
            enum PieceType promo = promotion_dialog(moving->color);
            m.was_promotion = 1;
            m.promoted_to   = promo;
        }
    }

    apply_move(gs, m);
    gs->current_turn = OPPONENT(human_color);
    logfile_function(m, gs->history.count, human_color, "chess_log.txt");
    log_append(m, human_color, gs->history.count);

    g_sel_active  = 0;
    g_legal_count = 0;
    return 1;
}

static int handle_click(struct GameState *gs, struct Pos pos,
                         enum PieceColor human_color)
{
    if (!pos_valid(pos)) {
        g_sel_active = 0; g_legal_count = 0;
        return 0;
    }

    if (!g_sel_active) {
        struct Piece *p = gs->board[pos.r][pos.f];
        if (p && p->color == human_color) {
            g_sel        = pos;
            g_sel_active = 1;
            compute_legal_moves(gs, pos);
        }
        return 0;
    }

    if (is_legal_target(pos))
        return try_move(gs, g_sel, pos, human_color);

    struct Piece *p = gs->board[pos.r][pos.f];
    if (p && p->color == human_color) {
        g_sel        = pos;
        g_sel_active = 1;
        compute_legal_moves(gs, pos);
    } else {
        g_sel_active  = 0;
        g_legal_count = 0;
    }
    return 0;
}

/* ================================================================== *
 *  Computer move                                                      *
 * ================================================================== */

static void do_computer_move(struct GameState *gs,
                              enum PieceColor comp_color, int ai_depth)
{
    snprintf(g_status, sizeof(g_status), "Computer is thinking...");
    render_frame(gs);

    gs->current_turn = comp_color;  /* bestmove_function needs this set */
    struct Move m = bestmove_function(gs, comp_color, ai_depth);
    struct Piece *moving = gs->board[m.from.r][m.from.f];
    if (moving && moving->type == PAWN) {
        int last_rank = (comp_color == WHITE) ? (NUM_RANKS - 1) : 0;
        if (m.to.r == last_rank) {
            m.was_promotion = 1;
            m.promoted_to   = QUEEN;
        }
    }

    apply_move(gs, m);
    logfile_function(m, gs->history.count, comp_color, "chess_log.txt");
    log_append(m, comp_color, gs->history.count);
}

/* ================================================================== *
 *  Public API                                                         *
 * ================================================================== */

int gui_init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return -1;
    }
    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        return -1;
    }

    g_win = SDL_CreateWindow(
        "Anteater Chess — The Knight Owls (EECS 22L)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        GUI_WIN_W, GUI_WIN_H, SDL_WINDOW_SHOWN);
    if (!g_win) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        TTF_Quit(); SDL_Quit();
        return -1;
    }

    g_rend = SDL_CreateRenderer(g_win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_rend) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(g_win);
        TTF_Quit(); SDL_Quit();
        return -1;
    }

    g_font_piece  = TTF_OpenFont(FONT_PATH_PIECE, GUI_PIECE_FONT_SIZE);
    g_font_label  = TTF_OpenFont(FONT_PATH_LABEL, GUI_LABEL_FONT_SIZE);
    g_font_status = TTF_OpenFont(FONT_PATH_LABEL, GUI_STATUS_FONT_SIZE);
    g_font_panel  = TTF_OpenFont(FONT_PATH_LABEL, GUI_PANEL_FONT_SIZE);

    if (!g_font_piece || !g_font_label || !g_font_status || !g_font_panel)
        fprintf(stderr, "TTF_OpenFont warning: %s\n", TTF_GetError());

    SDL_SetRenderDrawBlendMode(g_rend, SDL_BLENDMODE_BLEND);
    return 0;
}

void gui_shutdown(void)
{
    if (g_font_piece)  TTF_CloseFont(g_font_piece);
    if (g_font_label)  TTF_CloseFont(g_font_label);
    if (g_font_status) TTF_CloseFont(g_font_status);
    if (g_font_panel)  TTF_CloseFont(g_font_panel);
    if (g_rend) SDL_DestroyRenderer(g_rend);
    if (g_win)  SDL_DestroyWindow(g_win);
    TTF_Quit();
    SDL_Quit();
}

GuiResult gui_run(struct GameState *gs, enum PieceColor human_color,
                  int ai_depth)
{
    enum PieceColor comp_color = OPPONENT(human_color);
    int running   = 1;
    int game_over = 0;

    snprintf(g_status, sizeof(g_status), "You are %s — good luck!",
             human_color == WHITE ? "White" : "Black");

    while (running) {

        /* ── Events ───────────────────────────────────────────────── */
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {

                case SDL_QUIT:
                    running = 0;
                    break;

                case SDL_KEYDOWN:
                    if (e.key.keysym.sym == SDLK_ESCAPE)
                        running = 0;
                    if (e.key.keysym.sym == SDLK_u && !game_over) {
                        if (gs->history.count >= 2) {
                            undo_move(gs); undo_move(gs);
                            if (g_log_count >= 2) g_log_count -= 2;
                            g_sel_active = 0; g_legal_count = 0;
                            snprintf(g_status, sizeof(g_status), "Move undone.");
                        }
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (game_over || gs->current_turn != human_color) break;
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        struct Pos pos = pixel_to_pos(e.button.x, e.button.y);
                        if (pos_valid(pos) && gs->board[pos.r][pos.f] &&
                            gs->board[pos.r][pos.f]->color == human_color) {
                            g_drag_active  = 1;
                            g_drag_from    = pos;
                            g_drag_mouse_x = e.button.x;
                            g_drag_mouse_y = e.button.y;
                            g_sel        = pos;
                            g_sel_active = 1;
                            compute_legal_moves(gs, pos);
                        } else {
                            handle_click(gs, pos, human_color);
                        }
                    }
                    break;

                case SDL_MOUSEMOTION:
                    if (g_drag_active) {
                        g_drag_mouse_x = e.motion.x;
                        g_drag_mouse_y = e.motion.y;
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (game_over) break;
                    if (e.button.button == SDL_BUTTON_LEFT && g_drag_active) {
                        struct Pos drop = pixel_to_pos(e.button.x, e.button.y);
                        g_drag_active = 0;
                        if (pos_valid(drop) && is_legal_target(drop))
                            try_move(gs, g_drag_from, drop, human_color);
                        else
                            handle_click(gs, drop, human_color);
                    }
                    break;

                default: break;
            }
        }

        /* ── Game-over / status update ────────────────────────────── */
        if (!game_over) {
            if (king_in_checkmate(gs, BLACK)) {
                snprintf(g_status, sizeof(g_status),
                         "Checkmate! White wins! (Esc to quit)");
                game_over = 1;
            } else if (king_in_checkmate(gs, WHITE)) {
                snprintf(g_status, sizeof(g_status),
                         "Checkmate! Black wins! (Esc to quit)");
                game_over = 1;
            } else if (king_in_stalemate(gs, gs->current_turn)) {
                snprintf(g_status, sizeof(g_status),
                         "Stalemate! Draw! (Esc to quit)");
                game_over = 1;
            } else if (king_in_check(gs, gs->current_turn)) {
                snprintf(g_status, sizeof(g_status), "%s to move — CHECK!",
                         gs->current_turn == WHITE ? "White" : "Black");
            } else {
                snprintf(g_status, sizeof(g_status), "%s to move  [U = undo]",
                         gs->current_turn == WHITE ? "White" : "Black");
            }
        }

        /* ── Render ───────────────────────────────────────────────── */
        render_frame(gs);

        /* ── Computer move ────────────────────────────────────────── */
        if (!game_over && gs->current_turn == comp_color) {
            SDL_Delay(300);
            do_computer_move(gs, comp_color, ai_depth);
            gs->current_turn = human_color;  /* force turn back to human */
            g_sel_active  = 0;
            g_legal_count = 0;
            render_frame(gs);
        }

        SDL_Delay(8);
    }

    return GUI_QUIT;
}

#endif /* !TEXT_ONLY */