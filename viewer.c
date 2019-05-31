#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>

typedef struct {
    uint32_t id;
    uint32_t font_height;
    uint32_t glyph_count;
    uint32_t header_size;
    int32_t reserved[8];
} ui_font_data_t;

typedef struct {
    uint32_t utf8_code;
    uint32_t font_width;
    uint32_t header_size;
    uint32_t data_size;
    int32_t reserved[4];
} ui_font_glyph_data_t;

//extern uint8_t din_condensed50[42084];
extern uint8_t samsungone20[643267];

SDL_Window *window = NULL;
SDL_Surface *screenSurface = NULL;
SDL_Event event;

//ui_font_data_t *fnt = (ui_font_data_t *)din_condensed50;
ui_font_data_t *fnt = (ui_font_data_t *)samsungone20;

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 0;
    }

    window = SDL_CreateWindow(argv[0], SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512,
                              512, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(0);
    }

    screenSurface = SDL_GetWindowSurface(window);
    SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00));

    ui_font_glyph_data_t *glyph = (ui_font_glyph_data_t *)((uint8_t *)fnt + fnt->header_size);

    int start_x = 0;
    int start_y = 0;
    for (int i = 0; i < fnt->glyph_count; i++) {
        int x = 0;
        int y = 0;
        uint8_t *body = (uint8_t *)glyph + glyph->header_size;

        for (int j = 0; j < glyph->data_size; j++) {
            if (body[j] == 0) {
                int count = body[j + 1];
                for (int k = 0; k < count + 1; k++) {
                    x++;
                    if (x >= glyph->font_width) {
                        x = 0;
                        y++;
                    }
                }
                j++;
            } else {
                SDL_Rect rect = {start_x + x, start_y + y, 1, 1};
                SDL_FillRect(screenSurface, &rect, SDL_MapRGB(screenSurface->format, body[j], body[j], body[j]));
                x++;
                if (x >= glyph->font_width) {
                    x = 0;
                    y++;
                }
            }
        }

        start_x += glyph->font_width;
        if (start_x > 490) {
            start_x = 0;
            start_y += fnt->font_height;
        }

        glyph = (ui_font_glyph_data_t *)((uint8_t *)glyph + glyph->header_size + glyph->data_size);
    }

    SDL_UpdateWindowSurface(window);

    while (SDL_WaitEvent(&event) >= 0) {
        switch (event.type) {
        case SDL_KEYDOWN: {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 0;
                break;
            }
        } break;

        case SDL_QUIT: {
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 0;
        } break;
        }
    }

    printf("Unknown error exit\n");
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}