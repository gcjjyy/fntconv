#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>

typedef struct {
    uint32_t id;
    int32_t width;
    int32_t height;
    uint32_t pf;
    uint32_t header_size;
    uint32_t data_size;
    int32_t reserved[8];
} ui_image_data_t;

extern uint8_t buf[16440];

SDL_Window *window = NULL;
SDL_Surface *screenSurface = NULL;
SDL_Event event;

ui_image_data_t *img = (ui_image_data_t *)buf;

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 0;
    }

    window = SDL_CreateWindow(argv[0], SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, img->width,
                              img->height, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(0);
    }

    screenSurface = SDL_GetWindowSurface(window);
    SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0x33, 0x33, 0x33));

    uint8_t *data = ((uint8_t *)img) + img->header_size;

    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            SDL_Rect rect = {j, i, 1, 1};
            int index = (i * img->width + j) * 3;
            uint8_t r = (data[index] & 0b11111000);
            uint8_t g = ((data[index] & 0b00000111) << 5) | ((data[index + 1] & 0b11100000) >> 3);
            uint8_t b = (data[index] & 0b00011111) << 3;
            uint8_t a = data[index + 2];

            uint32_t *screen = (uint32_t *)screenSurface->pixels;
            uint8_t _r = 0;
            uint8_t _g = 0;
            uint8_t _b = 0;
            SDL_GetRGB(screen[i * img->width + j], screenSurface->format, &_r, &_g, &_b);

            float _a = a / 255.0f;

            r = r * _a + _r * (1.0f - _a);
            g = g * _a + _g * (1.0f - _a);
            b = b * _a + _b * (1.0f - _a);

            SDL_FillRect(screenSurface, &rect, SDL_MapRGB(screenSurface->format, r, g, b));
        }
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