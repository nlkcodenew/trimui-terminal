#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_ttf.h>
#include <ctype.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <locale.h>
#include <pty.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "font.h"
#include "keyboard.h"
#include "vt100.h"

#define USAGE "Trimui Terminal\nusage: simple-terminal [-h] [-scale 2.0] [-font font.ttf] [-fontsize 14] [-fontshade 0|1|2] [-rotate 0|90|180|270] [-o file] [-q] [-r command ...]\n"

/* Arbitrary sizes */
#define DRAW_BUF_SIZ 20 * 1024

#define REDRAW_TIMEOUT (80 * 1000) /* 80 ms */

/* macros */
#define TIMEDIFF(t1, t2) ((t1.tv_sec - t2.tv_sec) * 1000 + (t1.tv_usec - t2.tv_usec) / 1000)

enum WindowState { WIN_VISIBLE = 1, WIN_REDRAW = 2, WIN_FOCUSED = 4 };

/* Purely graphic info */
typedef struct {
    // Colormap cmap;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Surface *surface;
    int width, height;           /* window width and height */
    int tty_width, tty_height;   /* tty width and height */
    int char_height, char_width; /* char height and width */
    char state;                  /* focus, redraw, visible */
} MainWindow;

/* Drawing Context */
typedef struct {
    SDL_Color colors[LEN(colormap) < 256 ? 256 : LEN(colormap)];
    // TTF_Font *font, *ifont, *bfont, *ibfont;
} DrawingContext;

/*
 * Special keys (change & recompile accordingly)
 * Keep in mind that kpress() in main.c hardcodes some keys.
 * Mask value:
 * * Use XK_NO_MOD to match the key alone (no modifiers)
 */
#define XK_ANY_MOD 0

typedef struct {
    SDL_Keycode k;        // key
    Uint16 mask;          // modifier mask: KMOD_ALT, KMOD_CTRL, KMOD_SHIFT, KMOD_CAPS, ...
    char s[ESC_BUF_SIZ];  // output string
} NonPrintingKeyboardKey;

static NonPrintingKeyboardKey non_printing_keyboard_keys[] = {
    {SDLK_ESCAPE, XK_ANY_MOD, "\033"},       // Escape
    {SDLK_TAB, KMOD_LSHIFT, "\033[Z"},       // Shift+Tab
    {SDLK_TAB, KMOD_RSHIFT, "\033[Z"},       // Shift+Tab
    {SDLK_TAB, XK_ANY_MOD, "\t"},            // Tab
    {SDLK_RETURN, KMOD_LALT, "\033\r"},      // Alt+Enter
    {SDLK_RETURN, KMOD_RALT, "\033\r"},      // Alt+Enter
    {SDLK_RETURN, XK_ANY_MOD, "\r"},         // Enter
    {SDLK_LEFT, XK_ANY_MOD, "\033[D"},       // Left
    {SDLK_RIGHT, XK_ANY_MOD, "\033[C"},      // Right
    {SDLK_UP, XK_ANY_MOD, "\033[A"},         // Up
    {SDLK_DOWN, XK_ANY_MOD, "\033[B"},       // Down
    {SDLK_BACKSPACE, XK_ANY_MOD, "\177"},    // Backspace
    {SDLK_HOME, XK_ANY_MOD, "\033[1~"},      // Home
    {SDLK_INSERT, XK_ANY_MOD, "\033[2~"},    // Insert
    {SDLK_DELETE, XK_ANY_MOD, "\033[3~"},    // Delete
    {SDLK_END, XK_ANY_MOD, "\033[4~"},       // End
    {SDLK_PAGEUP, XK_ANY_MOD, "\033[5~"},    // Page Up
    {SDLK_PAGEDOWN, XK_ANY_MOD, "\033[6~"},  // Page Down
    {SDLK_F1, XK_ANY_MOD, "\033OP"},         // F1
    {SDLK_F2, XK_ANY_MOD, "\033OQ"},         // F2
    {SDLK_F3, XK_ANY_MOD, "\033OR"},         // F3
    {SDLK_F4, XK_ANY_MOD, "\033OS"},         // F4
    {SDLK_F5, XK_ANY_MOD, "\033[15~"},       // F5
    {SDLK_F6, XK_ANY_MOD, "\033[17~"},       // F6
    {SDLK_F7, XK_ANY_MOD, "\033[18~"},       // F7
    {SDLK_F8, XK_ANY_MOD, "\033[19~"},       // F8
    {SDLK_F9, XK_ANY_MOD, "\033[20~"},       // F9
    {SDLK_F10, XK_ANY_MOD, "\033[21~"},      // F10
    {SDLK_F11, XK_ANY_MOD, "\033[23~"},      // F11
    {SDLK_F12, XK_ANY_MOD, "\033[24~"},      // F12
};

/* SDL Surfaces */
SDL_Surface *screen;
SDL_Surface *osk_screen;
SDL_Surface *rotated_screen;   // final frame matching window size

static void draw(void);
static void draw_region(int, int, int, int);
static void draw_scrollbar(void);
static void main_loop(void);
int tty_thread(void *unused);

static void x_draws(char *, Glyph, int, int, int, int);
static void x_clear(int, int, int, int);
static void x_draw_cursor(void);
static void sdl_init(void);
static void create_tty_thread();
static void init_color_map(void);
static void sdl_term_clear(int, int, int, int);
static void x_resize(int, int);
static void scale_to_size(int, int);
static char *k_map(SDL_Keycode, Uint16);
static void k_press(SDL_Event *);
static void text_input(SDL_Event *);
static void window_event_handler(SDL_Event *);

static void update_render(void);
static Uint32 clear_popup_timer(Uint32 interval, void *param);
int trimui_ticks_ms(void);
void trimui_request_quit(void);
void trimui_show_quit_confirm(void);
void trimui_hide_quit_confirm(void);

static void (*event_handler[SDL_LASTEVENT])(SDL_Event *) = {[SDL_KEYDOWN] = k_press, [SDL_TEXTINPUT] = text_input, [SDL_WINDOWEVENT] = window_event_handler};

/* Globals */
static DrawingContext drawing_ctx;
static MainWindow main_window;
static SDL_Joystick *joystick;

SDL_Thread *thread = NULL;

char **opt_cmd = NULL;
int opt_cmd_size = 0;
char *opt_io = NULL;

static int embedded_font_name = 1;  // 1 or 2
static volatile int thread_should_exit = 0;
static volatile int tty_thread_done = 0;
static volatile int tty_data_pending = 0; /* tty co output moi -> main ve lai */
static int shutdown_called = 0;
/* OTA badge: chu "Dang cap nhat..." goc phai khi ota-update.sh dang chay nen */
static SDL_Surface *ota_badge = NULL;
static char ota_badge_text[64] = "";
static Uint32 ota_last_poll = 0;
static int ota_done_shown = 0;
static SDL_Surface *ver_label = NULL; /* nhan version nho goc phai man hinh terminal */
/* Version hien thi: doc file VERSION (ota-update.sh ghi lai sau moi lan cap nhat
   OTA) va fallback ve -DVERSION luc bien dich. Truoc day chi dung -DVERSION nen
   nhan luon giu version cu trong khi app da cap nhat xong. */
char app_version[32] = "";
const char *terminal_version(void);
static void load_app_version(void) {
    FILE *vf = fopen("VERSION", "r");
    if (vf) {
        char buf[32];
        if (fgets(buf, sizeof(buf), vf)) {
            buf[strcspn(buf, "\r\n")] = '\0';
            snprintf(app_version, sizeof(app_version), "%s", buf);
        }
        fclose(vf);
    }
    if (app_version[0] == '\0') {
#ifdef VERSION
        snprintf(app_version, sizeof(app_version), "%s", VERSION);
#else
        snprintf(app_version, sizeof(app_version), "%s", "0.0.0");
#endif
    }
}
static void trimui_free_popup_cache(void);
static void trimui_free_ota_badge(void);
extern volatile int trimui_thread_should_exit;
extern volatile int trimui_child_exited;

char popup_message[256];

size_t x_write(int fd, char *s, size_t len) {
    size_t aux = len;

    while (len > 0) {
        ssize_t r = write(fd, s, len);
        if (r < 0) return r;
        len -= r;
        s += r;
    }
    return aux;
}

void *x_malloc(size_t len) {
    void *p = malloc(len);
    if (!p) die("Out of memory\n");
    return p;
}

void *x_realloc(void *p, size_t len) {
    if ((p = realloc(p, len)) == NULL) die("Out of memory\n");
    return p;
}

void *x_calloc(size_t nmemb, size_t size) {
    void *p = calloc(nmemb, size);
    if (!p) die("Out of memory\n");
    return p;
}

static const char *trimui_system_fonts[] = {
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
    "assets/fallback.ttf",
    NULL
};
void sdl_load_fonts() {
    if (opt_font && init_ttf_font(opt_font, opt_fontsize > 0 ? opt_fontsize : 16, opt_fontshade)) {
        main_window.char_width = get_ttf_char_width();
        main_window.char_height = get_ttf_char_height();
        fprintf(stderr, "TTF font user: %dx%d\n", main_window.char_width, main_window.char_height);
    } else {
#if defined(BR2) && !defined(RPI)
        int term_size = 16;
        const char **fp = trimui_system_fonts;
        int ok = 0;
        while (*fp) {
            if (init_ttf_font(*fp, term_size, opt_fontshade)) {
                main_window.char_width = get_ttf_char_width();
                main_window.char_height = get_ttf_char_height();
                fprintf(stderr, "TTF font auto (term): %s size %d -> %dx%d\n", *fp, term_size, main_window.char_width, main_window.char_height);
                ok = 1;
                break;
            }
            fp++;
        }
        if (!ok) {
            main_window.char_width = get_embedded_font_char_width(embedded_font_name);
            main_window.char_height = get_embedded_font_char_height(embedded_font_name);
            fprintf(stderr, "Using embedded bitmap font %d (%dx%d)\n", embedded_font_name, main_window.char_width, main_window.char_height);
        }
#else
        main_window.char_width = get_embedded_font_char_width(embedded_font_name);
        main_window.char_height = get_embedded_font_char_height(embedded_font_name);
        fprintf(stderr, "Using embedded bitmap font %d (%dx%d)\n", embedded_font_name, main_window.char_width, main_window.char_height);
#endif
    }
#if defined(BR2) && !defined(RPI)
    {
        int osk_max_w = main_window.width > 0 ? main_window.width - 16 : 1264;
        int osk_max_h = (main_window.height > 0 ? main_window.height : 720) * 55 / 100;
        int osk_size = 0;
        const char *osk_path = NULL;
        const char **fp = trimui_system_fonts;
        while (*fp) {
            int got = pick_osk_ttf_font(*fp, osk_max_w, osk_max_h);
            if (got > 0) {
                osk_size = got;
                osk_path = *fp;
                break;
            }
            fp++;
        }
        if (osk_size > 0)
            fprintf(stderr, "TTF font auto (osk): %s size %d -> %dx%d (max %dx%d)\n", osk_path, osk_size, get_osk_ttf_char_width(), get_osk_ttf_char_height(), osk_max_w, osk_max_h);
    }
#endif
}

int trimui_ticks_ms(void) { return (int)SDL_GetTicks(); }
void trimui_request_quit(void) {
    SDL_Event q; q.type = SDL_QUIT;
    SDL_PushEvent(&q);
}
void trimui_show_quit_confirm(void) {
    snprintf(popup_message, sizeof(popup_message), "BẤM B LẦN NỮA ĐỂ THOÁT|A ĐỂ HỦY");
    SDL_AddTimer(4000, clear_popup_timer, NULL);
}
void trimui_hide_quit_confirm(void) {
    popup_message[0] = '\0';
}
void sdl_shutdown(void) {
    if (SDL_WasInit(SDL_INIT_EVERYTHING) != 0 && !shutdown_called) {
        shutdown_called = 1;
        fprintf(stderr, "Đang thoát Terminal\n");
        /* Chan SIGCHLD handler tu dong exit (se deadlock voi WaitThread). */
        signal(SIGCHLD, SIG_DFL);
        thread_should_exit = 1;
        trimui_thread_should_exit = 1;
        trimui_kill_shell();
        if (thread) {
            /* Cho toi da ~2s, khong doi vo han (fix treo B lan 2). */
            int waited = 0;
            while (!tty_thread_done && waited < 2000) {
                SDL_Delay(50);
                waited += 50;
            }
            if (tty_thread_done) {
                SDL_WaitThread(thread, NULL);
            } else {
                fprintf(stderr, "Luồng tty kẹt sau 2s, bỏ qua chờ (thoát sẽ thu hồi)\n");
            }
            thread = NULL;
        }
        extern int cmdfd;
        if (cmdfd >= 0) { close(cmdfd); cmdfd = -1; }

        // Cleanup TTF font
        cleanup_ttf_font();

        if (main_window.surface) SDL_FreeSurface(main_window.surface);
        if (osk_screen) SDL_FreeSurface(osk_screen);
        if (rotated_screen) SDL_FreeSurface(rotated_screen);
        if (ota_badge) SDL_FreeSurface(ota_badge);
        ota_badge = NULL;
        if (ver_label) SDL_FreeSurface(ver_label);
        ver_label = NULL;
        trimui_free_popup_cache();
        main_window.surface = NULL;
        SDL_JoystickClose(joystick);
        SDL_Quit();
    }
}

void window_event_handler(SDL_Event *event) {
#ifdef BR2
    return;  // no resize for BR2 handheld devices builds because of kms video driver
#endif
    switch (event->window.event) {
        case SDL_WINDOWEVENT_RESIZED:
            scale_to_size(event->window.data1, event->window.data2);
            break;
        default:
            break;
    }
}

void scale_to_size(int width, int height) {
    if (width <= 0 || height <= 0 || width > 8192 || height > 8192) return;
    main_window.width = width;
    main_window.height = height;
    printf("Set scale to size: %dx%d (x%.1f)\n", main_window.width, main_window.height, opt_scale);

    // Recreate texture for new size
    if (main_window.texture) {
        SDL_DestroyTexture(main_window.texture);
    }
    main_window.texture = SDL_CreateTexture(main_window.renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, main_window.width, main_window.height);
    if (!main_window.texture) {
        fprintf(stderr, "Unable to recreate texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Recreate surfaces
    if (main_window.surface) SDL_FreeSurface(main_window.surface);
    int compose_w = (opt_rotate == 90 || opt_rotate == 270) ? main_window.height : main_window.width;
    int compose_h = (opt_rotate == 90 || opt_rotate == 270) ? main_window.width : main_window.height;
    main_window.surface = SDL_CreateRGBSurface(0, compose_w, compose_h, 16, 0xF800, 0x7E0, 0x1F, 0);  // compose buffer
    if (osk_screen) SDL_FreeSurface(osk_screen);
    osk_screen = SDL_CreateRGBSurface(0, compose_w, compose_h, 16, 0xF800, 0x7E0, 0x1F, 0);          // compose + keyboard
    if (rotated_screen) SDL_FreeSurface(rotated_screen);
    rotated_screen = SDL_CreateRGBSurface(0, main_window.width, main_window.height, 16, 0xF800, 0x7E0, 0x1F, 0);      // final frame

    // Recreate screen surface for compatibility
    if (screen) SDL_FreeSurface(screen);
    screen = SDL_CreateRGBSurface(0, 640, 480, 16, 0xF800, 0x7E0, 0x1F, 0);

    // resize terminal to fit content buffer (which may be swapped for 90/270)
    int col, row;
    int content_w = main_window.surface ? main_window.surface->w : main_window.width;
    int content_h = main_window.surface ? main_window.surface->h : main_window.height;
    col = (content_w - 2 * borderpx) / main_window.char_width;
    row = (content_h - 2 * borderpx) / main_window.char_height;
    t_resize(col, row);
    x_resize(col, row);
    tty_resize();
}

void sdl_init(void) {
    fprintf(stderr, "SDL init\n");

    load_app_version();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

#ifdef BR2
    SDL_ShowCursor(0);
#endif
    SDL_StartTextInput();

    /* colors */
    init_color_map();

    int display_index = 0;  // usually 0 unless you have multiple screens
    SDL_DisplayMode mode;
    if (SDL_GetCurrentDisplayMode(display_index, &mode) != 0) {
        printf("SDL_GetCurrentDisplayMode failed: %s\n", SDL_GetError());
        main_window.width = initial_width;
        main_window.height = initial_height;
    } else {
        printf("Detected screen: %dx%d @ %dHz\n", mode.w, mode.h, mode.refresh_rate);
        main_window.width = mode.w;
        main_window.height = mode.h;
#ifndef BR2
        main_window.width = initial_width * 2;
        main_window.height = initial_height * 2;
#endif
        printf("Setting resolution to: %dx%d\n", main_window.width, main_window.height);
    }

    /* font: load SAU khi da biet kich thuoc man hinh that de OSK
       pick dung co full-width (truoc day load khi width=0 nen sai). */
    sdl_load_fonts();

    main_window.window = SDL_CreateWindow("Trimui Terminal", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, main_window.width, main_window.height, SDL_WINDOW_SHOWN);
    if (!main_window.window) {
        fprintf(stderr, "Unable to create window: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    main_window.renderer = SDL_CreateRenderer(main_window.window, -1, SDL_RENDERER_ACCELERATED);
    if (!main_window.renderer) {
        main_window.renderer = SDL_CreateRenderer(main_window.window, -1, SDL_RENDERER_SOFTWARE);
        if (!main_window.renderer) {
            fprintf(stderr, "Unable to create renderer: %s\n", SDL_GetError());
            exit(EXIT_FAILURE);
        }
    }

    // SDL_RenderSetLogicalSize(main_window.renderer, main_window.width, main_window.height);

    main_window.texture = SDL_CreateTexture(main_window.renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, main_window.width, main_window.height);
    if (!main_window.texture) {
        fprintf(stderr, "Unable to create texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    int compose_w = (opt_rotate == 90 || opt_rotate == 270) ? main_window.height : main_window.width;
    int compose_h = (opt_rotate == 90 || opt_rotate == 270) ? main_window.width : main_window.height;
    main_window.surface = SDL_CreateRGBSurface(0, compose_w, compose_h, 16, 0xF800, 0x7E0, 0x1F, 0);  // console screen
    osk_screen = SDL_CreateRGBSurface(0, compose_w, compose_h, 16, 0xF800, 0x7E0, 0x1F, 0);           // for keyboard mix
    rotated_screen = SDL_CreateRGBSurface(0, main_window.width, main_window.height, 16, 0xF800, 0x7E0, 0x1F, 0);        // final frame after rotation

    // Create a temporary surface for the screen to maintain compatibility
    screen = SDL_CreateRGBSurface(0, main_window.width, main_window.height, 16, 0xF800, 0x7E0, 0x1F, 0);

    main_window.state |= WIN_VISIBLE | WIN_REDRAW;

    joystick = SDL_JoystickOpen(0);
}

void create_tty_thread() {
    // TODO: might need to use system threads
    if (!(thread = SDL_CreateThread(tty_thread, "ttythread", NULL))) {
        fprintf(stderr, "Unable to create thread: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

/* Panel thong bao to, can giua man hinh (vd xac nhan thoat).
   popup_message co the chua '|' de tach 2 dong. Chu duoc render 1 lan roi
   cache (dung kich thuoc that cua surface chu, khong uoc luong bang strlen
   vi tieng Viet co dau so byte != so ky tu). */
static char popup_cache_key[256] = "";
static SDL_Surface *popup_line1 = NULL;
static SDL_Surface *popup_line2 = NULL;
static void trimui_free_popup_cache(void) {
    if (popup_line1) SDL_FreeSurface(popup_line1);
    if (popup_line2) SDL_FreeSurface(popup_line2);
    popup_line1 = popup_line2 = NULL;
    popup_cache_key[0] = '\0';
}
static void trimui_ensure_popup_cache(void) {
    if (strcmp(popup_message, popup_cache_key) == 0) return;
    trimui_free_popup_cache();
    if (!popup_message[0]) return;
    snprintf(popup_cache_key, sizeof(popup_cache_key), "%s", popup_message);
    char l1[200], l2[200];
    const char *sep = strchr(popup_message, '|');
    if (sep) {
        size_t n1 = (size_t)(sep - popup_message);
        if (n1 >= sizeof(l1)) n1 = sizeof(l1) - 1;
        memcpy(l1, popup_message, n1);
        l1[n1] = '\0';
        snprintf(l2, sizeof(l2), "%s", sep + 1);
    } else {
        snprintf(l1, sizeof(l1), "%s", popup_message);
        l2[0] = '\0';
    }
    SDL_Color fg = {255, 255, 128, 255};
    SDL_Color bg = {0, 0, 0, 255};
    if (is_osk_ttf_loaded()) {
        popup_line1 = render_osk_ttf_text(l1, fg, bg);
        if (l2[0]) popup_line2 = render_osk_ttf_text(l2, fg, bg);
    } else if (is_ttf_loaded()) {
        popup_line1 = render_term_ttf_text(l1, fg, bg);
        if (l2[0]) popup_line2 = render_term_ttf_text(l2, fg, bg);
    }
    /* Khong TTF: giu NULL, draw_popup_box se ve bitmap truc tiep. */
}
static void draw_popup_box(SDL_Surface *s) {
    trimui_ensure_popup_cache();
    if (!is_osk_ttf_loaded() && !is_ttf_loaded()) {
        /* Du phong bitmap (hiem): chu nho nhu cu. */
        SDL_Rect rect = {borderpx, s->h / 2 - 4, s->w - borderpx * 2, 14};
        SDL_FillRect(s, &rect, SDL_MapRGB(s->format, 128, 128, 128));
        char flat[256];
        snprintf(flat, sizeof(flat), "%s", popup_message);
        for (char *p = flat; *p; p++) {
            if (*p == '|') *p = ' ';
        }
        draw_string(s, flat, rect.x + 2, rect.y + 4, SDL_MapRGB(s->format, 255, 255, 128), embedded_font_name);
        return;
    }
    if (!popup_line1) return;
    int w1 = popup_line1->w, h1 = popup_line1->h;
    int w2 = popup_line2 ? popup_line2->w : 0;
    int h2 = popup_line2 ? popup_line2->h : 0;
    int maxw = w1 > w2 ? w1 : w2;
    int w = maxw + 48;
    if (w > s->w - 16) w = s->w - 16;
    if (w < 200) w = 200;
    int h = h1 + h2 + 36;
    if (popup_line2) h += 6;
    int x = (s->w - w) / 2;
    if (x < 8) x = 8;
    int y = s->h * 28 / 100;
    if (y + h > s->h) y = s->h - h - 8;
    if (y < 8) y = 8;
    /* vien vang + nen den cho noi bat */
    SDL_Rect outer = {x, y, w, h};
    SDL_Rect inner = {x + 3, y + 3, w - 6, h - 6};
    SDL_FillRect(s, &outer, SDL_MapRGB(s->format, 255, 255, 128));
    SDL_FillRect(s, &inner, SDL_MapRGB(s->format, 0, 0, 0));
    SDL_Rect d1 = {x + (w - w1) / 2, y + 18, w1, h1};
    SDL_BlitSurface(popup_line1, NULL, s, &d1);
    if (popup_line2) {
        SDL_Rect d2 = {x + (w - w2) / 2, y + 18 + h1 + 6, w2, h2};
        SDL_BlitSurface(popup_line2, NULL, s, &d2);
    }
}

/* OTA: doc file .ota-status (do ota-update.sh ghi khi chay nen) de hien
   badge "Dang tai..." goc phai + popup bao khi cap nhat xong. */
static void trimui_free_ota_badge(void) {
    if (ota_badge) SDL_FreeSurface(ota_badge);
    ota_badge = NULL;
    ota_badge_text[0] = '\0';
}
static void trimui_set_ota_badge(const char *txt) {
    if (ota_badge && strcmp(txt, ota_badge_text) == 0) return;
    trimui_free_ota_badge();
    snprintf(ota_badge_text, sizeof(ota_badge_text), "%s", txt);
    SDL_Color fg = {128, 255, 128, 255};
    SDL_Color bg = {0, 0, 0, 255};
    if (is_osk_ttf_loaded())
        ota_badge = render_osk_ttf_text(txt, fg, bg);
    else if (is_ttf_loaded())
        ota_badge = render_term_ttf_text(txt, fg, bg);
    /* Khong TTF: bo badge, khong bao loi. */
}
static void trimui_poll_ota(void) {
    Uint32 now = SDL_GetTicks();
    if (now - ota_last_poll < 1000) return;
    ota_last_poll = now;
    FILE *f = fopen(".ota-status", "r");
    if (!f) {
        /* ota-update.sh xoa .ota-status khi xong (hoac chua chay). Badge "Dang
           kiem tra..." phai bien mat ngay, nguoi dung truoc day thay no dinh
           o goc phai ma khong co gi chay. */
        trimui_free_ota_badge();
        return;
    }
    char st[128];
    if (!fgets(st, sizeof(st), f)) {
        fclose(f);
        return;
    }
    fclose(f);
    st[strcspn(st, "\r\n")] = '\0';
    if (strncmp(st, "done ", 5) == 0) {
        const char *ver = st + 5;
        /* Cap nhat that bai -> thong bao "mo lai app". Version dang chay lay
           tu file VERSION, nen so sanh bang chinh no. */
        int is_new = (strcmp(ver, app_version) != 0);
        if (is_new && !ota_done_shown) {
            ota_done_shown = 1;
            snprintf(popup_message, sizeof(popup_message), "ĐÃ CẬP NHẬT LÊN v%s|MỞ LẠI APP ĐỂ DÙNG", ver);
            SDL_AddTimer(12000, clear_popup_timer, NULL);
        }
        remove(".ota-status");
        trimui_free_ota_badge();
    } else if (strncmp(st, "downloading ", 12) == 0) {
        char b[80];
        snprintf(b, sizeof(b), "Đang tải %s...", st + 12);
        trimui_set_ota_badge(b);
    } else if (strcmp(st, "checking") == 0) {
        trimui_set_ota_badge("Đang kiểm tra...");
    } else {
        /* failed...: xoa lang, chi tiet xem Terminal-ota.log */
        remove(".ota-status");
        trimui_free_ota_badge();
    }
}

void update_render(void) {
    if (main_window.surface == NULL) return;
    // printf("Updating render\n");

    memcpy(osk_screen->pixels, main_window.surface->pixels, main_window.surface->w * main_window.surface->h * 2);
    if (popup_message[0] != '\0') {
        draw_popup_box(osk_screen);
    }
    draw_keyboard(osk_screen);  // osk_screen(SW) = console + keyboard
/* Nhan version thuong truc goc tren-phai vung terminal. */
    if (!ver_label && is_ttf_loaded() && app_version[0]) {
        char vt[32];
        snprintf(vt, sizeof(vt), "v%s", app_version);
        ver_label = render_term_ttf_text(vt, (SDL_Color){150, 150, 150, 255}, (SDL_Color){0, 0, 0, 255});
    }
    if (ver_label) {
        SDL_Rect vd = {osk_screen->w - ver_label->w - 6, 4, ver_label->w, ver_label->h};
        SDL_Rect vbg = {vd.x - 2, vd.y - 1, vd.w + 4, vd.h + 2};
        SDL_FillRect(osk_screen, &vbg, SDL_MapRGB(osk_screen->format, 0, 0, 0));
        SDL_BlitSurface(ver_label, NULL, osk_screen, &vd);
    }
    if (ota_badge) { /* badge OTA ngay duoi nhan version */
        int by = ver_label ? 4 + ver_label->h + 4 : 8;
        SDL_Rect bd = {osk_screen->w - ota_badge->w - 8, by, ota_badge->w, ota_badge->h};
        SDL_Rect bgrect = {bd.x - 4, bd.y - 3, bd.w + 8, bd.h + 6};
        SDL_FillRect(osk_screen, &bgrect, SDL_MapRGB(osk_screen->format, 0, 0, 0));
        SDL_BlitSurface(ota_badge, NULL, osk_screen, &bd);
    }
    // Update texture with screen pixels and render
    SDL_RenderClear(main_window.renderer);
    if (opt_rotate == 90 || opt_rotate == 270) {
        // Ensure rotated_screen matches window size
        if (!rotated_screen || rotated_screen->w != main_window.width || rotated_screen->h != main_window.height) {
            if (rotated_screen) SDL_FreeSurface(rotated_screen);
            rotated_screen = SDL_CreateRGBSurface(0, main_window.width, main_window.height, 16, 0xF800, 0x7E0, 0x1F, 0);
        }

        // Rotate osk_screen into rotated_screen
        SDL_LockSurface(osk_screen);
        SDL_LockSurface(rotated_screen);
        int sw = osk_screen->w, sh = osk_screen->h;
        int dpw = rotated_screen->w, dph = rotated_screen->h; // dpw=window width, dph=window height
        Uint16 *sdata = (Uint16 *)osk_screen->pixels;
        Uint16 *ddata = (Uint16 *)rotated_screen->pixels;
        int spitch = osk_screen->pitch / 2;
        int dpitch = rotated_screen->pitch / 2;
        if (opt_rotate == 90) {
            // source (sw=H, sh=W) -> dest (dpw=W, dph=H)
            for (int y = 0; y < sh; y++) {
                for (int x = 0; x < sw; x++) {
                    int dx = dpw - 1 - y;
                    int dy = x;
                    ddata[dy * dpitch + dx] = sdata[y * spitch + x];
                }
            }
        } else { // 270 degrees
            for (int y = 0; y < sh; y++) {
                for (int x = 0; x < sw; x++) {
                    int dx = y;
                    int dy = dph - 1 - x;
                    ddata[dy * dpitch + dx] = sdata[y * spitch + x];
                }
            }
        }
        SDL_UnlockSurface(rotated_screen);
        SDL_UnlockSurface(osk_screen);
        SDL_UpdateTexture(main_window.texture, NULL, rotated_screen->pixels, rotated_screen->pitch);
        SDL_RenderCopy(main_window.renderer, main_window.texture, NULL, NULL);
    } else {
        // 0 or 180 degrees: upload and render; 180 uses renderer rotation for speed
        SDL_UpdateTexture(main_window.texture, NULL, osk_screen->pixels, osk_screen->pitch);
        if (opt_rotate == 0) {
            SDL_RenderCopy(main_window.renderer, main_window.texture, NULL, NULL);
        } else { // 180
            SDL_RenderCopyEx(main_window.renderer, main_window.texture, NULL, NULL, 180.0, NULL, SDL_FLIP_NONE);
        }
    }
    SDL_RenderPresent(main_window.renderer);
}

void die(const char *errstr, ...) {
    va_list ap;
    va_start(ap, errstr);
    vfprintf(stderr, errstr, ap);
    va_end(ap);
    sdl_shutdown();
}

void x_resize(int col, int row) {
    main_window.tty_width = MAX(1, 2 * borderpx + col * main_window.char_width);
    main_window.tty_height = MAX(1, 2 * borderpx + row * main_window.char_height);
}

void init_color_map(void) {
    int i, r, g, b;

    // TODO: allow these to override the xterm ones somehow?
    memcpy(drawing_ctx.colors, colormap, sizeof(drawing_ctx.colors));

    /* init colors [16-255] ; same colors as xterm */
    for (i = 16, r = 0; r < 6; r++) {
        for (g = 0; g < 6; g++) {
            for (b = 0; b < 6; b++) {
                drawing_ctx.colors[i].r = r == 0 ? 0 : 0x3737 + 0x2828 * r;
                drawing_ctx.colors[i].g = g == 0 ? 0 : 0x3737 + 0x2828 * g;
                drawing_ctx.colors[i].b = b == 0 ? 0 : 0x3737 + 0x2828 * b;
                i++;
            }
        }
    }

    for (r = 0; r < 24; r++, i++) {
        b = 0x0808 + 0x0a0a * r;
        drawing_ctx.colors[i].r = b;
        drawing_ctx.colors[i].g = b;
        drawing_ctx.colors[i].b = b;
    }
}

void sdl_term_clear(int col1, int row1, int col2, int row2) {
    if (main_window.surface == NULL) return;
    SDL_Rect r = {borderpx + col1 * main_window.char_width, borderpx + row1 * main_window.char_height, (col2 - col1 + 1) * main_window.char_width, (row2 - row1 + 1) * main_window.char_height};
    SDL_Color c = drawing_ctx.colors[IS_SET(MODE_REVERSE) ? defaultfg : defaultbg];
    SDL_FillRect(main_window.surface, &r, SDL_MapRGB(main_window.surface->format, c.r, c.g, c.b));
}

/*
 * Absolute coordinates.
 */
void x_clear(int x1, int y1, int x2, int y2) {
    if (main_window.surface == NULL) return;
    SDL_Rect r = {x1, y1, x2 - x1, y2 - y1};
    SDL_Color c = drawing_ctx.colors[IS_SET(MODE_REVERSE) ? defaultfg : defaultbg];
    SDL_FillRect(main_window.surface, &r, SDL_MapRGB(main_window.surface->format, c.r, c.g, c.b));
}

void x_draws(char *s, Glyph base, int x, int y, int charlen, int bytelen) {
    int winx = borderpx + x * main_window.char_width, winy = borderpx + y * main_window.char_height, width = charlen * main_window.char_width;
    // TTF_Font *font = drawing_ctx.font;
    SDL_Color *fg, *bg, *temp, revfg, revbg;

    // Some programs request >256-color drawing that we don't support. Fall back
    // to defaults in that case.
    if (BETWEEN(base.fg, 0, LEN(drawing_ctx.colors) - 1)) {
      fg = &drawing_ctx.colors[base.fg];
    } else {
      fg = &drawing_ctx.colors[defaultfg];
    }
    if (BETWEEN(base.bg, 0, LEN(drawing_ctx.colors) - 1)) {
      bg = &drawing_ctx.colors[base.bg];
    } else {
      bg = &drawing_ctx.colors[defaultbg];
    }

    s[bytelen] = '\0';

    if (base.mode & ATTR_BOLD) {
        if (BETWEEN(base.fg, 0, 7)) {
            /* basic system colors */
            fg = &drawing_ctx.colors[base.fg + 8];
        } else if (BETWEEN(base.fg, 16, 195)) {
            /* 256 colors */
            fg = &drawing_ctx.colors[base.fg + 36];
        } else if (BETWEEN(base.fg, 232, 251)) {
            /* greyscale */
            fg = &drawing_ctx.colors[base.fg + 4];
        }
        /*
         * Those ranges will not be brightened:
         *	8 - 15 – bright system colors
         *	196 - 231 – highest 256 color cube
         *	252 - 255 – brightest colors in greyscale
         */
        // font = drawing_ctx.bfont;
    }

    /*if(base.mode & ATTR_ITALIC)
        font = drawing_ctx.ifont;
    if((base.mode & ATTR_ITALIC) && (base.mode & ATTR_BOLD))
        font = drawing_ctx.ibfont;*/

    if (IS_SET(MODE_REVERSE)) {
        if (fg == &drawing_ctx.colors[defaultfg]) {
            fg = &drawing_ctx.colors[defaultbg];
        } else {
            revfg.r = ~fg->r;
            revfg.g = ~fg->g;
            revfg.b = ~fg->b;
            fg = &revfg;
        }

        if (bg == &drawing_ctx.colors[defaultbg]) {
            bg = &drawing_ctx.colors[defaultfg];
        } else {
            revbg.r = ~bg->r;
            revbg.g = ~bg->g;
            revbg.b = ~bg->b;
            bg = &revbg;
        }
    }

    if (base.mode & ATTR_REVERSE) temp = fg, fg = bg, bg = temp;

    /* Intelligent cleaning up of the borders. */
    if (x == 0) {
        x_clear(0, (y == 0) ? 0 : winy, borderpx, winy + main_window.char_height + (y == term.row - 1) ? main_window.height : 0);
    }
    if (x + charlen >= term.col - 1) {
        x_clear(winx + width, (y == 0) ? 0 : winy, main_window.width, (y == term.row - 1) ? main_window.height : (winy + main_window.char_height));
    }
    if (y == 0) x_clear(winx, 0, winx + width, borderpx);
    if (y == term.row - 1) x_clear(winx, winy + main_window.char_height, winx + width, main_window.height);

    // SDL_Surface *text_surface;
    SDL_Rect r = {winx, winy, width, main_window.char_height};

    if (main_window.surface != NULL) {
        SDL_FillRect(main_window.surface, &r, SDL_MapRGB(main_window.surface->format, bg->r, bg->g, bg->b));
        // TODO: find a better way to draw cursor box y + 1
        int ys = r.y + 1;
        if (is_ttf_loaded()) {
            // Use TTF rendering
            draw_string_ttf(main_window.surface, s, winx, winy, *fg, *bg);
        } else {
            // Use bitmap rendering
            draw_string(main_window.surface, s, winx, ys, SDL_MapRGB(main_window.surface->format, fg->r, fg->g, fg->b), embedded_font_name);
        }
    }

    if (base.mode & ATTR_UNDERLINE) {
        // r.y += TTF_FontAscent(font) + 1;
        r.y += main_window.char_height;
        r.h = 1;
        if (main_window.surface != NULL) SDL_FillRect(main_window.surface, &r, SDL_MapRGB(main_window.surface->format, fg->r, fg->g, fg->b));
    }
}

void x_draw_cursor(void) {
    static int oldx = 0, oldy = 0;
    int sl;
    Glyph g = {{' '}, ATTR_NULL, defaultbg, defaultcs, 0};
    
    /* Don't draw cursor when scrolled */
    if (t_get_scroll_offset() > 0) return;

    LIMIT(oldx, 0, term.col - 1);
    LIMIT(oldy, 0, term.row - 1);

    if (term.line[term.c.y][term.c.x].state & GLYPH_SET) memcpy(g.c, term.line[term.c.y][term.c.x].c, UTF_SIZ);

    /* remove the old cursor */
    if (term.line[oldy][oldx].state & GLYPH_SET) {
        sl = utf8_size(term.line[oldy][oldx].c);
        x_draws(term.line[oldy][oldx].c, term.line[oldy][oldx], oldx, oldy, 1, sl);
    } else {
        sdl_term_clear(oldx, oldy, oldx, oldy);
    }

    /* draw the new one */
    if (!(term.c.state & CURSOR_HIDE)) {
        if (!(main_window.state & WIN_FOCUSED)) g.bg = defaultucs;

        if (IS_SET(MODE_REVERSE)) g.mode |= ATTR_REVERSE, g.fg = defaultcs, g.bg = defaultfg;

        sl = utf8_size(g.c);
        x_draws(g.c, g, term.c.x, term.c.y, 1, sl);
        oldx = term.c.x, oldy = term.c.y;
    }
}

void redraw(void) {
    struct timespec tv = {0, REDRAW_TIMEOUT * 1000};

    t_full_dirt();
    draw();
    nanosleep(&tv, NULL);
}

void draw(void) {
    draw_region(0, 0, term.col, term.row);
    draw_scrollbar();
    update_render();
}

void draw_scrollbar(void) {
    int scroll_offset = t_get_scroll_offset();
    if (scroll_offset == 0 || main_window.surface == NULL) return;
    
    /* Draw scroll indicator in top-right corner */
    char scroll_text[64];
    snprintf(scroll_text, sizeof(scroll_text), "[%d]^", scroll_offset);
    
    int text_x = main_window.surface->w - (strlen(scroll_text) * main_window.char_width) - borderpx - 2;
    int text_y = borderpx;
    
    SDL_Color indicator_bg = drawing_ctx.colors[defaultcs];
    SDL_Color indicator_fg = drawing_ctx.colors[defaultbg];
    
    /* Draw background box */
    SDL_Rect bg_rect = {
        text_x - 2,
        text_y - 1,
        strlen(scroll_text) * main_window.char_width + 4,
        main_window.char_height + 2
    };
    SDL_FillRect(main_window.surface, &bg_rect, SDL_MapRGB(main_window.surface->format, indicator_bg.r, indicator_bg.g, indicator_bg.b));
    
    /* Draw text */
    if (is_ttf_loaded()) {
        draw_string_ttf(main_window.surface, scroll_text, text_x, text_y, indicator_fg, indicator_bg);
    } else {
        draw_string(main_window.surface, scroll_text, text_x, text_y, SDL_MapRGB(main_window.surface->format, indicator_fg.r, indicator_fg.g, indicator_fg.b), embedded_font_name);
    }
}

void draw_region(int x1, int y1, int x2, int y2) {
    int ic, ib, x, y, ox, sl;
    Glyph base, new;
    char buf[DRAW_BUF_SIZ];
    int scroll_offset = t_get_scroll_offset();
    Line line_to_draw;

    if (!(main_window.state & WIN_VISIBLE)) return;

    for (y = y1; y < y2; y++) {
        if (!term.dirty[y]) continue;

        /* Determine which line to draw (from scrollback or current screen) */
        if (scroll_offset > 0 && y < scroll_offset) {
            /* Draw from scrollback buffer (circular buffer) */
            int sb_idx = (term.scrollback_pos - scroll_offset + y + term.scrollback_size) % term.scrollback_size;
            if (sb_idx >= 0 && sb_idx < term.scrollback_count) {
                line_to_draw = term.scrollback[sb_idx];
            } else {
                line_to_draw = term.line[y];
            }
        } else {
            /* Draw from current screen, offset by scroll amount */
            int screen_y = y - scroll_offset;
            if (screen_y >= 0 && screen_y < term.row) {
                line_to_draw = term.line[screen_y];
            } else {
                sdl_term_clear(0, y, term.col, y);
                term.dirty[y] = 0;
                continue;
            }
        }

        sdl_term_clear(0, y, term.col, y);
        term.dirty[y] = 0;
        base = line_to_draw[0];
        ic = ib = ox = 0;
        for (x = x1; x < x2; x++) {
            new = line_to_draw[x];
            if (ib > 0 && (!(new.state & GLYPH_SET) || ATTRCMP(base, new) || ib >= DRAW_BUF_SIZ - UTF_SIZ)) {
                x_draws(buf, base, ox, y, ic, ib);
                ic = ib = 0;
            }
            if (new.state & GLYPH_SET) {
                if (ib == 0) {
                    ox = x;
                    base = new;
                }
                sl = utf8_size(new.c);
                memcpy(buf + ib, new.c, sl);
                ib += sl;
                ++ic;
            }
        }
        if (ib > 0) x_draws(buf, base, ox, y, ic, ib);
    }
    x_draw_cursor();
}

char *k_map(SDL_Keycode k, Uint16 state) {
    int i;
    SDL_Keymod mask;

    for (i = 0; i < LEN(non_printing_keyboard_keys); i++) {
        mask = non_printing_keyboard_keys[i].mask;

        if (non_printing_keyboard_keys[i].k == k && ((state & mask) == mask || (mask == 0 && !state))) {
            return (char *)non_printing_keyboard_keys[i].s;
        }
    }
    return NULL;
}

void print_non_printing_key_for_debug(char *non_printing_key, SDL_KeyboardEvent *e) {
    char escaped_seq[16] = {0};
    int idx = 0;
    for (int i = 0; non_printing_key[i] != '\0'; i++) {
        unsigned char ch = non_printing_key[i];
        if (ch == 27) {  // '\033'
            strcpy(&escaped_seq[idx], "\\033");
            idx += 4;
        } else if (ch >= 32 && ch <= 126) {
            escaped_seq[idx++] = ch;
        } else {
            sprintf(&escaped_seq[idx], "\\x%02X", ch);
            idx += 4;
        }
    }
    escaped_seq[idx] = '\0';
    printf("Custom key mapped: %s - ksym=%d, scancode=%d, mod=%d\n", escaped_seq, e->keysym.sym, e->keysym.scancode, e->keysym.mod);
}

void k_press(SDL_Event *ev) {
    SDL_KeyboardEvent *e = &ev->key;
    char *non_printing_key;
    int meta, shift, ctrl, synth;
    SDL_Keycode ksym = e->keysym.sym;

    if (IS_SET(MODE_KBDLOCK)) return;

    meta = e->keysym.mod & KMOD_ALT;
    shift = e->keysym.mod & KMOD_SHIFT;
    ctrl = e->keysym.mod & KMOD_CTRL;
    synth = e->keysym.mod & KMOD_SYNTHETIC;

    // printf("kpress: keysym=%d scancode=%d mod=%d\n", ksym, e->keysym.scancode, e->keysym.mod);

    /* Handle scroll up/down for scrollback */
    if (ksym == KEY_SCROLLUP) {
        t_scroll_view_up(3);
        draw();  // Force immediate redraw
        return;
    } else if (ksym == KEY_SCROLLDOWN) {
        t_scroll_view_down(3);
        draw();  // Force immediate redraw
        return;
    }
    
    /* Reset scroll on any other key press */
    if (t_get_scroll_offset() > 0) {
        t_scroll_view_reset();
    }

    if ((non_printing_key = k_map(ksym, e->keysym.mod))) { /* 1. non printing keys from vt100.h */
        // print_non_printing_key_for_debug(non_printing_key, e);
        tty_write(non_printing_key, strlen(non_printing_key));
    } else if (ctrl && !meta && !shift) { /* 2. handle ctrl key */
        switch (ksym) {
            case SDLK_a:
                tty_write("\001", 1);
                break;
            case SDLK_b:
                tty_write("\002", 1);
                break;
            case SDLK_c:
                tty_write("\003", 1);
                break;
            case SDLK_d:
                tty_write("\004", 1);
                break;
            case SDLK_e:
                tty_write("\005", 1);
                break;
            case SDLK_f:
                tty_write("\006", 1);
                break;
            case SDLK_g:
                tty_write("\007", 1);
                break;
            case SDLK_h:
                tty_write("\010", 1);
                break;
            case SDLK_i:
                tty_write("\011", 1);
                break;
            case SDLK_j:
                tty_write("\012", 1);
                break;
            case SDLK_k:
                tty_write("\013", 1);
                break;
            case SDLK_l:
                tty_write("\014", 1);
                break;
            case SDLK_m:
                tty_write("\015", 1);
                break;
            case SDLK_n:
                tty_write("\016", 1);
                break;
            case SDLK_o:
                tty_write("\017", 1);
                break;
            case SDLK_p:
                tty_write("\020", 1);
                break;
            case SDLK_q:
                tty_write("\021", 1);
                break;
            case SDLK_r:
                tty_write("\022", 1);
                break;
            case SDLK_s:
                tty_write("\023", 1);
                break;
            case SDLK_t:
                tty_write("\024", 1);
                break;
            case SDLK_u:
                tty_write("\025", 1);
                break;
            case SDLK_v:
                tty_write("\026", 1);
                break;
            case SDLK_w:
                tty_write("\027", 1);
                break;
            case SDLK_x:
                tty_write("\030", 1);
                break;
            case SDLK_y:
                tty_write("\031", 1);
                break;
            case SDLK_z:
                tty_write("\032", 1);
                break;
            default:
                break;
        }
    } else {
        // special volumeup/down/powerkey handling
        if (e->keysym.scancode == 128) {
            printf("Volume Up key pressed\n");
        } else if (e->keysym.scancode == 129) {
            printf("Volume Down key pressed\n");
        } else if (e->keysym.scancode == 102) {
            printf("Power key pressed\n");
        }

        // keys pressed by on-screen keyboard
        if (synth) {
            // printf("Synthetic key event: %s\n", SDL_GetKeyName(e->keysym.sym));
            if (e->keysym.sym <= 128) {
                char ch = (char)e->keysym.sym;
                if (meta) {
                    tty_write("\033", 1);
                }
                tty_write(&ch, 1);
            }
        }
    }
    /* For printable keys, we handle text input separately with SDL_TEXTINPUT events */
}

void text_input(SDL_Event *ev) {
    SDL_TextInputEvent *e = &ev->text;
    tty_write(e->text, strlen(e->text));
}

int tty_thread(void *unused) {
    int i;
    int got_data = 0; /* co output moi tu shell ke tu lan ve cuoi */
    fd_set rfd;
    struct timeval drawtimeout, *tv = NULL;
    SDL_Event event;
    (void)unused;

    event.type = SDL_USEREVENT;
    event.user.code = 0;
    event.user.data1 = NULL;
    event.user.data2 = NULL;

    for (i = 0;; i++) {
        if (thread_should_exit || trimui_thread_should_exit) break;
        if (cmdfd < 0 || cmdfd >= FD_SETSIZE) break;
        FD_ZERO(&rfd);
        FD_SET(cmdfd, &rfd);
        {
            struct timeval tv_idle = {0, 200 * 1000};
            int sr = select(cmdfd + 1, &rfd, NULL, NULL, tv ? tv : &tv_idle);
            if (sr < 0) {
                if (errno == EINTR) continue;
                break;
            }
            /* Het burst output (timeout) ma co du lieu moi -> bao main ve lai
               terminal (co che goc cua upstream). Neu continue luon o day thi
               man hinh terminal khong bao gio duoc ve lai (den thui). */
            if (sr == 0) {
                tv = NULL;
                i = 0;
                if (got_data) {
                    got_data = 0;
                    tty_data_pending = 1;
                    SDL_PushEvent(&event);
                }
                continue;
            }
        }

        /*
         * Stop after a certain number of reads so the user does not
         * feel like the system is stuttering.
         */
        if (i < 1000 && FD_ISSET(cmdfd, &rfd)) {
            if (tty_read() < 0) { /* EOF/pty chet -> ve not cuoi roi thoat */
                tty_data_pending = 1;
                SDL_PushEvent(&event);
                break;
            }
            tty_data_pending = 1;
            got_data = 1;

            /*
             * Just wait a bit so it isn't disturbing the
             * user and the system is able to write something.
             */
            drawtimeout.tv_sec = 0;
            drawtimeout.tv_usec = 5;
            tv = &drawtimeout;
            continue;
        }
        i = 0;
        tv = NULL;

        SDL_PushEvent(&event);
    }

    tty_thread_done = 1;
    return 0;
}

static Uint32 clear_popup_timer(Uint32 interval, void *param) {
    popup_message[0] = '\0';
    return 0;  // one-shot timer
}

void take_screenshot() {
    char filename[64];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(filename, sizeof(filename), "st-%y%m%d_%H%M%S.bmp", t);
    // get home directory
    const char *home_dir = getenv("HOME");
    if (home_dir != NULL) {
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s/%s", home_dir, filename);
        strcpy(filename, filepath);
    }

    if (main_window.surface) {
        if (SDL_SaveBMP(main_window.surface, filename) == 0) {
            sprintf(popup_message, "Đã lưu ảnh: %s", filename);
        } else {
            sprintf(popup_message, "Lỗi lưu ảnh: %s", SDL_GetError());
        }
    }

    // Clear the popup message after 3 seconds
    SDL_AddTimer(3000, clear_popup_timer, NULL);
}

void main_loop(void) {
    SDL_Event ev;
    int running = 1;
    int should_rerender = 0;
    int button_up_held = 0, button_down_held = 0, button_left_held = 0, button_right_held = 0;
    Uint32 last_button_held_time = 0;
#if defined(RG35XXSP) || defined(TRIMUI_BRICK)
    Uint8 joy0_hat0_last_state = 0;
#endif
    while (running) {
        /* Shell chet (go exit) -> ve menu may, khong treo. */
        if (trimui_child_exited && !thread_should_exit) {
            running = 0;
            break;
        }
        while (SDL_PollEvent(&ev))
        // while (SDL_WaitEvent(&ev))
        {
            if (ev.type == SDL_QUIT) {
                running = 0;
                break;
            }
            if (ev.type == SDL_MOUSEMOTION || ev.type == SDL_MOUSEBUTTONDOWN || ev.type == SDL_MOUSEBUTTONUP) {
                continue;  // skip mouse events
            }
            if (ev.type == SDL_WINDOWEVENT) {
                // if (ev.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                //     main_window.state |= WIN_FOCUSED;
                //     draw();  // redraw to update cursor color
                // } else if (ev.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                //     main_window.state &= ~WIN_FOCUSED;
                //     draw();  // redraw to update cursor color
                // }
                continue;  // skip other window events for now
            }

            if (ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP) {
                // printf("Keyboard event received - key: %d (%s), state: %s\n", ev.key.keysym.sym, SDL_GetKeyName(ev.key.keysym.sym), (ev.type == SDL_KEYDOWN) ? "DOWN" : "UP");
                int keyboard_event = handle_keyboard_event(&ev);
                if (keyboard_event == 1) {
                    // printf("OSK handled the event.\n");
                } else {
                    // printf("OSK passing event to default handler.\n");
                    if (event_handler[ev.type]) (event_handler[ev.type])(&ev);
                }

                int held = (ev.type == SDL_KEYDOWN);
                switch (ev.key.keysym.sym) {
                    case KEY_LEFT:
                        button_left_held = held;
                        break;
                    case KEY_RIGHT:
                        button_right_held = held;
                        break;
                    case KEY_UP:
                        button_up_held = held;
                        break;
                    case KEY_DOWN:
                        button_down_held = held;
                        break;
                    default:
                        break;
                }
            } else if (ev.type == SDL_JOYBUTTONDOWN || ev.type == SDL_JOYBUTTONUP) {
                // printf("Joystick event received type: %s - %d\n", (ev.jbutton.state == SDL_PRESSED) ? "down" : "up", ev.jbutton.button);
                SDL_Event sdl_event = {.key = {.type = (ev.jbutton.state == SDL_PRESSED) ? SDL_KEYDOWN : SDL_KEYUP,
                                               .state = (ev.jbutton.state == SDL_PRESSED) ? SDL_PRESSED : SDL_RELEASED,
                                               .keysym = {
                                                   .scancode = -ev.jbutton.button,
                                                   .sym = -ev.jbutton.button,
                                                   .mod = 0,
                                               }}};

                SDL_PushEvent(&sdl_event);
#if defined(RG35XXSP) || defined(TRIMUI_BRICK)
            } else if (ev.type == SDL_JOYHATMOTION && ev.jhat.which == 0 &&
                       ev.jhat.hat == 0) {
                // The RG35XXSP does not treat the d-pad directions as individual buttons; instead it treats it as a joystick hat.
                // Here we translate hat events into key events to handle those directions.
                static const Uint8 HAT_MASKS[] = {
                  SDL_HAT_LEFT, SDL_HAT_RIGHT, SDL_HAT_UP, SDL_HAT_DOWN,
                };
                static const int HAT_BUTTONS[] = {
                  JOYBUTTON_LEFT, JOYBUTTON_RIGHT, JOYBUTTON_UP, JOYBUTTON_DOWN,
                };
                for (int i = 0; i < 4; i++) {
                    if ((joy0_hat0_last_state & HAT_MASKS[i]) && !(ev.jhat.value & HAT_MASKS[i])) {
                        SDL_Event sdl_event = {.key = {.type = SDL_KEYUP,
                                                       .state = SDL_RELEASED,
                                                       .keysym = {
                                                           .scancode = HAT_BUTTONS[i],
                                                           .sym = HAT_BUTTONS[i],
                                                           .mod = 0,
                                                       }}};

                        SDL_PushEvent(&sdl_event);
                    } else if (!(joy0_hat0_last_state & HAT_MASKS[i]) && (ev.jhat.value & HAT_MASKS[i])) {
                        SDL_Event sdl_event = {.key = {.type = SDL_KEYDOWN,
                                                       .state = SDL_PRESSED,
                                                       .keysym = {
                                                           .scancode = HAT_BUTTONS[i],
                                                           .sym = HAT_BUTTONS[i],
                                                           .mod = 0,
                                                       }}};

                        SDL_PushEvent(&sdl_event);
                    }
                }
                joy0_hat0_last_state = ev.jhat.value;
#endif
            } else {
                if (event_handler[ev.type]) (event_handler[ev.type])(&ev);
            }

            switch (ev.type) {
                case SDL_USEREVENT:
                    if (ev.user.code == 0) {  // redraw terminal
                        draw();
                    } else if (ev.user.code == 1) {  // Take a screenshot
                        take_screenshot();
                    }
            }
            should_rerender = 1;
        }

        Uint32 now = SDL_GetTicks();
        int key = 0;
        if (button_down_held)
            key = KEY_DOWN;
        else if (button_up_held)
            key = KEY_UP;
        else if (button_left_held)
            key = KEY_LEFT;
        else if (button_right_held)
            key = KEY_RIGHT;

        if (key && now - last_button_held_time > BUTTON_HELD_DELAY) {
            handle_narrow_keys_held(key);
            last_button_held_time = now;
            should_rerender = 1;
        }

        /* Shell co output moi (bao tu tty thread) -> ve lai terminal ngay,
           ke ca khi event USEREVENT bi rot khoi hang doi. */
        if (tty_data_pending) {
            tty_data_pending = 0;
            draw(); /* ve terminal + present */
            should_rerender = 0;
        }

        /* OTA nen: hien badge/popup thong bao tien trinh cap nhat. */
        trimui_poll_ota();

        if (should_rerender) {
            update_render();  // redraw the screen
            should_rerender = 0;
        }
        SDL_Delay(33);    // ~30 FPS
    }

    sdl_shutdown();
}

int main(int argc, char *argv[]) {
    setenv("SDL_NOMOUSE", "1", 1);
    signal(SIGPIPE, SIG_IGN); /* write ra pty chet tra EPIPE, khong kill process */
    int is_scale_set_by_user = 0;

    for (int i = 1; i < argc; i++) {
        // Handle multi-character options first
        if (strcmp(argv[i], "-scale") == 0) {
            if (++i < argc) {
                opt_scale = atof(argv[i]);
                if (opt_scale <= 0) {
                    fprintf(stderr, "Invalid scale: %s (must be positive)\n", argv[i]);
                    opt_scale = 2.0;
                }
                is_scale_set_by_user = 1;
            } else {
                fprintf(stderr, "Missing argument for -scale\n");
                die(USAGE);
            }
            continue;
        }
        if (strcmp(argv[i], "-rotate") == 0) {
            if (++i < argc) {
                int val = atoi(argv[i]);
                if (val == 0 || val == 90 || val == 180 || val == 270) {
                    opt_rotate = val;
                } else {
                    fprintf(stderr, "Invalid rotate: %s (allowed: 0,90,180,270)\n", argv[i]);
                    die(USAGE);
                }
            } else {
                fprintf(stderr, "Missing argument for -rotate\n");
                die(USAGE);
            }
            continue;
        }
        if (strcmp(argv[i], "-font") == 0) {
            if (++i < argc) {
                opt_font = argv[i];
                if (!is_scale_set_by_user) {
                    opt_scale = 1.0;  // if custom font is set, default scale to 1.0
                }

                if (strcmp(opt_font, "1") == 0) {
                    opt_font = NULL;
                    embedded_font_name = 1;
                    opt_scale = 2.0;
                } else if (strcmp(opt_font, "2") == 0) {
                    opt_font = NULL;
                    embedded_font_name = 2;
                    opt_scale = 2.0;
                } else if (strcmp(opt_font, "3") == 0) {
                    opt_font = NULL;
                    embedded_font_name = 3;
                    opt_scale = 2.0;
                } else if (strcmp(opt_font, "4") == 0) {
                    opt_font = NULL;
                    embedded_font_name = 4;
                    opt_scale = 1.0;
                } else if (strcmp(opt_font, "5") == 0) {
                    opt_font = NULL;
                    embedded_font_name = 5;
                    opt_scale = 1.0;
                }
            } else {
                fprintf(stderr, "Missing argument for -font\n");
                die(USAGE);
            }
            continue;
        }
        if (strcmp(argv[i], "-fontsize") == 0) {
            if (++i < argc) {
                opt_fontsize = atoi(argv[i]);
                if (opt_fontsize <= 0) {
                    fprintf(stderr, "Invalid fontsize: %s (must be positive)\n", argv[i]);
                    opt_fontsize = 0;
                }
            } else {
                fprintf(stderr, "Missing argument for -fontsize\n");
                die(USAGE);
            }
            continue;
        }
        if (strcmp(argv[i], "-fontshade") == 0) {
            if (++i < argc) {
                opt_fontshade = atoi(argv[i]);
            } else {
                fprintf(stderr, "Missing argument for -fontshade\n");
                die(USAGE);
            }
            continue;
        }
        if (strcmp(argv[i], "-useEmbeddedFontForKeyboard") == 0) {
            if (++i < argc) {
                opt_use_embedded_font_for_keyboard = atoi(argv[i]);
            } else {
                fprintf(stderr, "Missing argument for -useEmbeddedFontForKeyboard\n");
                die(USAGE);
            }
            continue;
        }

        switch (argv[i][0] != '-' || argv[i][2] ? -1 : argv[i][1]) {
            case 'r':  // run commands from arguments, must be at the end of argv
                if (++i < argc) {
                    opt_cmd = &argv[i];
                    opt_cmd_size = argc - i;
                    for (int j = 0; j < opt_cmd_size; j++) {
                        printf("Command to execute: %s\n", opt_cmd[j]);
                    }
                    show_help = 0;
                }
                break;
            case 'o':  // save output commands to file
                if (++i < argc) opt_io = argv[i];
                break;
            case 'q':  // quiet mode
                active = show_help = 0;
                break;
            case 'h':  // print help
                die(USAGE);
                exit(0);
            default:
                die(USAGE);
        }
    }

    if (atexit(sdl_shutdown)) {
        fprintf(stderr, "Unable to register SDL_Quit atexit\n");
    }

    sdl_init();
    {
        int content_w = main_window.surface ? main_window.surface->w : main_window.width;
        int content_h = main_window.surface ? main_window.surface->h : main_window.height;
        t_new((content_w - borderpx) / main_window.char_width, (content_h - borderpx) / main_window.char_height);
    }
    tty_new();
    create_tty_thread();
    scale_to_size((int)(main_window.width / opt_scale), (int)(main_window.height / opt_scale));
    init_keyboard(embedded_font_name, opt_use_embedded_font_for_keyboard);
    draw(); /* ve khung terminal + ban phim ngay, khong doi event dau */
    main_loop();
    return 0;
}
