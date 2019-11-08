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

/* 
 * Driver for the JACK
 *
 * This code is derived from the fluid_jack.c in the fluidsynth
 * The code is modified for android jack and fluidsynth
 */

#include "SapaSynthClient.h"
#include "pthread.h"
#include <unistd.h>

#define MAX_MIDI_NOTE_COUNT     32
#define MAX_MIDI_NOTE_LENGTH    64
char audiodriver_midi_note[MAX_MIDI_NOTE_COUNT][MAX_MIDI_NOTE_LENGTH] =
		{ { 0, }, };
JAMSynthClient *JAMSynthClient::mRef = NULL;

pthread_mutex_t JAMSynthClient::last_client_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t JAMSynthClient::get_driver_mutex = PTHREAD_MUTEX_INITIALIZER;
fluid_jack_client_t* JAMSynthClient::last_client = NULL;

JAMSynthClient::JAMSynthClient() {
	strcpy(mClientName, "fluidsynth");

	mEnableMidiOut = 0;

	mPortMidiIn = NULL;
	mPortAudioLeftOut = NULL;
	mPortAudioRightOut = NULL;
	mRef = this;
}

JAMSynthClient::~JAMSynthClient() {
}

fluid_audio_driver_t*
new_fluid_apa_audio_driver(fluid_settings_t* settings, fluid_synth_t* synth) {
	return JAMSynthClient::static_new_driver(settings, synth);
}

fluid_audio_driver_t*
new_fluid_apa_audio_driver2(fluid_settings_t* settings, fluid_audio_func_t func,
		void* data) {
	return JAMSynthClient::static_new2_driver(settings, func, data);
}

int delete_fluid_apa_audio_driver(fluid_audio_driver_t* p) {
	return JAMSynthClient::static_free_driver(p);
}

void fluid_apa_audio_driver_settings(fluid_settings_t* settings) {
	JAMSynthClient::static_driver_settings(settings);
}

void fluid_apa_midi_driver_settings(fluid_settings_t *settings) {
	FLUID_LOG(FLUID_DBG,"fluid_apa_midi_driver_settings");
	JAMSynthClient::static_midi_driver_settings(settings);
}

fluid_midi_driver_t *
new_fluid_apa_midi_driver(fluid_settings_t *settings,
		handle_midi_event_func_t handler, void *data) {
	return JAMSynthClient::static_new_midi_driver(settings, handler, data);
}
int delete_fluid_apa_midi_driver(fluid_midi_driver_t *p) {
	return JAMSynthClient::static_delete_midi_driver(p);
}

int
new_fluid_apa_set_freewheel_mode(fluid_audio_driver_t* p, int onoff) {
	return JAMSynthClient::static_fluid_set_freewheel_mode(p, onoff);
}

fluid_audio_driver_t*
JAMSynthClient::static_new_driver(fluid_settings_t* settings,
		fluid_synth_t* synth) {
	return mRef->new_driver(settings, synth);

}

fluid_audio_driver_t*
JAMSynthClient::static_new2_driver(fluid_settings_t* settings,
		fluid_audio_func_t func, void* data) {
	return mRef->new2_driver(settings, func, data);

}

int JAMSynthClient::static_free_driver(fluid_audio_driver_t* p) {
	return mRef->free_driver(p);

}

void JAMSynthClient::static_driver_settings(fluid_settings_t* settings) {
	mRef->driver_settings(settings);

}

void JAMSynthClient::static_midi_driver_settings(fluid_settings_t *settings) {
	FLUID_LOG(FLUID_DBG,"static_midi_driver_settings");
	mRef->midi_driver_settings(settings);

}

fluid_midi_driver_t *
JAMSynthClient::static_new_midi_driver(fluid_settings_t *settings,
		handle_midi_event_func_t handler, void *data) {
	return mRef->new_midi_driver(settings, handler, data);

}

int JAMSynthClient::static_delete_midi_driver(fluid_midi_driver_t *p) {
	return mRef->delete_midi_driver(p);

}

int
JAMSynthClient::static_fluid_set_freewheel_mode(fluid_audio_driver_t* p, int onoff) {
	return mRef->fluid_jack_set_freewheel_mode(p, onoff);
}

fluid_audio_driver_t*
JAMSynthClient::new_driver(fluid_settings_t* settings, fluid_synth_t* synth) {
	return new_fluid_jack_audio_driver(settings, synth);
}

fluid_audio_driver_t*
JAMSynthClient::new2_driver(fluid_settings_t* settings, fluid_audio_func_t func,
		void* data) {
	return new_fluid_jack_audio_driver2(settings, func, data);
}

int JAMSynthClient::free_driver(fluid_audio_driver_t* p) {
	return delete_fluid_jack_audio_driver(p);
}

void JAMSynthClient::driver_settings(fluid_settings_t *settings) {
	fluid_jack_audio_driver_settings(settings);
}
void JAMSynthClient::midi_driver_settings(fluid_settings_t *settings) {
	FLUID_LOG(FLUID_DBG,"midi_driver_settings");
	fluid_jack_midi_driver_settings(settings);
}

fluid_midi_driver_t *
JAMSynthClient::new_midi_driver(fluid_settings_t *settings,
		handle_midi_event_func_t handler, void *data) {
	return new_fluid_jack_midi_driver(settings, handler, data);
}

int JAMSynthClient::delete_midi_driver(fluid_midi_driver_t *p) {
	return delete_fluid_jack_midi_driver(p);
}

int
JAMSynthClient::fluid_jack_set_freewheel_mode(fluid_audio_driver_t* p, int onoff)
{
  fluid_jack_audio_driver_t* dev = (fluid_jack_audio_driver_t*) p;
  if (jack_set_freewheel (dev->client_ref->client, onoff)) {
      FLUID_LOG(FLUID_DBG,"failed to reset freewheel mode\n");
  }
  return 0;
}

void JAMSynthClient::fluid_jack_audio_driver_settings(
		fluid_settings_t* settings) {
	FLUID_LOG(FLUID_DBG,"fluid_jack_audio_driver_settings");
	fluid_settings_register_str(settings, "audio.jack.id", mClientName, 0, NULL,
			NULL);

	fluid_settings_register_int(settings, "audio.jack.multi", 0, 0, 1,
			FLUID_HINT_TOGGLED, NULL, NULL);
	fluid_settings_register_int(settings, "audio.jack.autoconnect", 0, 0, 1,
			FLUID_HINT_TOGGLED, NULL, NULL);
	fluid_settings_register_str(settings, "audio.jack.server", "", 0, NULL,
			NULL);
}

/*
 * Create Jack client as necessary, share clients of the same server.
 * @param settings Settings object
 * @param isaudio TRUE if audio driver, FALSE if MIDI
 * @param driver fluid_jack_audio_driver_t or fluid_jack_midi_driver_t
 * @param data The user data instance associated with the driver (fluid_synth_t for example)
 * @return New or paired Audio/MIDI Jack client
 */
fluid_jack_client_t *
JAMSynthClient::new_fluid_jack_client(fluid_settings_t *settings, int isaudio,
		void *driver) {
	fluid_jack_client_t *client_ref = NULL;
	char *server = NULL;
	char* client_name;
	char name[64];

	fluid_settings_dupstr(settings, isaudio ? "audio.jack.server" /* ++ alloc server name */
	:
												"midi.jack.server", &server);

	//glib_g_static_mutex_lock (&JAMSynthClient::last_client_mutex);     /* ++ lock JAMSynthClient::last_client */
	pthread_mutex_lock(&JAMSynthClient::last_client_mutex);

	/* If the last client uses the same server and is not the same type (audio or MIDI),
	 * then re-use the client. */
	if (JAMSynthClient::last_client
			&& ((!JAMSynthClient::last_client->server && !server)
					|| FLUID_STRCMP(JAMSynthClient::last_client->server, server)
							== 0)
			&& ((!isaudio && !JAMSynthClient::last_client->midi_driver)
					|| (isaudio && !JAMSynthClient::last_client->audio_driver))) {
		client_ref = JAMSynthClient::last_client;
		JAMSynthClient::last_client = NULL; /* No more pairing for this client */

		/* Register ports */
		FLUID_LOG(FLUID_DBG,"new_fluid_jack_client exist client");
		if (fluid_jack_client_register_ports(driver, isaudio,
				client_ref->client, settings) != FLUID_OK)
			goto error_recovery;

		FLUID_LOG(FLUID_DBG,"after port register");
		if (isaudio) {
			pthread_mutex_lock(&JAMSynthClient::get_driver_mutex);
			//glib_g_atomic_pointer_set ((volatile gpointer G_GNUC_MAY_ALIAS *)&client_ref->audio_driver, (gint)driver);
			client_ref->audio_driver = (fluid_jack_audio_driver_t*) driver;
			pthread_mutex_unlock(&JAMSynthClient::get_driver_mutex);
		} else {
			pthread_mutex_lock(&JAMSynthClient::get_driver_mutex);
			//glib_g_atomic_pointer_set ((volatile gpointer G_GNUC_MAY_ALIAS *)&client_ref->midi_driver, (gint)driver);
			client_ref->midi_driver = (fluid_jack_midi_driver_t*) driver;
			pthread_mutex_unlock(&JAMSynthClient::get_driver_mutex);
		}

//    glib_g_static_mutex_unlock (&JAMSynthClient::last_client_mutex);       /* -- unlock JAMSynthClient::last_client */
		pthread_mutex_unlock(&JAMSynthClient::last_client_mutex);

		if (server)
			FLUID_FREE(server);

		return client_ref;
	}

	/* No existing client for this Jack server */
	client_ref = FLUID_NEW(fluid_jack_client_t);
	FLUID_LOG(FLUID_DBG,"new_fluid_jack_client make client \n");

	if (!client_ref) {
		FLUID_LOG(FLUID_DBG,"Out of memory");
		goto error_recovery;
	}

	FLUID_MEMSET(client_ref, 0, sizeof(fluid_jack_client_t));

	fluid_settings_dupstr(settings, isaudio ? "audio.jack.id" /* ++ alloc client name */
	:
												"midi.jack.id", &client_name);

	if (client_name != NULL && client_name[0] != 0)
		snprintf(name, 64, "%s", client_name);
	else
		strcpy(name, "fluidsynth");

	name[63] = '\0';

	if (client_name)
		FLUID_FREE(client_name); /* -- free client name */

	/* Open a connection to the Jack server and use the server name if specified */
	if (server && server[0] != '\0') {
		FLUID_LOG(FLUID_DBG,"new_fluid_jack_client have name %s \n", server);
		client_ref->client = jack_client_open(name, JackServerName, NULL,
				server);
	} else {
		FLUID_LOG(FLUID_DBG,"new_fluid_jack_client no name \n", server);
		client_ref->client = jack_client_open(name, JackNullOption, NULL);
	}

	if (!client_ref->client) {
		FLUID_LOG(FLUID_DBG,"Failed to connect to Jack server.");
		goto error_recovery;
	}

	jack_set_process_callback(client_ref->client, fluid_jack_driver_process,
			client_ref);
//  jack_set_buffer_size_callback (client_ref->client, fluid_jack_driver_bufsize, client_ref);
//  jack_set_sample_rate_callback (client_ref->client, fluid_jack_driver_srate, client_ref);
	jack_on_shutdown(client_ref->client, fluid_jack_driver_shutdown,
			client_ref);

	/* Register ports */
	if (fluid_jack_client_register_ports(driver, isaudio, client_ref->client,
			settings) != FLUID_OK)
		goto error_recovery;

	/* tell the JACK server that we are ready to roll */
	if (jack_activate(client_ref->client)) {
		FLUID_LOG(FLUID_DBG,"Failed to activate Jack client");
		goto error_recovery;
	}

	client_ref->server = server; /* !! takes over allocation */
	server = NULL; /* Set to NULL so it doesn't get freed below */

	JAMSynthClient::last_client = client_ref;

	if (isaudio) {
		pthread_mutex_lock(&JAMSynthClient::get_driver_mutex);
		//glib_g_atomic_pointer_set ((volatile gpointer G_GNUC_MAY_ALIAS *)&client_ref->audio_driver, (gint)driver);
		client_ref->audio_driver = (fluid_jack_audio_driver_t*) driver;
		pthread_mutex_unlock(&JAMSynthClient::get_driver_mutex);
	} else {
		pthread_mutex_lock(&JAMSynthClient::get_driver_mutex);
		//glib_g_atomic_pointer_set ((volatile gpointer G_GNUC_MAY_ALIAS *)&client_ref->midi_driver, (gint)driver);
		client_ref->midi_driver = (fluid_jack_midi_driver_t*) driver;
		pthread_mutex_unlock(&JAMSynthClient::get_driver_mutex);
	}

	//glib_g_static_mutex_unlock (&JAMSynthClient::last_client_mutex);       /* -- unlock JAMSynthClient::last_client */
	pthread_mutex_unlock(&JAMSynthClient::last_client_mutex);

	if (server)
		FLUID_FREE(server);

	return client_ref;

	error_recovery:

//glib_g_static_mutex_unlock (&JAMSynthClient::last_client_mutex);       /* -- unlock clients list */
	pthread_mutex_unlock(&JAMSynthClient::last_client_mutex);
	if (server)
		FLUID_FREE(server); /* -- free server name */

	if (client_ref) {
		if (client_ref->client)
			jack_client_close(client_ref->client);

		FLUID_FREE(client_ref);
	}

	return NULL;
}

int JAMSynthClient::fluid_jack_client_register_ports(void *driver, int isaudio,
		jack_client_t *client, fluid_settings_t *settings) {
	fluid_jack_audio_driver_t *dev;
	char name[64];
	int multi;
	int i;
	int jack_srate;
	//int midi_out;
	double sample_rate;

	FLUID_LOG(FLUID_DBG,"fluid_jack_client_register_ports");
	if (!isaudio) {
		fluid_jack_midi_driver_t *dev = (fluid_jack_midi_driver_t*) driver;

		dev->midi_port = jack_port_register(client, "midi-in",
				JACK_DEFAULT_MIDI_TYPE, JackPortIsInput | JackPortIsTerminal,
				0);
		if (!dev->midi_port) {
			FLUID_LOG(FLUID_DBG,"Failed to create Jack MIDI port");
			return FLUID_FAILED;
		}

		mRef->mPortMidiIn = dev->midi_port;

		// fluid_settings_getint (settings, "midi.jack.midi.out", &midi_out);
		//FLUID_LOG(FLUID_DBG, "midi.jack.midi.out %n", midi_out);

		if (mRef->mEnableMidiOut > 0)

		{

			dev->midi_out_port = jack_port_register(client, "midi-out",
					JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);

			if (!dev->midi_out_port) {
				FLUID_LOG(FLUID_DBG,"Failed to create Jack MIDI out port");
				return FLUID_FAILED;
			}
		} else {
			dev->midi_out_port = NULL;
		}
		return FLUID_OK;
	}

	dev = (fluid_jack_audio_driver_t*) driver;

	fluid_settings_getint(settings, "audio.jack.multi", &multi);

	if (multi) {
		/* create the two audio output ports */
		dev->num_output_ports = 1;

		dev->output_ports = FLUID_ARRAY (jack_port_t*, 2 * dev->num_output_ports);

		if (dev->output_ports == NULL) {
			FLUID_LOG(FLUID_DBG,"Jack server not running?");
			return FLUID_FAILED;
		}

		dev->output_bufs = FLUID_ARRAY (float*, 2 * dev->num_output_ports);
		FLUID_MEMSET(dev->output_ports, 0,
				2 * dev->num_output_ports * sizeof(jack_port_t*));

		dev->output_ports[0] = jack_port_register(client, "left",
				JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

		dev->output_ports[1] = jack_port_register(client, "right",
				JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

	} else {
		fluid_settings_getint(settings, "synth.audio-channels",
				&dev->num_output_ports);

		dev->output_ports = FLUID_ARRAY (jack_port_t *, 2 * dev->num_output_ports);

		if (dev->output_ports == NULL) {
			FLUID_LOG(FLUID_DBG,"Out of memory");
			return FLUID_FAILED;
		}

		dev->output_bufs = FLUID_ARRAY (float *, 2 * dev->num_output_ports);

		if (dev->output_bufs == NULL) {
			FLUID_LOG(FLUID_DBG,"Out of memory");
			return FLUID_FAILED;
		}

		FLUID_MEMSET(dev->output_ports, 0,
				2 * dev->num_output_ports * sizeof(jack_port_t*));

		for (i = 0; i < dev->num_output_ports; i++) {
			sprintf(name, "l_%02d", i);
			dev->output_ports[2 * i] = jack_port_register(client, name,
					JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
			mRef->mPortAudioLeftOut = dev->output_ports[2 * i];

			sprintf(name, "r_%02d", i);
			dev->output_ports[2 * i + 1] = jack_port_register(client, name,
					JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
			mRef->mPortAudioRightOut = dev->output_ports[2 * i + 1];
		}
	}

	/* Adjust sample rate to match JACK's */
	jack_srate = jack_get_sample_rate(client);
	FLUID_LOG(FLUID_DBG,"Jack engine sample rate: %lu", jack_srate);

	fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);

	if ((int) sample_rate != jack_srate) {
		FLUID_LOG(FLUID_DBG,"Jack sample rate mismatch, adjusting."
				" (synth.sample-rate=%lu, jackd=%lu)", (int) sample_rate,
				jack_srate);
		fluid_settings_setnum(settings, "synth.sample-rate", jack_srate);
	}

	/* Changing sample rate is non RT, so make sure we process it and/or other things now */
	if (dev->callback == NULL) {
		fluid_synth_process_event_queue((fluid_synth_t*) dev->data);
	}
	return FLUID_OK;
}

void JAMSynthClient::fluid_jack_client_close(fluid_jack_client_t *client_ref,
		void *driver) {
	if (client_ref->audio_driver == driver) {
		pthread_mutex_lock(&JAMSynthClient::get_driver_mutex);
		//glib_g_atomic_pointer_set ((volatile gpointer G_GNUC_MAY_ALIAS *)&client_ref->audio_driver, (gint)NULL);
		client_ref->audio_driver = NULL;
		pthread_mutex_unlock(&JAMSynthClient::get_driver_mutex);
	} else if (client_ref->midi_driver == driver) {
		pthread_mutex_lock(&JAMSynthClient::get_driver_mutex);
		//glib_g_atomic_pointer_set ((volatile gpointer G_GNUC_MAY_ALIAS *)&client_ref->midi_driver, (gint)NULL);
		client_ref->midi_driver = NULL;
		pthread_mutex_unlock(&JAMSynthClient::get_driver_mutex);

	}

	if (client_ref->audio_driver || client_ref->midi_driver) {
		usleep(100000); /* FIXME - Hack to make sure that resources don't get freed while Jack callback is active */
		return;
	}

	//glib_g_static_mutex_lock (&JAMSynthClient::last_client_mutex);
	pthread_mutex_lock(&JAMSynthClient::last_client_mutex);

	if (client_ref == JAMSynthClient::last_client)
		JAMSynthClient::last_client = NULL;

	//glib_g_static_mutex_unlock (&JAMSynthClient::last_client_mutex);
	pthread_mutex_unlock(&JAMSynthClient::last_client_mutex);

	mRef->mPortMidiIn = NULL;
	mRef->mPortAudioLeftOut = NULL;
	mRef->mPortAudioRightOut = NULL;
	if (client_ref->client)
		jack_client_close(client_ref->client);

	if (client_ref->server)
		FLUID_FREE(client_ref->server);

	FLUID_FREE(client_ref);
}

fluid_audio_driver_t*
JAMSynthClient::new_fluid_jack_audio_driver(fluid_settings_t* settings,
		fluid_synth_t* synth) {
	return new_fluid_jack_audio_driver2(settings, NULL, synth);
}

fluid_audio_driver_t*
JAMSynthClient::new_fluid_jack_audio_driver2(fluid_settings_t* settings,
		fluid_audio_func_t func, void* data) {
	fluid_jack_audio_driver_t* dev = NULL;
	jack_client_t *client;
	const char ** jack_ports; /* for looking up ports */
	int autoconnect = 0;
	int i;

	FLUID_LOG(FLUID_DBG,"new_fluid_jack_audio_driver2");
	dev = FLUID_NEW(fluid_jack_audio_driver_t);
	if (dev == NULL) {
		FLUID_LOG(FLUID_ERR,"Out of memory");
		return NULL;
	}
	FLUID_MEMSET(dev, 0, sizeof(fluid_jack_audio_driver_t));

	dev->callback = func;
	dev->data = data;

	dev->client_ref = new_fluid_jack_client(settings, TRUE, dev);

	if (!dev->client_ref) {
		FLUID_FREE(dev);
		return NULL;
	}

	client = dev->client_ref->client;

	/* connect the ports. */

	/* FIXME: should be done by a patchbay application */

	/* find some physical ports and connect to them */
	fluid_settings_getint(settings, "audio.jack.autoconnect", &autoconnect);
	FLUID_LOG(FLUID_DBG,"audio.jack.autoconnect %d", autoconnect);
	if (0) {
		FLUID_LOG(FLUID_DBG,"audio.jack.autoconnect %d", autoconnect);
		jack_ports = jack_get_ports(client, NULL, NULL,
				JackPortIsInput | JackPortIsPhysical);
		if (jack_ports) {
			int err;
			int connected = 0;

			for (i = 0; jack_ports[i] && i < 2 * dev->num_output_ports; ++i) {
				err = jack_connect(client, jack_port_name(dev->output_ports[i]),
						jack_ports[i]);
				if (err) {
					FLUID_LOG(FLUID_DBG,"Error connecting jack port");
				} else {
					FLUID_LOG(FLUID_DBG,"connected jack port %d", i);
					connected++;
				}
			}

			jack_free(jack_ports); /* free jack ports array (not the port values!) */
		} else {
			FLUID_LOG(FLUID_DBG,"Could not connect to any physical jack ports; fluidsynth is unconnected");
		}
	}

	return (fluid_audio_driver_t*) dev;
}

/*
 * delete_fluid_jack_audio_driver
 */
int JAMSynthClient::delete_fluid_jack_audio_driver(fluid_audio_driver_t* p) {
	fluid_jack_audio_driver_t* dev = (fluid_jack_audio_driver_t*) p;

	if (dev == NULL)
		return 0;

	if (dev->client_ref != NULL)
		fluid_jack_client_close(dev->client_ref, dev);

	if (dev->output_bufs)
		FLUID_FREE(dev->output_bufs);

	if (dev->output_ports)
		FLUID_FREE(dev->output_ports);

	FLUID_FREE(dev);
	return 0;
}

/* Process function for audio and MIDI Jack drivers */
int JAMSynthClient::fluid_jack_driver_process(jack_nframes_t nframes,
		void *arg) {
	fluid_jack_client_t *client = (fluid_jack_client_t *) arg;
	fluid_jack_audio_driver_t *audio_driver;
	fluid_jack_midi_driver_t *midi_driver;
	float *left, *right;
	int i, k;

	jack_midi_event_t midi_event;
	fluid_midi_event_t *evt;
	void *midi_buffer = NULL;
	void* port_buf = NULL;
	jack_nframes_t event_count;
	jack_nframes_t event_index;
	unsigned int u;

	int midi_note_read[64];
	unsigned char* buffer;

//  FLUID_LOG( "fluid_jack_driver_process");

	/* Process MIDI events first, so that they take effect before audio synthesis */
	// midi_driver = (fluid_jack_midi_driver_t*)fluid_atomic_pointer_get ((volatile gpointer G_GNUC_MAY_ALIAS *)&client->midi_driver);
	pthread_mutex_lock(&JAMSynthClient::get_driver_mutex);
	//midi_driver = (fluid_jack_midi_driver_t*)glib_g_atomic_pointer_get ((volatile gpointer G_GNUC_MAY_ALIAS *)&client->midi_driver);
	midi_driver = (fluid_jack_midi_driver_t*) client->midi_driver;
	pthread_mutex_unlock(&JAMSynthClient::get_driver_mutex);
	if (midi_driver) {
		midi_buffer = jack_port_get_buffer(midi_driver->midi_port, 0);
		if (mRef->mEnableMidiOut > 0)

		{
			port_buf = jack_port_get_buffer(midi_driver->midi_out_port,
					nframes);
		}
		event_count = jack_midi_get_event_count(midi_buffer);

		for (event_index = 0; event_index < event_count; event_index++) {
			jack_midi_event_get(&midi_event, midi_buffer, event_index);

			/* let the parser convert the data into events */
			for (u = 0; u < midi_event.size; u++) {
				evt = fluid_midi_parser_parse(midi_driver->parser,
						midi_event.buffer[u]);

				/* send the event to the next link in the chain */
				if (evt != NULL)
					midi_driver->driver.handler(midi_driver->driver.data, evt);
			}
		}

		// if midi out is enable?

		if (mRef->mEnableMidiOut > 0)

		{
			if (port_buf) {
				jack_midi_clear_buffer(port_buf);
			}
			for (jack_nframes_t i = 0; i < nframes; i++) {
				//get one note
				char midi_note_one[MAX_MIDI_NOTE_LENGTH] = { 0, };
				for (int cnt = 0; cnt < MAX_MIDI_NOTE_COUNT; cnt++) {
					if (audiodriver_midi_note[cnt][0] != 0x0) {
						memcpy(midi_note_one, audiodriver_midi_note[cnt],
								MAX_MIDI_NOTE_LENGTH);
						memset(audiodriver_midi_note[cnt], 0x0,
								MAX_MIDI_NOTE_LENGTH);
						break;
					}
				}

				if (midi_note_one[0] != 0x0) //someone wrote midi event
						{
					sscanf(midi_note_one, "%x", &midi_note_read[0]);
					switch (midi_note_read[0] & 0xf0) {
					case 0xc0: //Program Change
						buffer = jack_midi_event_reserve(port_buf, i, 2);
						sscanf(midi_note_one, "%x %x", &midi_note_read[0],
								&midi_note_read[1]);
						buffer[1] = (unsigned char) midi_note_read[1];
						buffer[0] = (unsigned char) midi_note_read[0];
						break;
					case 0x80: //Note Off
					case 0x90: //Note On
					case 0xb0: //Control Change
					case 0xe0: //Pitch Wheel Change
						buffer = jack_midi_event_reserve(port_buf, i, 3);
						sscanf(midi_note_one, "%x %x %x", &midi_note_read[0],
								&midi_note_read[1], &midi_note_read[2]);
						buffer[2] = (unsigned char) midi_note_read[2];
						buffer[1] = (unsigned char) midi_note_read[1];
						buffer[0] = (unsigned char) midi_note_read[0];
						FLUID_LOG(FLUID_DBG,"Midi message process %x %x %x", buffer[0],
								buffer[1], buffer[2]);
						break;
					default:
						break;
					}

				}
			}
		}
	}

	audio_driver = client->audio_driver;

	if (!audio_driver)
		return 0;

	if (audio_driver->callback != NULL) {
		for (i = 0; i < audio_driver->num_output_ports * 2; i++)
			audio_driver->output_bufs[i] = (float *) jack_port_get_buffer(
					audio_driver->output_ports[i], nframes);

		return (*audio_driver->callback)(audio_driver->data, nframes, 0, NULL,
				2 * audio_driver->num_output_ports, audio_driver->output_bufs);
	} else if (audio_driver->num_output_ports == 1) {
		  left = (float*) jack_port_get_buffer (audio_driver->output_ports[0], nframes);
		  right = (float*) jack_port_get_buffer (audio_driver->output_ports[1], nframes);
		  fluid_synth_write_float ((fluid_synth_t*)audio_driver->data, nframes, left, 0, 1, right, 0, 1);
	} else {
		for (i = 0, k = audio_driver->num_output_ports;
				i < audio_driver->num_output_ports; i++, k++) {
			audio_driver->output_bufs[i] = (float *) jack_port_get_buffer(
					audio_driver->output_ports[2 * i], nframes);
			audio_driver->output_bufs[k] = (float *) jack_port_get_buffer(
					audio_driver->output_ports[2 * i + 1], nframes);
		}

		fluid_synth_nwrite_float((fluid_synth_t*) audio_driver->data, nframes,
				audio_driver->output_bufs,
				audio_driver->output_bufs + audio_driver->num_output_ports,
				NULL, NULL);
	}
	return 0;
}

int JAMSynthClient::fluid_jack_driver_bufsize(jack_nframes_t nframes,
		void *arg) {
	/*   printf("the maximum buffer size is now %lu\n", nframes); */
	return 0;
}

int JAMSynthClient::fluid_jack_driver_srate(jack_nframes_t nframes, void *arg) {
	/*   printf("the sample rate is now %lu/sec\n", nframes); */
	/* FIXME: change the sample rate of the synthesizer! */
	return 0;
}

void JAMSynthClient::fluid_jack_driver_shutdown(void *arg) {
//  fluid_jack_audio_driver_t* dev = (fluid_jack_audio_driver_t*) arg;
	FLUID_LOG(FLUID_DBG,"Help! Lost the connection to the JACK server");
	/*   exit (1); */
}

void JAMSynthClient::fluid_jack_midi_driver_settings(
		fluid_settings_t *settings) {
	FLUID_LOG(FLUID_DBG,"fluid_jack_midi_driver_settings");
	char tmp[128];
	memcpy(tmp, mClientName, strlen(mClientName));

	strcat(tmp, "-midi");

	fluid_settings_register_str(settings, "midi.jack.id", tmp, 0, NULL, NULL);

	//fluid_settings_register_int (settings, "midi.jack.midi.out", mEnableMidiOut, 0, 1, FLUID_HINT_TOGGLED,  NULL, NULL);	
	fluid_settings_register_str(settings, "midi.jack.server", "", 0, NULL,
			NULL);
}

/*
 * new_fluid_jack_midi_driver
 */
fluid_midi_driver_t *
JAMSynthClient::new_fluid_jack_midi_driver(fluid_settings_t *settings,
		handle_midi_event_func_t handler, void *data) {
	fluid_jack_midi_driver_t* dev;

	if (handler == NULL)
		return NULL;

	FLUID_LOG(FLUID_DBG,"new_fluid_jack_midi_driver");

	/* allocate the device */
	dev = FLUID_NEW(fluid_jack_midi_driver_t);

	if (dev == NULL) {
		FLUID_LOG(FLUID_DBG,"Out of memory");
		return NULL;
	}

	FLUID_MEMSET(dev, 0, sizeof(fluid_jack_midi_driver_t));

	dev->driver.handler = handler;
	dev->driver.data = data;

	/* allocate one event to store the input data */
	dev->parser = new_fluid_midi_parser();

	if (dev->parser == NULL) {
		FLUID_LOG(FLUID_DBG,"Out of memory");
		FLUID_FREE(dev);
		return NULL;
	}

	dev->client_ref = new_fluid_jack_client(settings, FALSE, dev);

	if (!dev->client_ref) {
		FLUID_FREE(dev);
		return NULL;
	}

	return (fluid_midi_driver_t *) dev;
}

int JAMSynthClient::delete_fluid_jack_midi_driver(fluid_midi_driver_t *p) {
	fluid_jack_midi_driver_t *dev = (fluid_jack_midi_driver_t *) p;

	if (dev == NULL)
		return FLUID_OK;

	if (dev->client_ref != NULL)
		fluid_jack_client_close(dev->client_ref, dev);

	if (dev->parser != NULL)
		delete_fluid_midi_parser(dev->parser);

	FLUID_FREE(dev);

	return FLUID_OK;
}


void JAMSynthClient::setClientName(const char* name) {
	strncpy(mClientName, name, JACK_CLIENT_NAME_SIZE - 3);

	FLUID_LOG(FLUID_DBG,"JAMSynthClient::setClientName requested name is %s, set name is %s",
			name, mClientName);

}

void JAMSynthClient::setMidiOut(int midiout) {
	FLUID_LOG(FLUID_DBG,"JAMSynthClient::setMidiOut %n", midiout);
	mEnableMidiOut = midiout;
}

int JAMSynthClient::static_fluid_midi_get_midi_out() {
	return mRef->mEnableMidiOut;
}

const jack_port_t*
JAMSynthClient::getMidiInPort() {
	return mPortMidiIn;
}

const jack_port_t*
JAMSynthClient::getLeftAudioOutPort() {
	return mPortAudioLeftOut;
}

const jack_port_t*
JAMSynthClient::getRightAudioOutPort() {
	return mPortAudioRightOut;

}

int fluid_apa_get_midi_out() {
	return JAMSynthClient::static_fluid_midi_get_midi_out();
}

void fluid_apa_make_midi_message(int chan, int key, int val) {
	static int cnt = 0;
	char midi[32] = { 0, };
	// 0x90 is 144 in decimal
	sprintf(midi, "%02x %02x %02x", chan + 144, key, val);
	FLUID_LOG(FLUID_DBG,"make mid message : %s", midi);

	for (cnt; cnt < MAX_MIDI_NOTE_COUNT; cnt++) {
		if (audiodriver_midi_note[cnt][0] == 0x0) {
			memcpy(audiodriver_midi_note[cnt], midi, strlen(midi));
			break;
		}
	}
	if (cnt >= (MAX_MIDI_NOTE_COUNT - 1)) {
		cnt = 0;
	}
}
