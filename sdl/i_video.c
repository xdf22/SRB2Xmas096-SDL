//      System specific interface stuff.
#include "../doomdef.h"
#include "../console.h"
#include "../screen.h"
#include "../v_video.h"
#include "../doomtype.h"
#include "../i_system.h"
#include "../i_video.h"
#include "i_video.h"

#include <SDL2/SDL.h>

rendermode_t rendermode = render_soft;
boolean highcolor = false;

SDL_Window* SDL_window;
SDL_Surface* surface;
SDL_Surface* window_surface;

int VID_SetMode(int modenum)
{
    (void)modenum; // no thanks

	if (SDL_window) { SDL_DestroyWindow(SDL_window); }
	if (surface) { SDL_FreeSurface(surface); }

	vid.modenum = 1;
	vid.width = 640;
	vid.height = 400;
	vid.bpp = 1;
	vid.rowbytes = vid.width; // not multiplying by vid.bpp because its always gonna be 1 
	vid.recalc = 1;

	SDL_window = SDL_CreateWindow("SRB2 XMAS v0.96", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, vid.width, vid.height, 0);
	surface = SDL_CreateRGBSurfaceWithFormat(0, vid.width, vid.height, 8, SDL_PIXELFORMAT_INDEX8);
	window_surface = SDL_GetWindowSurface(SDL_window);

	vid.buffer = malloc((vid.width * vid.height * vid.bpp * NUMSCREENS) + (vid.width * ST_HEIGHT * vid.bpp));

	return 0;
}

void I_StartupGraphics(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) { I_Error("Failed to initialize SDL2: %s\n", SDL_GetError()); }

	VID_SetMode(1);
	graphics_started = true;
}

void I_ShutdownGraphics(void) {}          //restore old video mode

// Takes full 8 bit values.
SDL_Color sdl_palette[256];

void I_SetPalette(byte *palette)
{
	RGB_t* rgbpalette;
	rgbpalette = (RGB_t *)palette;

	for (int i = 0; i < 256; i++) {

		sdl_palette[i].r = rgbpalette[i].r;
		sdl_palette[i].g = rgbpalette[i].g;
		sdl_palette[i].b = rgbpalette[i].b;
		sdl_palette[i].a = 255;
	}

	SDL_SetPaletteColors(surface->format->palette, sdl_palette, 0, 256);
}

void I_UpdateNoBlit (void) {}

void I_FinishUpdate(void)
{
	byte *pixels = (byte *)surface->pixels;

	for (int i = 0; i < vid.width * vid.height; i++)
	{
		pixels[i] = vid.buffer[i];
	}

	SDL_BlitSurface(surface, NULL, window_surface, NULL);
	SDL_UpdateWindowSurface(SDL_window);
}

// Wait for vertical retrace or pause a bit.
void I_WaitVBL(int count) {}

void I_ReadScreen (byte* scr) {}

void I_BeginRead (void) {}
void I_EndRead (void) {}

void VID_BlitLinearScreen (void *srcptr, void *destptr, int width,
                           int height, int srcrowbytes, int destrowbytes) {}

int VID_GetModeForSize( int w, int h) {return 1;}

char *VID_GetModeName(int modenum) {return "640x400";}
int VID_NumModes(void) {return 1;}
