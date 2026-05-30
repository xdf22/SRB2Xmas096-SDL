// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
//
// DESCRIPTION:
//      System interface, sound.
//
//-----------------------------------------------------------------------------

#include "../doomdef.h"
#include "../command.h"
#include "../doomdata.h"
#include "../m_fixed.h"
#include "../r_data.h"
#include "../byteptr.h"
#include "../i_sound.h"
#include "../s_sound.h"
#include "../w_wad.h"
#include "../sounds.h"
#include "../z_zone.h"

// move these later
#define SINT8 signed char
#define UINT8 uint8_t
#define UINT16 uint16_t
#define UINT32 uint32_t
#define READUINT16(p)       ({  UINT16 *p_tmp = (void *)p;  UINT16 b; memcpy(&b, p, sizeof( UINT16)); p_tmp++; p = (void *)p_tmp; b; })
#define READUINT32(p)       ({  UINT32 *p_tmp = (void *)p;  UINT32 b; memcpy(&b, p, sizeof( UINT32)); p_tmp++; p = (void *)p_tmp; b; })
#define min(a,b)  (a<b ? a : b)

#include <SDL2/SDL_mixer.h>

static int music_volume, sfx_volume, internal_volume = 0;
static Mix_Music *current_music = NULL; // do i REALLY need this?

// this is as fast as I can possibly make it.
// sorry. more asm needed.
static Mix_Chunk *ds2chunk(void *stream)
{
	UINT16 ver,freq;
	UINT32 samples, i, newsamples;
	UINT8 *sound;

	SINT8 *s;
	INT16 *d;
	INT16 o;
	fixed_t step, frac;

	// lump header
	ver = READUINT16(stream); // sound version format?
	if (ver != 3) // It should be 3 if it's a doomsound...
		return NULL; // onos! it's not a doomsound!
	freq = READUINT16(stream);
	samples = READUINT32(stream);

	// convert from signed 8bit ???hz to signed 16bit 44100hz.
	switch(freq)
	{
	case 44100:
		if (samples >= UINT32_MAX>>2)
			return NULL; // would wrap, can't store.
		newsamples = samples;
		break;
	case 22050:
		if (samples >= UINT32_MAX>>3)
			return NULL; // would wrap, can't store.
		newsamples = samples<<1;
		break;
	case 11025:
		if (samples >= UINT32_MAX>>4)
			return NULL; // would wrap, can't store.
		newsamples = samples<<2;
		break;
	default:
		frac = (44100 << FRACBITS) / (UINT32)freq;
		if (!(frac & 0xFFFF)) // other solid multiples (change if FRACBITS != 16)
			newsamples = samples * (frac >> FRACBITS);
		else // strange and unusual fractional frequency steps, plus anything higher than 44100hz.
			newsamples = FixedMul(FixedDiv(samples, freq), 44100) + 1; // add 1 to counter truncation.
		if (newsamples >= UINT32_MAX>>2)
			return NULL; // would and/or did wrap, can't store.
		break;
	}
	sound = Z_Malloc(newsamples<<2, PU_SOUND, NULL); // samples * frequency shift * bytes per sample * channels

	s = (SINT8 *)stream;
	d = (INT16 *)sound;

	i = 0;
	switch(freq)
	{
	case 44100: // already at the same rate? well that makes it simple.
		while(i++ < samples)
		{
			o = ((INT16)(*s++)+0x80)<<8; // changed signedness and shift up to 16 bits
			*d++ = o; // left channel
			*d++ = o; // right channel
		}
		break;
	case 22050: // unwrap 2x
		while(i++ < samples)
		{
			o = ((INT16)(*s++)+0x80)<<8; // changed signedness and shift up to 16 bits
			*d++ = o; // left channel
			*d++ = o; // right channel
			*d++ = o; // left channel
			*d++ = o; // right channel
		}
		break;
	case 11025: // unwrap 4x
		while(i++ < samples)
		{
			o = ((INT16)(*s++)+0x80)<<8; // changed signedness and shift up to 16 bits
			*d++ = o; // left channel
			*d++ = o; // right channel
			*d++ = o; // left channel
			*d++ = o; // right channel
			*d++ = o; // left channel
			*d++ = o; // right channel
			*d++ = o; // left channel
			*d++ = o; // right channel
		}
		break;
	default: // convert arbitrary hz to 44100.
		step = 0;
		frac = ((UINT32)freq << FRACBITS) / 44100 + 1; //Add 1 to counter truncation.
		while (i < samples)
		{
			o = (INT16)(*s+0x80)<<8; // changed signedness and shift up to 16 bits
			while (step < FRACUNIT) // this is as fast as I can make it.
			{
				*d++ = o; // left channel
				*d++ = o; // right channel
				step += frac;
			}
			do {
				i++; s++;
				step -= FRACUNIT;
			} while (step >= FRACUNIT);
		}
		break;
	}

	// return Mixer Chunk.
	return Mix_QuickLoad_RAW(sound, (Uint32)((UINT8*)d-sound));
}

void* I_GetSfx (sfxinfo_t*  sfx)
{
	void *lump;
	Mix_Chunk *chunk;
	SDL_RWops *rw;

	if (sfx->lumpnum == UINT32_MAX)
		sfx->lumpnum = S_GetSfxLumpNum(sfx);

	lump = W_CacheLumpNum(sfx->lumpnum, PU_SOUND);

	// convert from standard DoomSound format.
	chunk = ds2chunk(lump);
	if (chunk)
	{
		Z_Free(lump);
		return chunk;
	}

    return NULL;
}

void I_FreeSfx(sfxinfo_t *sfx)
{
	if (sfx->data)
	{
		Mix_Chunk *chunk = (Mix_Chunk*)sfx->data;
		UINT8 *abufdata = NULL;
		if (chunk->allocated == 0)
		{
			// We allocated the data in this chunk, so get the abuf from mixer, then let it free the chunk, THEN we free the data
			// I believe this should ensure the sound is not playing when we free it
			abufdata = chunk->abuf;
		}
		Mix_FreeChunk(sfx->data);
		if (abufdata)
		{
			// I'm going to assume we used Z_Malloc to allocate this data.
			Z_Free(abufdata);
		}
	}
	sfx->data = NULL;
	sfx->lumpnum = UINT32_MAX;
}


// Init at program start...
void I_StartupSound() {}

// ... update sound buffer and audio device at runtime...
void I_UpdateSound(void) {}
void I_SubmitSound(void) {}

// ... shut down and relase at program termination.
void I_ShutdownSound(void) {}

//
//  SFX I/O
//

// Starts a sound in a particular sound channel.
int I_StartSound(int id, int vol, int sep, int pitch, int priority)
{
  	UINT8 volume = (((UINT16)vol + 1) * (UINT16)sfx_volume) / 62; // (256 * 31) / 62 == 127
	INT32 handle = Mix_PlayChannel(0, S_sfx[id].data, 0);
	Mix_Volume(handle, volume);
	Mix_SetPanning(handle, min((UINT16)(0xff-sep)<<1, 0xff), min((UINT16)(sep)<<1, 0xff));
	(void)pitch; // Mixer can't handle pitch
	(void)priority; // priority and channel management is handled by SRB2...
	return handle;  
}

// Stops a sound channel.
void I_StopSound(int handle)
{
    Mix_HaltChannel(handle);
}

// Called by S_*() functions
//  to see if a channel is still playing.
// Returns 0 if no longer playing, 1 if playing.
int I_SoundIsPlaying(int handle) {return 0;}

// Updates the volume, separation,
//  and pitch of a sound channel.
void I_UpdateSoundParams(int handle, int vol, int sep, int pitch) {}


//
//  MUSIC I/O
//
void I_InitMusic(void) 
{
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        CONS_Printf("Mix_OpenAudio failed: %s\n", Mix_GetError()); // uh oh
        return;
    }
}

void I_ShutdownMusic(void) {}

// Volume.
void I_SetMusicVolume(int volume)
{
    Mix_VolumeMusic((volume * MIX_MAX_VOLUME) / 31);
}

void I_SetSfxVolume(int volume)
{
	sfx_volume = volume;
}

// PAUSE game handling.
void I_PauseSong(int handle) {}
void I_ResumeSong(int handle) {}

// Registers a song handle to song data.
int I_RegisterSong(void *data,int len)
{
    SDL_RWops *rw = SDL_RWFromConstMem(data, len);
    if (!rw)
        return 0;

    current_music = Mix_LoadMUS_RW(rw, 1);

    if (!current_music)
    {
        CONS_Printf("Mix_LoadMUS_RW failed: %s\n", Mix_GetError()); // uh oh
        return 0;
    }

    return 1;
}

// Called by anything that wishes to start music.
//  plays a song, and when the song is done,
//  starts playing it again in an endless loop.
// Horrible thing to do, considering.
void I_PlaySong(int handle, int looping)
{
    (void)handle;

    if (current_music)
        Mix_PlayMusic(current_music, looping ? -1 : 1);
}

// Stops a song over 3 seconds.
void I_StopSong(int handle) {}

// See above (register), then think backwards
void I_UnRegisterSong(int handle) {}
