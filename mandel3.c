#include <stdio.h>
#include <SDL.h>
#include <math.h>
#include "color_custom.h"
#define WIDTH 100
#define HEIGHT 100
#define BPP 4
#define DEPTH 32
#define Z_CUBED 0

//complex number struct
struct comp {
    double real;
    double im;
};

struct value_depth {
    double value;
    int depth;
};

//starting point for z
struct comp julia_root = {0, 0};

//viewing center
struct comp center = {0, 0};

//zoom level
double zoom = 100;

//how many iterations to calculate for each pixel
int iterations = 10;

//takes in x y in screen coordinates, y is adjusted for screen pitch inside.
void setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
    y = y*screen->pitch/BPP;

    Uint32 *pixmem32;
    Uint32 color;

    color = SDL_MapRGB(screen->format, r, g, b);

    pixmem32 = (Uint32*) screen->pixels + y + x;
    *pixmem32 = color;
}


//suite of stub functions for manipulating complex number structs
struct comp add(struct comp a, struct comp b)
{
    return (struct comp){a.real+b.real, a.im+b.im};
}

struct comp mult(struct comp a, struct comp b)
{ 
    return (struct comp){a.real*b.real - a.im*b.im, a.real*b.im + a.im*b.real};
}

double abs_im(struct comp a)
{
    return sqrtf(a.real * a.real + a.im * a.im);
}

double sqr(double x) 
{
    return x*x;
}

double cube(double x)
{
    return x*x*x;
}

double sign(double x)
{
    if (x > 0)
        return 1;
    return -1;
}

struct value_depth mandel(struct comp c, int iterations)
{
    struct comp z = julia_root;
    for (int i=0; i<iterations; i++)
    {
        
        z = add(mult(z, z), c);
        if (abs_im(z) > 2)
            return (struct value_depth) {abs_im(z), i};
    }
    return (struct value_depth) {0.0, iterations};
}

struct value_depth mandel_3(struct comp c, int iterations)
{
    struct comp z = julia_root;
    for (int i=0; i<iterations; i++)
    {
        z = add(mult(z, mult(z, z)), c);
        if (abs_im(z) > 2)
            return (struct value_depth) {abs_im(z), i};
    }
    return (struct value_depth) {0.0, iterations};
    
}


struct comp px_to_math(int x, int y)
{
    return (struct comp) {(x - (WIDTH/2)) / zoom - center.real, 
                          (y - (HEIGHT/2)) / zoom - center.im};
}
   

void DrawScreen(SDL_Surface* screen, int h)
{

    int x, y;
    Uint32 val;

    //conditionally perform locking before accessing pixels
    //return if failed to lock
    if (SDL_MUSTLOCK(screen))
        if (SDL_LockSurface(screen) <0)
            return;

    //printf("%.3f %.3f\n", julia_root.real, julia_root.im);
    
    double v, color;
    struct comp c;
    struct value_depth vd;

    for (y = 0; y < screen->h; y++)
    {
        for(x = 0; x < screen->w; x++)
        {
            //get x y in a complex number and adjust for the "julian drift"
            //aka move to center the origin as the julian number changes
            c = px_to_math(x, y);
            if (Z_CUBED)
            {
                c.real -=  cube(julia_root.real) - cube(julia_root.im);
                c.im -= 3*(julia_root.real*julia_root.im);
                vd = mandel_3(c, iterations);
                v = vd.value;
            }
            else
            {
                c.real -=  sqr(julia_root.real) - sqr(julia_root.im);
                c.im -= 2*(julia_root.real*julia_root.im);
                vd = mandel(c, iterations);
                v = vd.value;
            }

            //use overshoot value to color, soft colors using some math
            color = ((.5*(v-1.5)) / (1+.5*(v-1.5))) * 30 + 20;
         
            //convert to RGB for rendering
            hsv HSV = {color, 0.8, 0.8};
            if (v==0)
                HSV.v = 0;
            else
                HSV.v = 0.1 + 0.4 * ((double) vd.depth/iterations)+ 0.5; //* (color/30-20);

            HSV.s = 1-((double) vd.depth/iterations);
            rgb RGB = hsv2rgb(HSV);
            
            //write this pixel
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
    
    int quit = 0;
    int keypress = 1;
    int h = 0;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return 1;

    if (!(screen = SDL_SetVideoMode(WIDTH, HEIGHT, DEPTH, SDL_HWSURFACE)))
    {
        SDL_Quit();
        return 1;
    }

    while(!quit)
    {
        if (keypress) 
        {
            DrawScreen(screen, h++);
            keypress = 0;
        }

        int mouse_x, mouse_y;
        if (SDL_GetRelativeMouseState(&mouse_x, &mouse_y) && SDL_BUTTON(SDL_BUTTON_LEFT))
        {
            julia_root.real += mouse_x/zoom;
            julia_root.im += mouse_y/zoom;
            keypress = 1;
        }
        
        SDL_GetMouseState(&mouse_x, &mouse_y);
            

        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYDOWN:
                    keypress = 1;
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_a:
                            center.real += 10/zoom;
                            break;

                        case SDLK_d:
                            center.real -= 10/zoom;
                            break;

                        case SDLK_w:
                            center.im += 10/zoom;
                            break;

                        case SDLK_s:
                            center.im -= 10/zoom;
                            break;
                        
                        case SDLK_e:
                            zoom *= 1.1;
                            center.real -= (mouse_x - WIDTH/2)/(zoom*2);
                            center.im -= (mouse_y - HEIGHT/2)/(zoom*2);
                            break;

                        case SDLK_q:
                            zoom /= 1.1;
                            break;

                        case SDLK_ESCAPE:
                            quit = 1;
                            break;

                        case SDLK_2:
                            iterations += 1;
                            printf("Iterations: %d\n", iterations);
                            break;

                        case SDLK_1:
                            iterations -= 1;
                            printf("Iterations: %d\n", iterations);
                            break;

                        case SDLK_3:
                            iterations = 10;
                            printf("Iterations: %d\n", iterations);
                            break;

                        case SDLK_4:
                            iterations += 10;
                            printf("Iterations: %d\n", iterations);
                            break;

                        case SDLK_r:
                            iterations = 10;
                            center = (struct comp) {0, 0};
                            zoom = 100;
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



