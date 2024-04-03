/*@
 * Headers used only by the DOS curses implementation curses.c
 *
 * It is hereby considered private implementation details and as such it
 * should NOT be included curses.h or any game files.
 *
 * And despite the header name, it also contains new defines and functions
 * meant for <curses.h>. Perhaps this should be called curses_private.h instead
 */

#ifndef ROGUE_SCR_TYPE
#define ROGUE_SCR_TYPE 3  //@ 80x25 Color. See winit() for other values
#endif

#define BX_UL	0
#define BX_UR	1
#define BX_LL	2
#define BX_LR	3
#define BX_VW	4
#define BX_HT	5
#define BX_HB	6
#define BX_SIZE	7

//@ max is also in rogue.h
#define max(a,b)	((a) > (b) ? (a) : (b))
#define min(a,b)	((a) < (b) ? (a) : (b))

#ifdef ROGUE_DOS_CURSES
//@ in ncurses, as unsigned long. see wsetmem()
//typedef uint16_t	chtype;  // character with attributes
#endif  // ROGUE_DOS_CURSES


/*@
 * Function prototypes
 * Some are unused
 */
char	*sav_win(void);
void	res_win(void);
void	vbox(byte box[BX_SIZE], int ul_r, int ul_c, int lr_r, int lr_c);
#ifdef ROGUE_DOS_CURSES
void	real_rc(int pn, int *rp, int *cp);
void	error(int mline, char *msg, int a1, int a2, int a3, int a4, int a5);
void	set_cursor(void);
void	scroll_up(int start_row, int end_row, int nlines);
void	scroll_dn(int start_row, int end_row, int nlines);
void	scroll(void);
void	fixup(void);

//@ originally in zoom.asm
void	putchr(byte ch);

//@ originally in dos.asm
void	wsetmem(void *buffer, int count, chtype attrchar);
#endif  // ROGUE_DOS_CURSES

/*@
 * New stuff from now on
 */

#define A_DOS_BLACK    0x00
#define A_DOS_BLUE     0x01
#define A_DOS_GREEN    0x02
#define A_DOS_RED      0x04
#define A_DOS_WHITE    0x07
#define A_DOS_BRIGHT   0x08
#define A_DOS_BLINK    0x80

#define A_DOS_CYAN     A_DOS_BLUE  | A_DOS_GREEN
#define A_DOS_MAGENTA  A_DOS_BLUE  | A_DOS_RED
#define A_DOS_BROWN    A_DOS_GREEN | A_DOS_RED
#define A_DOS_YELLOW   A_DOS_BROWN | A_DOS_BRIGHT

#define A_DOS_FG_COLOR    0  // foreground color shift offset
#define A_DOS_BG_COLOR    4  // background color shift offset
#define A_DOS_COLOR_MASK  7  // to extract color after shifting

#define A_DOS_BG(fg)	(((fg) & A_DOS_COLOR_MASK) << A_DOS_BG_COLOR)

/*@
 * The DOS attribute set on standend(), also the initial value of ch_attr
 * Light Gray ("non-bright White") on Black
 */
#define A_DOS_NORMAL	A_DOS_WHITE

// AKA "reverse"
#define A_DOS_STANDOUT	A_DOS_BG(A_DOS_WHITE)

/*
 * Actually, for underline just setting foreground to 1 would be enough,
 * but the game uses 17 (0x11) in uline()/set_attr(12) for monoc_attr, setting
 * also the background. Not needed, but harmless
 */
#define A_DOS_BW_ULINE    1 | A_DOS_BG(1)
#define A_DOS_BW_STANDOUT A_DOS_STANDOUT | A_DOS_BRIGHT

#ifndef ROGUE_DOS_CURSES
#define PAIR_INDEX(fg, bg)	(bg * colors + fg + 1)
#define COLOR_PAIR_N(fg, bg)	COLOR_PAIR(PAIR_INDEX(fg, bg))

/*
 * Original CGA colors
 * https://en.wikipedia.org/wiki/Color_Graphics_Adapter#Color_palette
 * red   := 2/3 * (colorNumber & 4)/4 + 1/3 * (colorNumber & 8)/8
 * green := 2/3 * (colorNumber & 2)/2 + 1/3 * (colorNumber & 8)/8
 * blue  := 2/3 * (colorNumber & 1)/1 + 1/3 * (colorNumber & 8)/8
 * if colorNumber = 6 then green := green / 2
 *
 * These macros swap Red and Blue components, so `c` is ANSI index (brown is 3)
 * Return a float in range [0, 1] (both ends included!)
 */
#define CGA_COMP(c, i)	(!!((c) & i) * 2 / 3.0 + !!((c) & 8) * 1 / 3.0)
#define CGA_RED(c)	 CGA_COMP(c, 1)
#define CGA_GREEN(c)	(CGA_COMP(c, 2) / ((c) == 3 ? 2 : 1))
#define CGA_BLUE(c)	 CGA_COMP(c, 4)

#define ALENGTH(arr)	(sizeof (arr) / sizeof (*arr))
#define ASIZE(arr)	(arr + ALENGTH(arr))

#define HORIZONTAL TRUE
#define VERTICAL   FALSE

#define cur_hline(chd, length)	cur_line(chd, length, HORIZONTAL)
#define cur_vline(chd, length)	cur_line(chd, length, VERTICAL)
#define cur_mvhline(y,x,c,n)	(cur_move(y,x) == ERR ? ERR : cur_hline(c, n))
#define cur_mvvline(y,x,c,n)	(cur_move(y,x) == ERR ? ERR : cur_vline(c, n))

#ifdef ROGUE_WIDECHAR
#define cur_mvaddchnstr	mvadd_wchnstr
#define cur_mvinchnstr	mvin_wchnstr
#else
#define cur_mvaddchnstr	mvaddchnstr
#define cur_mvinchnstr	mvinchnstr
#endif  // ROGUE_WIDECHAR

#define TTY_ESC "\033"
#define TTY_CSI TTY_ESC "["
#define TTY_SS3 TTY_ESC "O"

struct ttykeys {
	char *def;
	int dest;
};
typedef struct ttykeys TTYSEQ;

struct charcode {
	byte ascii;
	wchar_t *unicode;
	byte dos;
};
typedef struct charcode CCODE;

#ifdef ROGUE_WIDECHAR
cchar_t *unicode_from_dos(byte chd, byte dos_attr, CCODE *mapping);
void	attrw_from_dos(byte dos_attr, attr_t *attrs, short *color_pair);
#endif  // ROGUE_WIDECHAR
void	define_keys(void);
byte	ascii_from_dos(byte chd, CCODE *mapping);
CCODE	*charcode_from_dos(byte chd, CCODE *mapping);
short	color_from_dos(byte dos_attr, bool fg);
chtype	attr_from_dos(byte dos_attr);
void	init_curses_colors(void);
void	resize_screen();
int	cur_line(byte chd, int length, bool orientation);
#endif  // not ROGUE_DOS_CURSES
