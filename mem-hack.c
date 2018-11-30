#include <stdio.h>
#include <SDL.h>
#include <math.h>
#include "handle_segfault.c"

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
    debug_enable_sigsev_handler();

    int breakmode = 0;
    int pxp = 1;

    for (int i = 1; i<argc; i++)
    {
        if (argv[i][0] == '-')
        {
            switch (argv[i][1])
            {
                case 'b':
                    breakmode = 1;
                    printf("breakmode!\n");
                    break;

                case 'p':
                    pxp = 16;
                    printf("big pix!\n");
                    break;

                case 'n':
                    pxp = atoi(argv[++i]);
                    printf("big pix n = %d!\n", pxp);
                    break;

                default:
                    printf("Not recorgnized\n");
            }
        } else
        {
            printf("Super not recognized\n");
        }
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
    int i = 0;
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


        int *die = malloc(WIDTH*HEIGHT*10);
        for (int foo = 0; foo < 900; foo++)
            *(die+foo) = 100;
        int *safe = die;
        free(die);
        
        int valid_pixels = 0;

        i = (i+1)%100;
        printf("%d\n", i);
        fflush(stdout); 
        for (int y = 0; y < HEIGHT; ++y)
        {
            for (int x = 0; x < WIDTH; ++x)
            {
                unsigned int ch = 0;
                //debug_print_pointer((safe + x + y*HEIGHT + WIDTH*HEIGHT*i));
                debug_catch_sigsegv_mode = true;
                if (sigsetjmp(jbuf, !0) == 0)
                {
                    ch =  *(safe + x + y*(HEIGHT/pxp) + (WIDTH*HEIGHT/(pxp*pxp))*i);
                    valid_pixels++;
                }
                else
                {
                    debug_catch_sigsegv_mode = false;
                    continue;
                }

                debug_catch_sigsegv_mode = false;



                int rgb[3];
                rgb[0] = (0b111 & ch) * 32;
                rgb[1] = (0b111 & (ch >> 3)) * 32;
                rgb[2] = (0b11 & (ch >> 6)) * 32;
                int remaineder = (ch >> 8);
                SDL_SetRenderDrawColor(renderer, rgb[0], rgb[1], rgb[2], 255);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
        printf("valid pixels: %d\n", valid_pixels);
        SDL_RenderPresent(renderer);
        SDL_Delay(50);
    }




    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
