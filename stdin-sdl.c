#include <stdio.h>
#include <SDL.h>
#include <math.h>

#include <unistd.h>
#include <signal.h>

#define WIDTH 640
#define HEIGHT 480


volatile sig_atomic_t stop;

void inthand(int signum) {
    stop = 1;
}


int main(int argc, char* argv[])
{

    int breakmode = 0;
    int pxp = 1;
    if (argc == 2)
    {
        breakmode = 1;
        printf("breakmode!\n");
    }
    if (argc == 3)
    {
        pxp = 16;
        printf("big pix!\n");
    }

    SDL_Event event;
    
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer);
    SDL_RenderClear(renderer);
    int * crash = NULL;
    signal(SIGINT, inthand);
    

    int quit = 0;
    int nomore = 0;
    while (!stop)
    {
        if (1 || SDL_PollEvent(&event))
        {

            switch(event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;

                case SDL_MOUSEMOTION:
                    //....
                    break;

                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym) {
                        case SDLK_ESCAPE: 
                            quit = 1;
                            break;
                            // cases for other keypresses
                        case SDL_SCANCODE_D:
                            *crash = 1;
                            break; // never run
                    }
                    break;
                    // cases for other events
            }
        }

        SDL_RenderClear(renderer);
        if (nomore) continue;
        for (int y = 0; y < HEIGHT; y+=pxp)
        {
            for (int x = 0; x < WIDTH; x+=pxp)
            {
                unsigned int ch = getchar();
                if (ch == EOF)
                {
                    nomore = 1;
                    break;
                }
                if (breakmode && ch == '\n')
                {
                    break;
                }
                int rgb[3];
                rgb[0] = (0b111 & ch) * 32;
                rgb[1] = (0b111 & (ch >> 3)) * 32;
                rgb[2] = (0b11 & (ch >> 6)) * 32;
                int remaineder = (ch >> 8);
                SDL_SetRenderDrawColor(renderer, rgb[0], rgb[1], rgb[2], 255);
                if (pxp == 1)
                {
                    SDL_RenderDrawPoint(renderer, x, y);
                }
                else
                {
                    SDL_Rect rect;

                    rect.x = x;
                    rect.y = y;
                    rect.w = pxp;
                    rect.h = pxp;
                    SDL_RenderDrawRect(renderer, &rect);
                }
            }
        }
        SDL_RenderPresent(renderer);
    }



    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
