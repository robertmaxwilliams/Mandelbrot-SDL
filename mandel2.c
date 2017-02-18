#include <stdio.h>
#include <SDL.h>
#include <math.h>
#include "color_custom.h"
#define WIDTH 1000
#define HEIGHT 600
#define BPP 4
#define DEPTH 32

struct imag {
    float real;
    float im;
};

struct imag julia_root = {0, 0};
struct imag center = {0, 0};
float zoom = 1;
int iterations = 10;



void setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
    y = y*screen->pitch/BPP;

    Uint32 *pixmem32;
    Uint32 color;

    color = SDL_MapRGB(screen->format, r, g, b);

    pixmem32 = (Uint32*) screen->pixels + y + x;
    *pixmem32 = color;
}


struct imag add(struct imag a, struct imag b)
{
    return (struct imag){a.real+b.real, a.im+b.im};
}

struct imag mult(struct imag a, struct imag b)
{ 
    return (struct imag){a.real*b.real - a.im*b.im, a.real*b.im + a.im*b.real};
}

float abs_im(struct imag a)
{
    return sqrtf(a.real * a.real + a.im * a.im);
}

float sqr(float x) 
{
    return x*x;
}

float sign(float x)
{
    if (x > 0)
        return 1;
    return -1;
}

float mandle(struct imag c, int iterations)
{
    struct imag z = julia_root;
    for (int i=0; i<iterations; i++)
    {
        z = add(mult(z, z), c);
        if (abs_im(z) > 2)
            return abs_im(z);
    }
    return 0;
}

struct imag px_to_math(int x, int y)
{
    return (struct imag) {(x - (WIDTH/2)) / (zoom * 100.0) - center.real, 
                          (y - (HEIGHT/2)) / (zoom * 100.0) - center.im};
}
   

void DrawScreen(SDL_Surface* screen, int h)
{

    int x, y;
    int h_slow = h/100;
    Uint32 val;
    //I think yb is y in pixels

    //conditionally perform locking before accessing pixels
    //return if failed to lock
    if (SDL_MUSTLOCK(screen))
        if (SDL_LockSurface(screen) <0)
            return;

    int mouse_x, mouse_y;

    if (SDL_GetMouseState(&mouse_x, &mouse_y) && SDL_BUTTON(SDL_BUTTON_LEFT))
    {
        zoom *= 1.1;
    }

    julia_root = (struct imag) {0, 0};//px_to_math(mouse_x, mouse_y);uncommment for track mode
    
    printf("%.3f %.3f\n", julia_root.real, julia_root.im);

    

    for (y = 0; y < screen->h; y++)
    {
        for(x = 0; x < screen->w; x++)
        {
            //convert screen coords to mathy coords
            struct imag c = px_to_math(x, y);
            c.real -=  sqr(julia_root.real) - sqr(julia_root.im);
            c.im -= 2*(julia_root.real*julia_root.im);

            float v = mandle(c, iterations);

            //printf("v: %.3f \n", v);//(int) visual_data[y][x]);
            float color = fmod(v, 1.0) * 360;
         
            hsv HSV = {color, 0.8, 0.8};
            if (v==0)
                HSV.v = 0;

            rgb RGB = hsv2rgb(HSV);
            
            setpixel(screen, x, y, RGB.r*256, RGB.g*256, RGB.b*256);
        }
    }

    if (SDL_MUSTLOCK(screen)) 
        SDL_UnlockSurface(screen);

    SDL_Flip(screen);
}


int main(int argc, char* argv[])
{
    SDL_Surface *screen;
    SDL_Event event;

    int keypress = 0;
    int h = 0;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return 1;

    if (!(screen = SDL_SetVideoMode(WIDTH, HEIGHT, DEPTH, SDL_HWSURFACE)))
    {
        SDL_Quit();
        return 1;
    }

    while(!keypress)
    {
        DrawScreen(screen, h++);
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    keypress = 1;
                    break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_a:
                            center.real += 0.1/zoom;
                            break;

                        case SDLK_d:
                            center.real -= 0.1/zoom;
                            break;

                        case SDLK_w:
                            center.im += 0.1/zoom;
                            break;

                        case SDLK_s:
                            center.im -= 0.1/zoom;
                            break;
                        
                        case SDLK_e:
                            zoom *= 1.1;
                            break;

                        case SDLK_q:
                            zoom /= 1.1;
                            break;

                        case SDLK_ESCAPE:
                            keypress = 1;
                            break;

                        case SDLK_2:
                            iterations += 1;
                            printf("Iterations: %d\n", iterations);
                            break;

                        case SDLK_1:
                            iterations -= 1;
                            printf("Iterations: %d\n", iterations);
                            break;


                        default:
                            break;
                    }
                    break;
            }
        }
    }

    SDL_Quit();

    return 0;
}



