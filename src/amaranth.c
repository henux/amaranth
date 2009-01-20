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

/* Sound stream filename. */
static const char *filename = NULL;

/* Sampling rate bits/second (-r option). */
static int sample_rate = 44100;

/* Sample format (-f option). */
static Uint16 sample_format = AUDIO_S16LSB;

/* Number of channels (-c option). */
static Uint8 channel_count = 2;

/* Audio buffer size in samples (-s option). */
static Uint16 samples = 4096;

/*
** Audio stream callback.
**
** Called by the SDL background thread each time the audio buffer is ready
** to receive more data.  The function decodes PCM samples from the sound
** stream and copies them to the buffer for playback. When an end-of-stream
** is reached, closes the audio stream and exits cleanly.
*/
static void
stream_callback (void *userdata, Uint8 *buffer, int length)
{
  Sound_Sample *stream = (Sound_Sample *) userdata;

  Uint32 bytes_decoded;

  /* Decode a chunk of PCM samples from the sound stream. */
  Sound_SetBufferSize (stream, length);
  bytes_decoded = Sound_Decode (stream);

  if (bytes_decoded == 0) {
    /* End-of-stream reached; exit cleanly. */
    Sound_FreeSample (stream);
    Sound_Quit ();
    SDL_Quit ();
    exit (0);
  }

  /* Copy the decoded samples to the audio buffer. */
  memcpy (buffer, stream->buffer, length);
}

/* Process command line options from the program arguments. */
static void
options (int argc, char **argv)
{
}

int
main (int argc, char **argv)
{
  Sound_AudioInfo  info;
  Sound_Sample    *stream;
  SDL_AudioSpec    spec;

  /* Handle command line options. */
  if (options (argc, argv) < 0) {
    puts ("Usage: amaranth OPTIONS FILE\n\n");
    puts ("Options\n");
    puts ("    -r RATE    sampling rate bits/second (default: 441000)\n");
    puts ("    -f FMT     sample format (default: signed 16-bit little-endian)\n");
    puts ("    -c COUNT   number of channels (default: 2)\n");
    puts ("    -s SIZE    audio buffer size in samples (default: 4096)\n");
    puts ("\nFormats accepted by -f option:\n");
    puts ("    S16LSB     signed 16-bit little-endian\n");
    return 1;
  }
  
  /* Initialize SDL audio subsystem. */
  if (SDL_Init (SDL_INIT_AUDIO) < 0) {
    fprintf (stderr, "amaranth: cannot initialize SDL audio: %s\n",
             SDL_GetError ());
    return 1;
  }

  /* Initialize SDL_sound library. */
  if (Sound_Init () < 0) {
    fprintf (stderr, "amaranth: cannot initialize SDL_sound: %s\n",
             Sound_GetError ());
    return 1;
  }

  /* Open the sound stream. */
  info.format   = sample_format;
  info.channels = channel_count;
  info.rate     = sample_rate;

  stream = Sound_NewSampleFromFile (filename, &info, 4096);

  if (stream == NULL) {
    fprintf (stderr, "amaranth: cannot open sound stream %s: %s\n",
             filename, Sound_GetError ());
    return 1;
  }
  
  /* Open the audio device. */
  spec.freq     = sample_rate;
  spec.format   = sample_format;
  spec.channels = channel_count;
  spec.samples  = samples;
  spec.callback = stream_callback;
  spec.userdata = (void *) stream;

  if (SDL_OpenAudio (&spec, NULL) < 0) {
    fprintf (stderr, "amaranth: cannot open audio device: %s\n",
             SDL_GetError ());
    return 1;
  }

  /* Start playing audio and engage infinite loop. */
  SDL_PauseAudio (0);
  for (;;) SDL_Delay (10000);

  /* NOT REACHED */
  return 0;
}
