/*
amaranth.c -- Command line music player.  

Copyright (C) 2008 Henri Häkkinen.

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option)
any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

You should have received a copy of the GNU General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"
#include "SDL_sound.h"

/*
** Audio stream callback.
**
** Called by the SDL background thread each time the audio buffer is ready
** to receive more data.  The function decodes PCM samples from the sound
** stream and copies them to the buffer for playback. When an end-of-stream
** is reached, closes the audio stream and exits cleanly.
*/
static void
stream_callback (void *userdata, Uint8 *stream, int length)
{
  Sound_Sample *sample = (Sound_Sample *) userdata;

  Uint32 bytes_decoded;

  /* Decode a chunk of PCM samples from the sound stream. */
  Sound_SetBufferSize (sample, length);
  bytes_decoded = Sound_Decode (sample);

  if (bytes_decoded == 0) {
    /* End-of-stream reached; exit cleanly. */
    Sound_FreeSample (sample);
    Sound_Quit ();
    SDL_Quit ();
    exit (0);
  }

  /* Copy the decoded samples to the audio buffer. */
  memcpy (stream, sample->buffer, length);
}

int
main (int argc, char **argv)
{
  Sound_AudioInfo  info;
  Sound_Sample    *sample;
  SDL_AudioSpec    spec;

  /* Handle command line options. */
  if (argc < 2) {
    fprintf (stderr, "Usage: amaranth FILE\n");
    return 1;
  }
  
  /* Initialize SDL audio subsystem. */
  if (SDL_Init (SDL_INIT_AUDIO) < 0) {
    fprintf (stderr, "amaranth: cannot initialize SDL audio: %s\n", SDL_GetError ());
    return 1;
  }

  /* Initialize SDL_sound library. */
  if (Sound_Init () < 0) {
    fprintf (stderr, "amaranth: cannot initialize SDL_sound: %s\n", Sound_GetError ());
    return 1;
  }

  /* Open the sound stream. */
  info.format   = AUDIO_S16LSB;
  info.channels = 2;
  info.rate     = 44100;

  sample = Sound_NewSampleFromFile (argv[1], &info, 4096);
  if (sample == NULL) {
    fprintf (stderr, "amaranth: cannot open sound stream %s: %s\n", argv[1], Sound_GetError ());
    return 1;
  }
  
  /* Open the audio device. */
  spec.freq     = 44100;
  spec.format   = AUDIO_S16LSB;
  spec.channels = 2;
  spec.samples  = 1024;
  spec.callback = stream_callback;
  spec.userdata = (void *) sample;

  if (SDL_OpenAudio (&spec, NULL) < 0) {
    fprintf (stderr, "amaranth: cannot open audio device: %s\n", SDL_GetError ());
    return 1;
  }

  /* Start playing audio and engage infinite loop. */
  SDL_PauseAudio (0);
  for (;;) SDL_Delay (10000);

  /* NOT REACHED */
  return 0;
}
