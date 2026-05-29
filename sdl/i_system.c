//      System specific interface stuff.
//		this sucks

#include "../doomdef.h"
#include "../d_ticcmd.h"
#include "../d_main.h"
#include "../i_system.h"
#include "../i_joy.h"
#include "../d_event.h"

#include <SDL2/SDL.h>

int mb_used = 48; // more than enough
JoyType_t   Joystick;

// See Shutdown_xxx() routines.
byte graphics_started;
byte keyboard_started;
byte sound_started;
//extern byte music_installed;

/* flag for 'win-friendly' mode used by interface code */
int i_love_bill; // no i dont
volatile ULONG ticcount;

// Called by DoomMain.
void I_InitJoystick (void) {}

// Called by startup code
// to get the ammount of memory to malloc
// for the zone management.
byte* I_ZoneBase(int* size)
{
	void* pmem;

	// do it the old way
	*size = mb_used * 1024 * 1024;
	pmem = malloc(*size);

	if (!pmem)
	{
		I_Error("Could not allocate %d megabytes.\n"
			"Please use -mb parameter and specify a lower value.\n", mb_used);
	}

	//TODO: lock the memory
	memset(pmem, 0, *size);

	return (byte*)pmem;
}

void I_GetFreeMem(void) {}

// Called by D_DoomLoop,
// returns current time in tics.
ULONG I_GetTime (void)
{
	ULONG ticks = SDL_GetTicks();
	ticks *= 35;
	ticks /= 1000;
	return ticks;
}


void I_GetEvent (void) {}


//
// Called by D_DoomLoop,
// called before processing any tics in a frame
// (just after displaying a frame).
// Time consuming syncronous operations
// are performed here (joystick reading).
// Can call D_PostEvent.
//
void I_StartFrame(void)
{
    SDL_Event ev;
    event_t e;

    while (SDL_PollEvent(&ev))
    {
        memset(&e, 0, sizeof(e));

        switch (ev.type)
        {
            case SDL_QUIT:
                I_Quit();
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                e.type = (ev.type == SDL_KEYDOWN) ? ev_keydown : ev_keyup;

                SDL_Keycode key = ev.key.keysym.sym;

                switch (key)
                {
                    case SDLK_UP:    e.data1 = KEY_UPARROW; break;
                    case SDLK_DOWN:  e.data1 = KEY_DOWNARROW; break;
                    case SDLK_LEFT:  e.data1 = KEY_LEFTARROW; break;
                    case SDLK_RIGHT: e.data1 = KEY_RIGHTARROW; break;
                    default:         e.data1 = key; break;
                }

                D_PostEvent(&e);
                break;
            }
        }
    }
}


//
// Called by D_DoomLoop,
// called before processing each tic in a frame.
// Quick syncronous operations are performed here.
// Can call D_PostEvent.
void I_StartTic (void) {}

// idk
int I_GetKey (void) {}

// Asynchronous interrupt functions should maintain private queues
// that are read by the synchronous functions
// to be converted into events.

// Either returns a null ticcmd,
// or calls a loadable driver to build it.
// This ticcmd will then be modified by the gameloop
// for normal input.
ticcmd_t emptycmd;
ticcmd_t* I_BaseTiccmd(void)
{
	return &emptycmd;
}


// Called by M_Responder when quit is selected, return code 0.
void I_Quit (void)
{
    SDL_Quit();
}

void I_Error (char *error, ...)
{
	va_list args;
	va_start(args, error);

	int len = vsnprintf(NULL, 0, error, args);
	va_end(args);

	char* buffer = (char*)malloc(len + 1);

	va_start(args, error);
	vsnprintf(buffer, len + 1, error, args);
	va_end(args);

	printf("%s", buffer);

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SRB2 Error", buffer, NULL);
    I_Quit();
}

// Allocates from low memory under dos,
// just mallocs under unix
byte* I_AllocLow (int length) {}

void I_Tactile (int on, int off, int total) {}

//added:18-02-98: write a message to stderr (use before I_Quit)
//                for when you need to quit with a msg, but need
//                the return code 0 of I_Quit();
void I_OutputMsg(char *error, ...)
{
	va_list args;
	va_start(args, error);

	int len = vsnprintf(NULL, 0, error, args);
	va_end(args);

	char* buffer = (char*)malloc(len + 1);

	va_start(args, error);
	vsnprintf(buffer, len + 1, error, args);
	va_end(args);

	printf(buffer);
}

void I_StartupMouse (void) {}

// keyboard startup,shutdown,handler
void I_StartupKeyboard (void) {}

// setup timer irq and user timer routine.
void I_TimerISR (void) {}      //timer callback routine.
void I_StartupTimer (void) {}

/* list of functions to call at program cleanup */
void I_AddExitFunc (void (*func)()) {}
void I_RemoveExitFunc (void (*func)()) {}

// Setup signal handler, plus stuff for trapping errors and cleanly exit.
int  I_StartupSystem (void) {return -1;}
void I_ShutdownSystem (void) 
{
    I_Quit();
}
