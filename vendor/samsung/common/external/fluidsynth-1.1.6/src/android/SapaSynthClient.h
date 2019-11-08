/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */


#ifndef _FLUID_APA_H
#define _FLUID_APA_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "fluidsynth_priv.h"
#include "fluid_sys.h"
#include "fluid_synth.h"
#include "fluid_adriver.h"
#include "fluid_mdriver.h"
#include "fluid_settings.h"

#ifdef __cplusplus
}
#endif

#include <jack/jack.h>
#include <jack/midiport.h>
#define JACK_CLIENT_NAME_SIZE 64

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _fluid_jack_audio_driver_t fluid_jack_audio_driver_t;
typedef struct _fluid_jack_midi_driver_t fluid_jack_midi_driver_t;
typedef struct _fluid_jack_client_t	fluid_jack_client_t;

/* Clients are shared for drivers using the same server. */
struct _fluid_jack_client_t
{
  jack_client_t *client;
  char *server;                 /* Jack server name used */
  fluid_jack_audio_driver_t *audio_driver;
  fluid_jack_midi_driver_t *midi_driver;
};

/* Jack audio driver instance */
struct _fluid_jack_audio_driver_t
{
  fluid_audio_driver_t driver;
  fluid_jack_client_t *client_ref;
  int audio_channels;
  jack_port_t **output_ports;
  int num_output_ports;
  float **output_bufs;
  fluid_audio_func_t callback;
  void* data;
};

/* Jack MIDI driver instance */
struct _fluid_jack_midi_driver_t
{
  fluid_midi_driver_t driver;
  fluid_jack_client_t *client_ref;
  jack_port_t *midi_port;
  jack_port_t *midi_out_port;
  fluid_midi_parser_t *parser;
};

//static fluid_mutex_t last_client_mutex = G_STATIC_MUTEX_INIT;     /* Probably not necessary, but just in case drivers are created by multiple threads */
//static fluid_jack_client_t *last_client = NULL;       /* Last unpaired client. For audio/MIDI driver pairing. */
fluid_audio_driver_t* new_fluid_apa_audio_driver(fluid_settings_t* settings, fluid_synth_t* synth); 
fluid_audio_driver_t* new_fluid_apa_audio_driver2(fluid_settings_t* settings, fluid_audio_func_t func, void* data);
int delete_fluid_apa_audio_driver(fluid_audio_driver_t* p);
void fluid_apa_audio_driver_settings(fluid_settings_t* settings);

void fluid_apa_midi_driver_settings (fluid_settings_t *settings);
fluid_midi_driver_t *new_fluid_apa_midi_driver (fluid_settings_t *settings, handle_midi_event_func_t handler, void *data);
int delete_fluid_apa_midi_driver(fluid_midi_driver_t *p);

void fluid_apa_make_midi_message(int chan, int key, int val);
int fluid_apa_get_midi_out();

int new_fluid_apa_set_freewheel_mode(fluid_audio_driver_t* p, int onoff);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
class JAMSynthClient
{
	public :
		//static void setInstance(JAMSynthClient* ref)

		//{

		//	mRef = ref;

		//}

		virtual ~JAMSynthClient();			
		JAMSynthClient();
	private:
		JAMSynthClient(const JAMSynthClient&);
		JAMSynthClient& operator=(const JAMSynthClient&);

		
	public :
		void driver_settings(fluid_settings_t* settings);
		fluid_audio_driver_t* new_driver(fluid_settings_t* settings, fluid_synth_t* synth);
		fluid_audio_driver_t* new2_driver(fluid_settings_t* settings, fluid_audio_func_t func, void* data);
		int free_driver(fluid_audio_driver_t* p);		

		void midi_driver_settings(fluid_settings_t *settings);
		fluid_midi_driver_t* new_midi_driver(fluid_settings_t *settings, handle_midi_event_func_t handler, void *data);
		int delete_midi_driver(fluid_midi_driver_t *p);

		fluid_jack_client_t *new_fluid_jack_client (fluid_settings_t *settings, int isaudio, void *driver);
		static int fluid_jack_client_register_ports (void *driver, int isaudio, jack_client_t *client, fluid_settings_t *settings);
		fluid_audio_driver_t* new_fluid_jack_audio_driver(fluid_settings_t* settings, fluid_synth_t* synth);
		fluid_audio_driver_t* new_fluid_jack_audio_driver2(fluid_settings_t* settings, fluid_audio_func_t func, void* data);
		int delete_fluid_jack_audio_driver(fluid_audio_driver_t* p);
		static void fluid_jack_driver_shutdown(void *arg);
		int fluid_jack_driver_srate(jack_nframes_t nframes, void *arg);
		int fluid_jack_driver_bufsize(jack_nframes_t nframes, void *arg);
		static int fluid_jack_driver_process(jack_nframes_t nframes, void *arg);		
		void fluid_jack_audio_driver_settings(fluid_settings_t* settings);
		static void fluid_jack_client_close (fluid_jack_client_t *client_ref, void *driver);
		void fluid_jack_midi_driver_settings (fluid_settings_t *settings);
		fluid_midi_driver_t * new_fluid_jack_midi_driver (fluid_settings_t *settings, handle_midi_event_func_t handler, void *data);
		int delete_fluid_jack_midi_driver(fluid_midi_driver_t *p);

		int fluid_jack_set_freewheel_mode(fluid_audio_driver_t *p, int onoff);

	public :		
		static fluid_audio_driver_t* static_new_driver(fluid_settings_t* settings, fluid_synth_t* synth);
		static fluid_audio_driver_t* static_new2_driver(fluid_settings_t* settings, fluid_audio_func_t func, void* data);
		static int static_free_driver(fluid_audio_driver_t* p);
		static void static_driver_settings(fluid_settings_t* settings);

		static void static_midi_driver_settings(fluid_settings_t *settings);
		static fluid_midi_driver_t* static_new_midi_driver(fluid_settings_t *settings, handle_midi_event_func_t handler, void *data);
		static int static_delete_midi_driver(fluid_midi_driver_t *p);
		
		static int static_fluid_midi_get_midi_out();
		static int static_fluid_set_freewheel_mode(fluid_audio_driver_t* p, int onoff);
			
		//static fluid_jack_client_t	jack_client_t;
		//static fluid_jack_audio_driver_t	jack_audio_driver_t;
		//static fluid_jack_midi_driver_t		jack_midi_driver_t;
		static pthread_mutex_t		last_client_mutex;
		static pthread_mutex_t		get_driver_mutex;
		static fluid_jack_client_t		*last_client;
		
	private :
		static JAMSynthClient* mRef;

		char mClientName[JACK_CLIENT_NAME_SIZE];

		int mEnableMidiOut;
		const jack_port_t* mPortMidiIn;
		const jack_port_t* mPortAudioLeftOut;
		const jack_port_t* mPortAudioRightOut;
				
	public :
		//virtual function

		const jack_port_t* getMidiInPort();
		const jack_port_t* getLeftAudioOutPort();
		const jack_port_t* getRightAudioOutPort();
		
	public :
		void setClientName(const char* name);
		void setMidiOut(int midiout);
};
#endif

#endif 
