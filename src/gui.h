#ifndef GUI_H
#define GUI_H

/*
 * gui.h  —  SDL2 Graphical User Interface for Anteater Chess
 * EECS 22L, Spring 2026 — Team 3 (The Knight Owls)
 *
 * Provides board rendering, piece graphics (Unicode via SDL_ttf),
 * and click/drag input handling.
 */

#include "chess.h"

/* ── Window / layout constants ──────────────────────────────────── */
#define GUI_CELL_SIZE   72          /* pixels per square              */
#define GUI_BOARD_COLS  NUM_FILES   /* 10                             */
#define GUI_BOARD_ROWS  NUM_RANKS   /* 8                              */
#define GUI_LABEL_W     36          /* rank/file label column width   */
#define GUI_LABEL_H     36          /* rank/file label row height     */
#define GUI_STATUS_H    52          /* status bar height at bottom    */
#define GUI_PANEL_W     220         /* right-side info panel width    */

#define GUI_WIN_W  (GUI_LABEL_W + GUI_BOARD_COLS * GUI_CELL_SIZE + GUI_PANEL_W)
#define GUI_WIN_H  (GUI_LABEL_H + GUI_BOARD_ROWS * GUI_CELL_SIZE + GUI_STATUS_H)

/* ── Colour palette (RGBA) ──────────────────────────────────────── */
#define GUI_COL_LIGHT   0xF0, 0xD9, 0xB5, 0xFF   /* cream square      */
#define GUI_COL_DARK    0xB5, 0x88, 0x63, 0xFF   /* brown square      */
#define GUI_COL_SELECT  0xF6, 0xF6, 0x69, 0xCC   /* selected square   */
#define GUI_COL_LEGAL   0x50, 0xC8, 0x78, 0xAA   /* legal-move dot    */
#define GUI_COL_CHECK   0xE7, 0x2B, 0x2B, 0xBB   /* king-in-check     */
#define GUI_COL_STATUS  0x2B, 0x2B, 0x2B, 0xFF   /* status bar bg     */
#define GUI_COL_PANEL   0x1E, 0x1E, 0x2E, 0xFF   /* right panel bg    */
#define GUI_COL_TEXT    0xFF, 0xFF, 0xFF, 0xFF   /* general text      */
#define GUI_COL_LABEL   0x88, 0x88, 0xAA, 0xFF   /* coord labels      */

/* ── Font size ──────────────────────────────────────────────────── */
#define GUI_PIECE_FONT_SIZE  46   /* Unicode chess glyph             */
#define GUI_LABEL_FONT_SIZE  16   /* A-J / 1-8 coordinate labels     */
#define GUI_STATUS_FONT_SIZE 18   /* status bar message              */
#define GUI_PANEL_FONT_SIZE  15   /* move log inside panel           */

/* ── GUI result codes returned by gui_run() ────────────────────── */
typedef enum {
    GUI_QUIT   = 0,   /* user closed the window          */
    GUI_WHITE  = 1,   /* white player chose to start     */
    GUI_BLACK  = 2    /* black player chose to start     */
} GuiResult;

/* ── Public API ─────────────────────────────────────────────────── */

/*
 * gui_init()
 *   Initialise SDL2, create window and renderer, load fonts.
 *   Returns 0 on success, -1 on failure (error printed to stderr).
 *   Must be called before any other gui_* function.
 */
int gui_init(void);

/*
 * gui_shutdown()
 *   Destroy SDL resources and quit SDL2.  Call at program exit.
 */
void gui_shutdown(void);

/*
 * gui_run()
 *   Main GUI event loop.  Drives a complete game using *gs.
 *   The human plays as `human_color`; the computer plays the other side.
 *   Returns GUI_QUIT when the window is closed or game ends.
 */
GuiResult gui_run(struct GameState *gs, enum PieceColor human_color, int ai_depth, int mode);

#endif /* GUI_H */