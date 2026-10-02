#ifndef __FONT_H__
#define __FONT_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

/* Bitmap font functions */
void draw_char(SDL_Surface *surface, unsigned char symbol, int x, int y, unsigned short color, int embedded_font_name);
void draw_string(SDL_Surface *surface, const char *text, int x, int y, unsigned short color, int embedded_font_name);
int get_embedded_font_char_width(int embedded_font_name);
int get_embedded_font_char_height(int embedded_font_name);

/* TTF font functions */
int init_ttf_font(const char *font_path, int font_size, int font_shaded);
void cleanup_ttf_font(void);
void draw_string_ttf(SDL_Surface *surface, const char *text, int x, int y, SDL_Color fg, SDL_Color bg);
void draw_string_ttf_with_linebreak(SDL_Surface *surface, const char *text, int x, int y, SDL_Color fg, SDL_Color bg);
int get_ttf_char_width(void);
int get_ttf_char_height(void);
int is_ttf_loaded(void);
int init_osk_ttf_font(const char *font_path, int font_size, int shade);
int is_osk_ttf_loaded(void);
int get_osk_ttf_char_width(void);
int get_osk_ttf_char_height(void);
SDL_Surface *render_osk_ttf_text(const char *text, SDL_Color fg, SDL_Color bg);
SDL_Surface *render_term_ttf_text(const char *text, SDL_Color fg, SDL_Color bg);
void draw_string_osk_ttf(SDL_Surface *surface, const char *text, int x, int y, SDL_Color fg, SDL_Color bg);
int pick_osk_ttf_font(const char *font_path, int max_w, int max_h);

#endif
