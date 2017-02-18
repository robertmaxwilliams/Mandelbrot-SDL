#include <stdio.h>
#include <SDL.h>
#include <math.h>
#include "color_custom.h"
#define WIDTH 500
#define HEIGHT 500
#define BPP 4
#define DEPTH 32

enum function { MANDEL, JULIA, JULIA_3, SINKING_SHIP};
enum function func = JULIA;

int compr_level = 1;
int smoothing = 0;

//complex number struct
typedef struct {
    double real;
    double im;
} comp;

typedef struct {
    double value;
    int depth;
} value_depth;

//starting point for z
comp julia_root = {0, 0};

//viewing center
comp center = {0, 0};

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

//directly get pixel using 32bit pixel value instead of rgb
Uint32 get_pixel32( SDL_Surface *screen, int x, int y )
{
    //y = y*screen->pitch/BPP;
    //Convert the pixels to 32 bit
    Uint32 *pixels = (Uint32 *)screen->pixels;
    
    //Get the requested pixel
    return pixels[ ( y * screen->w) + x ];
}

//put pixel for raw pixel instead of rgb
void put_pixel32( SDL_Surface *screen, int x, int y, Uint32 pixel )
{
    //y = y*screen->pitch/BPP;
    //Convert the pixels to 32 bit
    Uint32 *pixels = (Uint32 *)screen->pixels;
    
    //Set the pixel
    pixels[ ( y * screen->w ) + x ] = pixel;
}

//suite of stub functions for manipulating complex number structs
comp add(comp a, comp b)
{
    return (comp){a.real+b.real, a.im+b.im};
}

comp mult(comp a, comp b)
{ 
    return (comp){a.real*b.real - a.im*b.im, a.real*b.im + a.im*b.real};
}

double abs_im(comp a)
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

// ===================================
// all of the fractal functions go here
// ===================================
value_depth sinking_ship(comp c, int iterations)
{
    comp z = julia_root;
    for (int i=0; i<iterations; i++)
    {
        z.real = fabs(z.real);
        z.im = fabs(z.im);
        z = add(mult(z, z), c);
        if (abs_im(z) > 2)
            return (value_depth) {abs_im(z), i};
    }
    return (value_depth) {0.0, iterations};
}

value_depth julia(comp c, int iterations)
{
    comp z = c;
    for (int i=0; i<iterations; i++)
    {
        
        z = add(mult(z, z), julia_root);
        if (abs_im(z) > 2)
            return (value_depth) {abs_im(z), i};
    }
    return (value_depth) {0.0, iterations};
}

value_depth mandel_3(comp c, int iterations)
{
    comp z = c;
    for (int i=0; i<iterations; i++)
    {
        z = add(mult(z, mult(z, z)), julia_root);
        if (abs_im(z) > 2)
            return (value_depth) {abs_im(z), i};
    }
    return (value_depth) {0.0, iterations};
}

value_depth mandel(comp c, int iterations)
{
    comp z = julia_root;
    for (int i=0; i<iterations; i++)
    {
        z = add(mult(z, z), c);
        if (abs_im(z) > 2)
            return (value_depth) {abs_im(z), i};
    }
    return (value_depth) {0.0, iterations};
}

//convert from screen pixels to coordinates in the imaginary plane
comp px_to_math(double x, double y)
{
    return (comp) {(x - (WIDTH/2)) / zoom - center.real, 
                          -((y - (HEIGHT/2)) / zoom - center.im)};
}

//get values for pixel at x y. not to be confused with get_pixel32 which retrieves prev written pixels
void get_pixel(double x, double y, value_depth *vd)
{
    comp c = px_to_math(x, y);
    switch (func)
    {
        case MANDEL://mandel
            c.real -=  sqr(julia_root.real) - sqr(julia_root.im);
            c.im -= 2*(julia_root.real*julia_root.im);
            *vd = mandel(c, iterations);
            break;

        case JULIA://julia
            *vd = julia(c, iterations);
            break;

        case JULIA_3://z^3+c
            *vd = mandel_3(c, iterations);
            break;

        case SINKING_SHIP://sinking ship
            c.real -=  fabs(sqr(julia_root.real) - sqr(julia_root.im));
            c.im -= fabs(2*(julia_root.real*julia_root.im));
            *vd = sinking_ship(c, iterations);
            break;
    }
}

//funtion for choosing which value to keep for mulitple depth readings
int zero_or_max(running, new)
{
    if (new ==0)
        return 0;
    if (new > running)
        return new;
    return running;
}

// =======================================================
// most of the work is done here, get each pixel and draw it.
// ======================================================
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
    int d;
    value_depth vd;

    for (y = 0; y <screen->h; y++)
    {
        for(x = 0; x < screen->w; x++)
        {
            v = 0.0;
            d = 0;
            //if in fast mode and not in a corner pixel
            if (!(x % compr_level == 0 && y % compr_level == 0))
            {
                //just copy and paste top corner pixel
                put_pixel32(screen, x, y,
                        get_pixel32(screen, x-(x%compr_level), y-(y%compr_level)));
            }
            else 
            {
                if (smoothing)
                {
                    //get x y in a complex number and adjust for the drift
                    //aka move to center the origin as the so-called "julia number" changes
                    get_pixel((double) x+0.25, (double) y+0.25, &vd);
                    v += vd.value; 
                    d = zero_or_max(d, vd.depth);

                    get_pixel((double) x-0.25, (double) y+0.25, &vd);
                    v += vd.value; 
                    d = zero_or_max(d, vd.depth);

                    get_pixel((double) x-0.25, (double) y-0.25, &vd);
                    v += vd.value; 
                    d = zero_or_max(d, vd.depth);
                    
                    get_pixel((double) x+0.25, (double) y-0.25, &vd);
                    v += vd.value; 
                    d = zero_or_max(d, vd.depth);

                    v /= 4;

                }
                else    
                {
                    get_pixel((double) x, (double) y, &vd);
                    v = vd.value; d = vd.depth;
                }

                //use overshoot value to color, soft colors using some math
                color = ((.5*(v-1.5)) / (1+.5*(v-1.5))) * 30 + 20;
             
                //convert to RGB for rendering
                hsv HSV = {color, 0.8, 0.8};
                if (v==0)
                    HSV.v = 0;
                else
                    HSV.v = 0.1 + 0.4 * ((double) d/iterations)+ 0.5; //* (color/30-20);

                HSV.s = 1-((double) d/iterations);
                rgb RGB = hsv2rgb(HSV);
                
                //write this pixel
                setpixel(screen, x, y, RGB.r*256, RGB.g*256, RGB.b*256);
            }
        }
    }

    if (SDL_MUSTLOCK(screen)) 
        SDL_UnlockSurface(screen);

    SDL_Flip(screen);
}

void print_data(void)
{
    
    printf("Iterations: %d\n", iterations);
    printf("Julia value: %f, %f\n", julia_root.real, julia_root.im);
    printf("Center: %f, %f\n", center.real, center.im);
    printf("Zoom: %f\n", zoom);
    printf("Smoothing: %d\n", smoothing);
    printf("Compression Level: %d\n", compr_level);
    printf("\n");
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
            print_data();
            DrawScreen(screen, h++);
            keypress = 0;
        }

        int mouse_x, mouse_y;
        if (SDL_GetRelativeMouseState(&mouse_x, &mouse_y) && SDL_BUTTON(SDL_BUTTON_LEFT))
        {
            julia_root.real -= mouse_x/zoom;
            julia_root.im -= mouse_y/zoom;
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

                        case SDLK_LEFT:
                            julia_root.real += 1/zoom;
                            break;

                        case SDLK_RIGHT:
                            julia_root.real -= 1/zoom;
                            break;

                        case SDLK_UP:
                            julia_root.im += 1/zoom;
                            break;

                        case SDLK_DOWN:
                            julia_root.im -= 1/zoom;
                            break;
                        
                        case SDLK_e:
                            zoom *= 1.1;
                            iterations = (int) pow(zoom, 0.2753)*10;
                            break;

                        case SDLK_f:
                            zoom *= 1.1;
                            center.real -= (mouse_x - WIDTH/2)/(zoom*2);
                            center.im -= (mouse_y - HEIGHT/2)/(zoom*2);
                            iterations = (int) pow(zoom, 0.2753)*10;
                            break;


                        case SDLK_q:
                            zoom /= 1.1;
                            break;

                        case SDLK_ESCAPE:
                            quit = 1;
                            break;

                        case SDLK_1:
                            iterations -= 1;
                            break;

                        case SDLK_2:
                            iterations += 1;
                            break;

                        case SDLK_3:
                            iterations /= 2;
                            break;

                        case SDLK_4:
                            iterations *= 2;
                            break;

                        case SDLK_r:
                            iterations = 10;
                            center = (comp) {0, 0};
                            zoom = 100;
                            julia_root = (comp) {0,0};
                            break;

                        case SDLK_x:
                            compr_level *= 2;
                            if (compr_level > 32)
                                compr_level = 1;
                            break;

                        case SDLK_z:
                            smoothing = !smoothing;
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



