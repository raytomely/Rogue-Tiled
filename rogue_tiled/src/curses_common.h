/*@
 * Header for curses-related objects used by both the game files and
 * the internal curses.c
 *
 * Some of the defines, the ones part of an informal group such as the colors,
 * are not actually used by both, and may even not be used at all, but were
 * kept together here for consistency and completeness.
 */

//@ not in original, to make IDE happy about 'bool'
#include "extern.h"
#include "keypad.h"

//@ Available charsets, not in original
#define ASCII	1
#define CP437	2
#define UNICODE	3

//@ User-selected charset, also not in original
#if !(ROGUE_CHARSET == ASCII || ROGUE_CHARSET == CP437 || ROGUE_CHARSET == UNICODE)
	//@ Factory default charset
	#define ROGUE_CHARSET	UNICODE
#endif
//@ UNICODE is subject to curses wide char availability
#if ROGUE_CHARSET == UNICODE && !defined (_XOPEN_CURSES)
	#undef  ROGUE_CHARSET
	#define ROGUE_CHARSET	ASCII
#endif
//@ Only enable wide chars if actually needed
#if ROGUE_CHARSET == UNICODE
	#define ROGUE_WIDECHAR
#endif


//@ Columns mode - should (but currently isn't) be selected at run-time
#ifndef ROGUE_COLUMNS
#define ROGUE_COLUMNS 80
#endif

/*
 * Don't change the constants, since they are used for sizes in many
 * places in the program.
 * @ Heed the warning! 80 and 25 are hard coded in many places... sigh
 * @ moved from rogue.h
 */
#define MAXSTR  	80	/* maximum length of strings */
#define MAXLINES	25	/* maximum number of screen lines used */
#define MAXCOLS 	80	/* maximum number of screen columns used */


/*@
 * This color/bw checks are inconsistent with each other:
 * scr_type 0 and 2 evaluate as TRUE for both (but they are mono),
 * scr_type 7 evaluate as FALSE for both (also mono)
 * See winit()
 */
#define is_color (scr_type!=7)
#define is_bw (scr_type==0 || scr_type==2)

//@ moved from rogue.h
#ifndef CTRL
#define CTRL(ch)	((ch) & 037)
#endif

#define cur_standend() set_attr( 0)  //@ normal white (light gray) on black
#define green()        set_attr( 1)
#define cyan()         set_attr( 2)
#define red()          set_attr( 3)
#define magenta()      set_attr( 4)
#define brown()        set_attr( 5)  //@ yellow, made brown by CGA hardware
#define dgrey()        set_attr( 6)  //@ "bright black". unused
#define lblue()        set_attr( 7)
#define lgrey()        set_attr( 8)  //@ bright *green*, not gray. unused
#define lred()         set_attr( 9)
#define lmagenta()     set_attr(10)
#define yellow()       set_attr(11)
#define uline()        set_attr(12)  //@ bright white on color, underline on bw
#define blue()         set_attr(13)
#define cur_standout() set_attr(14)  //@ black on normal white (reverse)
#define high()         set_attr(15)  //@ bright white on color, normal on bw
#define bold()         set_attr(16)  //@ black on normal white (reverse)

#define cur_getch()	cur_getch_timeout(-1)

/*
 * Things that appear on the screens
 * @ moved from rogue.h
 */
#define PASSAGE		(0xb1)
#define DOOR		(0xce)
#define FLOOR		(0xfa)
#define PLAYER		(0x01)
#define TRAP		(0x04)
#define STAIRS		(0xf0)
#define GOLD		(0x0f)
#define POTION		(0xad)
#define SCROLL		(0x0d)
#define MAGIC		'$'
#define BMAGIC		'+'  //'~'//@ originally '+'. Reverse ASCII map must be unique
#define FOOD		(0x05)
#define STICK		(0xe7)
#define ARMOR		(0x08)
#define AMULET		(0x0c)
#define RING		(0x09)
#define WEAPON		(0x18)
#define CALLABLE	-1

#define VWALL	(0xba)
#define HWALL	(0xcd)
#define ULWALL	(0xc9)
#define URWALL	(0xbb)
#define LLWALL	(0xc8)
#define LRWALL	(0xbc)

//@ The following were not in original - values were hard-coded

//@ single-width box glyphs
#define HLINE	(0xc4)
#define VLINE	(0xb3)
#define CORNER	'+'  //@ unused, added just for completeness
#define ULCORNER	(0xda)
#define URCORNER	(0xbf)
#define LLCORNER	(0xc0)
#define LRCORNER	(0xd9)

//@ double-width box glyphs
#define DHLINE	HWALL  // 205 in credits()
#define DVLINE	VWALL
#define DCORNER	'#'  //@ also unused
#define DULCORNER	ULWALL
#define DURCORNER	URWALL
#define DLLCORNER	LLWALL
#define DLRCORNER	LRWALL

//@ only used in credits()
#define DVLEFT	(0xb9)  //@ 185
#define DVRIGHT	(0xcc)  //@ 204

//@ only used in drop_curtain()
#define FILLER	PASSAGE


//@ moved from rogue.h
#define ESCAPE	(27)

//@ same as ERR, but different semantics
#define NOCHAR	(-1)

//@ total time, in milliseconds, for each drop and raise curtain animation
#define CURTAIN_TIME	1500

/*@
 * Function prototypes
 * Names with 'cur_' prefix were renamed to avoid conflict with <curses.h>
 */
typedef uint16_t	chtype;
byte	xlate_ch(int ch);
void	clear(void);
bool	cursor(bool ison);
void	getrc(int *rp, int *cp);
void	refresh(void);
void	clrtoeol(void);
void	mvaddstr(int r, int c, char *s);
void	mvaddch(int r, int c, byte chr);
byte	mvinch(int r, int c);
int	    addch(byte chr);
void	addstr(char *s);
void	set_attr(int bute);
void	winit(void);
void	wdump(void);
void	wrestor(void);
void	cur_endwin(void);
void	cur_box(int ul_r, int ul_c, int lr_r, int lr_c);
void	center(int row, char *string);
void	cur_printw(const char *msg, ...);
void	repchr(byte chr, int cnt);
void	implode(void);
void	drop_curtain(void);
void	raise_curtain(void);
byte	get_mode(void);
byte	video_mode(int type);
void    wsetmem(void *buffer, int count, chtype attrchar);
void	switch_page(int pn);
void	blot_out(int ul_row, int ul_col, int lr_row, int lr_col);
void     move(int row, int col);
byte    curch(void);
void    scroll_up(int start_row, int end_row, int nlines);
void    putchr(byte ch);
void    printw(const char *msg, ...);
bool    isjr();
void    dmaout(void* data, unsigned int wordlength, unsigned int segment, unsigned int offset);
void    dmain(void* data, unsigned int wordlength, unsigned int segment, unsigned int offset);
void    beep(void);
void    addstr(char *s);
void    box(int ul_r, int ul_c,int lr_r,int lr_c);
void    vbox(byte *box, int ul_r, int ul_c,int lr_r,int lr_c);
int     getch();
bool    no_char();
#define cur_inch curch
void    wclose();
#define cur_endwin wclose
void cursor_flash(void);
void draw_tile(byte ch);
void draw_rogue_tile(byte c);
void move_and_draw_tile(int r, int c, byte chr);
void draw_map_rect(void);
void draw_scrolling_map(void);
bool draw_weapon_at(int y, int x);

//@ originally in dos.asm
void	cur_beep(void);
int 	cur_getch_timeout(int msdelay);

//@ originally in zoom.asm
int	cur_move(int row, int col);
byte	cur_inch(void);

//@ moved from io.c
int 	getinfo(char *str, int size);
void	backspace(void);
