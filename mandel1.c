#include <stdio.h>
#include <SDL.h>
#include <math.h>

#define WIDTH 640
#define HEIGHT 480
#define BPP 4
#define DEPTH 32

#define MAXITER 10

float julia_root = 0;


void setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
    y = y*screen->pitch/BPP;

    Uint32 *pixmem32;
    Uint32 color;

    color = SDL_MapRGB(screen->format, r, g, b);

    pixmem32 = (Uint32*) screen->pixels + y + x;
    *pixmem32 = color;
}

struct imag {
    float real;
    float im;
};

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


float mandle(struct imag c, int iterations)
{
    struct imag z = {julia_root, 0};
    for (int i=0; i<iterations; i++)
    {
        z = add(mult(z, z), c);
        if (abs_im(z) > 2)
            return abs_im(z);
    }
    return 0;
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

    for (y = 0; y < screen->h; y++)
    {
        for(x = 0; x < screen->w; x++)
        {
            //convert screen coords to mathy coords
            float xf = (x - (WIDTH/2)) / 100.0;
            float yf = (y - (HEIGHT/2)) / 100.0;

            struct imag c = {xf, yf};

            float v = mandle(c, 10);

            //printf("%.3f %.3f %.3f \n", xf, yf, v);//(int) visual_data[y][x]);

            setpixel(screen, x, y, v*100, 50, 50);
        }
    }
    julia_root += 0.01;

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
                    keypress = 1;
                    break;
            }
        }
    }

    SDL_Quit();

    return 0;
}



///hsv to rgb from https://gist.github.com/fairlight1337/4935ae72bcbcc1ba5c72
/*! \brief Convert HSV to RGB color space
  
  Converts a given set of HSV values `h', `s', `v' into RGB
  coordinates. The output RGB values are in the range [0, 1], and
  the input HSV values are in the ranges h = [0, 360], and s, v =
  [0, 1], respectively.
  
  \param fR Red component, used as output, range: [0, 1]
  \param fG Green component, used as output, range: [0, 1]
  \param fB Blue component, used as output, range: [0, 1]
  \param fH Hue component, used as input, range: [0, 360]
  \param fS Hue component, used as input, range: [0, 1]
  \param fV Hue component, used as input, range: [0, 1]
  
*/

