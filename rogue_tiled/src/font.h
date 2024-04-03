
#define CHAR_COLUMNS 16
#define CHAR_WIDTH 16
#define CHAR_HEIGHT 16
#define CHAR_OFFSET 0

#define TILE_COLUMNS 16
#define TILE_WIDTH 32
#define TILE_HEIGHT 32
#define TILE_OFFSET 0

void load_font(void);
void free_font(void);
void color_font(Uint32 color);
void print_char(SDL_Surface *surface, int x, int y, unsigned char c);
void print_tile(SDL_Surface *surface, int x, int y, unsigned char c);
void print_string(SDL_Surface *surface, int x, int y, char *s);
void print_char_colored(SDL_Surface *surface, int x, int y, unsigned char c, Uint32 color);
void print_string_colored(SDL_Surface *surface, int x, int y, char *s, Uint32 color);
