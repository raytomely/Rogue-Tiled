#include <dirent.h>
#include <SDL/SDL.h>
#include <math.h>
#include "util.h"

#define RMASK 0XFF0000
#define GMASK 0XFF00
#define BMASK 0XFF

#define RSHIFT 16
#define GSHIFT 8
#define BSHIFT 0

#define UNPACK_RGB(r, g, b, rgb) r = (rgb & RMASK) >> RSHIFT; g = (rgb & GMASK) >> GSHIFT; b = (rgb & BMASK) >> BSHIFT
#define PACK_RGB(r, g, b) (r << RSHIFT) | (g << GSHIFT) | (b << BSHIFT)

int listdir(void)
{
  DIR *dp;
  struct dirent *ep;
  dp = opendir ("./");
  if (dp != NULL)
  {
    while ((ep = readdir (dp)) != NULL)
      puts (ep->d_name);

    (void) closedir (dp);
    return 0;
  }
  else
  {
    perror ("Couldn't open the directory");
    return -1;
  }
}

void sleep(void)
{
    static int old_time = 0,  actual_time = 0;
    actual_time = SDL_GetTicks();
    if (actual_time - old_time < 16) // if less than 16 ms has passed
    {
        SDL_Delay(16 - (actual_time - old_time));
        old_time = SDL_GetTicks();
    }
    else
    {
        old_time = actual_time;
    }
}

SDL_Surface *create_surface32(int width, int height)
{
    SDL_Surface *surface;
    Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
    surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, rmask, gmask, bmask, amask);
    convert_surface(&surface);
    return surface;
}

SDL_Surface *copy_surface(SDL_Surface *surface)
{
    return SDL_ConvertSurface(surface, surface->format, SDL_SWSURFACE);
}

void setPixel32(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    if (x >= surface->w || y >= surface->h || x < 0 || y < 0)
        return;
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    *(Uint32 *)p = pixel;
}

Uint32 getpixel32(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    return *(Uint32 *)p;
}

void convert_surface(SDL_Surface **surface)
{
    SDL_Surface *temp_surf = &(**surface);
    *surface = SDL_DisplayFormat (temp_surf);
    SDL_FreeSurface(temp_surf);
}

void convert_surface_alpha(SDL_Surface **surface)
{
    SDL_Surface *temp_surf = &(**surface);
    *surface = SDL_DisplayFormatAlpha(temp_surf);
    SDL_FreeSurface(temp_surf);
}

int get_file_size(FILE* file)
{
    fseek (file, 0, SEEK_END);
    int file_size = ftell(file);
    fseek (file, 0, SEEK_SET);
    return file_size;
}

void blit_colored(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)
{
    int x, y, x2, bpp;
    Uint32 pixel, alpha_pixel = src->format->colorkey;

    // first do trivial rejections of bitmap, is it totally invisible?
    if ((dstrect->x >= dst->w) || (dstrect->y >= dst->h) ||
       ((dstrect->x + srcrect->w) <= 0) || ((dstrect->y + srcrect->h) <= 0))
            return;

    // clip rectangles
    // upper left hand corner first
    if (dstrect->x < 0)
    {
        srcrect->x -= dstrect->x;
        srcrect->w += dstrect->x;
        dstrect->x = 0;
    }

    if (dstrect->y < 0)
    {
        srcrect->y -= dstrect->y;
        srcrect->h += dstrect->y;
        dstrect->y = 0;
    }

    // now lower left hand corner
    if (dstrect->x + srcrect->w > dst->w)
    {
        //srcrect->x -= dstrect->x;
        srcrect->w = dst->w - dstrect->x;
        dstrect->w = dst->w;
    }

    if (dstrect->y + srcrect->h > dst->h)
    {
        srcrect->h = dst->h - dstrect->y;
        dstrect->h = dst->h;
    }

    // compute starting address in dst surface
    bpp = dst->format->BytesPerPixel;
    Uint32 *dst_buffer = (Uint32 *)((Uint8 *)dst->pixels + dstrect->y * dst->pitch + dstrect->x * bpp);

    // compute starting address in src surface to scan data from
    bpp = src->format->BytesPerPixel;
    Uint32 *src_bitmap = (Uint32 *)((Uint8 *)src->pixels + srcrect->y * src->pitch + srcrect->x * bpp);

    for(y = 0; y < srcrect->h; y++)
    {
        x2 = 0;
        for(x = 0; x < srcrect->w; x++)
        {
            pixel = src_bitmap[x];
            if(pixel != alpha_pixel)
                dst_buffer[x2] = color;
            x2++;
        }
        dst_buffer += dst->w;
        src_bitmap += src->w;
    }
}

void blit_blended(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, Uint8 alpha)
{
    int x, y, bpp;
    Uint32 pixel, alpha_pixel = src->format->colorkey;
    Uint8 r1, g1, b1, r2, g2, b2;
    float alpha_ratio1 = (float) alpha / 0XFF;
    float alpha_ratio2 = 1 - alpha_ratio1;

    // first do trivial rejections of bitmap, is it totally invisible?
    if ((dstrect->x >= dst->w) || (dstrect->y >= dst->h) ||
       ((dstrect->x + srcrect->w) <= 0) || ((dstrect->y + srcrect->h) <= 0))
            return;

    // clip rectangles
    // upper left hand corner first
    if (dstrect->x < 0)
    {
        srcrect->x -= dstrect->x;
        srcrect->w += dstrect->x;
        dstrect->x = 0;
    }

    if (dstrect->y < 0)
    {
        srcrect->y -= dstrect->y;
        srcrect->h += dstrect->y;
        dstrect->y = 0;
    }

    // now lower left hand corner
    if (dstrect->x + srcrect->w > dst->w)
    {
        //srcrect->x -= dstrect->x;
        srcrect->w = dst->w - dstrect->x;
        dstrect->w = dst->w;
    }

    if (dstrect->y + srcrect->h > dst->h)
    {
        srcrect->h = dst->h - dstrect->y;
        dstrect->h = dst->h;
    }

    // compute starting address in dst surface
    bpp = dst->format->BytesPerPixel;
    Uint32 *dst_buffer = (Uint32 *)((Uint8 *)dst->pixels + dstrect->y * dst->pitch + dstrect->x * bpp);

    // compute starting address in src surface to scan data from
    bpp = src->format->BytesPerPixel;
    Uint32 *src_bitmap = (Uint32 *)((Uint8 *)src->pixels + srcrect->y * src->pitch + srcrect->x * bpp);

    for(y = 0; y < srcrect->h; y++)
    {
        for(x = 0; x < srcrect->w; x++)
        {
            pixel = src_bitmap[x];
            if(pixel != alpha_pixel)
            {
                UNPACK_RGB(r1, g1, b1, src_bitmap[x]);
                UNPACK_RGB(r2, g2, b2, dst_buffer[x]);
                r1 *= alpha_ratio1; g1 *= alpha_ratio1; b1 *= alpha_ratio1;
                r2 *= alpha_ratio2; g2 *= alpha_ratio2; b2 *= alpha_ratio2;
                dst_buffer[x] = PACK_RGB((r1 + r2), (g1 + g2), (b1 + b2));
            }
        }
        dst_buffer += dst->w;
        src_bitmap += src->w;
    }
}

void draw_rect(SDL_Surface *dst, int x, int y, int w, int h, Uint32 color, Uint8 thickness)
{
    int line_start, line_end, i, t, x_buff, y_buff;
    int bpp = dst->format->BytesPerPixel;
    int inc = dst->pitch >> 2; //dst->w
    Uint32 *dst_buffer, *old_dst_buffer;

    if(y >= 0 && y <= dst->h)
    {
        line_start = x;  line_end = x + w;
        if(line_end > 0 && line_start < dst->w)
        {
            if(line_start < 0) line_start = 0;
            if(line_end > dst->w) line_end = dst->w;
            dst_buffer = (Uint32 *)((Uint8 *)dst->pixels + y * dst->pitch + line_start * bpp);
            for(t = 0; t < thickness; t++)
            {
                for(i = 0; i < line_end - line_start; i++)
                {
                    dst_buffer[i] = color;
                }
                if((y + t) > dst->h) break;
                dst_buffer += inc;
            }
        }
    }

    if((y + h) >= 0 && (y + h) <= dst->h)
    {
        line_start = x;  line_end = x + w;
        if(line_end > 0 && line_start < dst->w)
        {
            if(line_start < 0) line_start = 0;
            if(line_end > dst->w) line_end = dst->w;
            y_buff = y + h - thickness;
            if(y_buff < 0) y_buff = 0;
            dst_buffer = (Uint32 *)((Uint8 *)dst->pixels + y_buff * dst->pitch + line_start * bpp);
            for(t = 0; t < thickness; t++)
            {
                for(i = 0; i < line_end - line_start; i++)
                {
                    dst_buffer[i] = color;
                }
                if((y + t) > dst->h) break;
                dst_buffer += inc;
            }
        }
    }

    if(x >= 0 && x <= dst->w)
    {
        line_start = y;  line_end = y + h;
        if(line_end > 0 && line_start < dst->h)
        {
            if(line_start < 0) line_start = 0;
            if(line_end > dst->h) line_end = dst->h;
            dst_buffer = old_dst_buffer = (Uint32 *)((Uint8 *)dst->pixels + line_start * dst->pitch + x * bpp);
            for(t = 0; t < thickness; t++)
            {
                if((x + t) > dst->w) break;
                dst_buffer = old_dst_buffer + t;
                for(i = 0; i < line_end - line_start; i++)
                {
                    *dst_buffer = color;
                    dst_buffer += inc;
                }
            }
        }
    }

    if((x + w) >= 0 && (x + w) <= dst->w)
    {
        line_start = y;  line_end = y + h;
        if(line_end > 0 && line_start < dst->h)
        {
            if(line_start < 0) line_start = 0;
            if(line_end > dst->h) line_end = dst->h;
            x_buff = x + w - thickness;
            if(x_buff < 0) x_buff = 0;
            dst_buffer = old_dst_buffer = (Uint32 *)((Uint8 *)dst->pixels + line_start * dst->pitch + x_buff * bpp);
            for(t = 0; t < thickness; t++)
            {
                if((x + t) > dst->w) break;
                dst_buffer = old_dst_buffer + t;
                for(i = 0; i < line_end - line_start; i++)
                {
                    *dst_buffer = color;
                    dst_buffer += inc;
                }
            }
        }
    }
}

